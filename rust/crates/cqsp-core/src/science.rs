//! Original science-field graph and technology definitions; no invented research tick.
use crate::read_entries;
use anyhow::{Context, Result, ensure};
use serde_json::Value;
use std::{
    collections::{BTreeSet, HashMap},
    path::Path,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct FieldId(pub usize);

#[derive(Debug)]
pub struct Field {
    pub identifier: String,
    pub name: String,
    pub description: Option<String>,
    pub parents: Vec<FieldId>,
    pub adjacent: Vec<FieldId>,
}

#[derive(Debug)]
pub struct Technology {
    pub identifier: String,
    pub name: String,
    pub description: Option<String>,
    pub fields: BTreeSet<FieldId>,
    pub actions: Vec<String>,
    pub difficulty: i32,
    pub tags: Vec<String>,
}

pub struct ScienceCatalog {
    pub fields: Vec<Field>,
    pub technologies: Vec<Technology>,
    pub diagnostics: Vec<String>,
    field_ids: HashMap<String, FieldId>,
}

impl ScienceCatalog {
    pub fn load(data: &Path) -> Result<Self> {
        Self::from_entries(
            &read_entries::<Value>(&data.join("data/science/fields"))?,
            &read_entries::<Value>(&data.join("data/science/technology"))?,
        )
    }

    pub fn field_id(&self, identifier: &str) -> Option<FieldId> {
        self.field_ids.get(identifier).copied()
    }

    pub fn definitions_json(&self) -> Value {
        let names = |ids: &[FieldId]| {
            ids.iter()
                .map(|id| self.fields[id.0].identifier.as_str())
                .collect::<Vec<_>>()
        };
        let fields: serde_json::Map<_, _> = self.fields.iter().map(|field| (field.identifier.clone(), serde_json::json!({
            "name": field.name, "description": field.description, "parents": names(&field.parents), "adjacent": names(&field.adjacent)
        }))).collect();
        let technologies: serde_json::Map<_, _> = self.technologies.iter().map(|tech| (tech.identifier.clone(), serde_json::json!({
            "name": tech.name, "description": tech.description, "fields": names(&tech.fields.iter().copied().collect::<Vec<_>>()),
            "actions": tech.actions, "difficulty": tech.difficulty, "tags": tech.tags
        }))).collect();
        serde_json::json!({"fields": fields, "technologies": technologies})
    }

    pub fn from_entries(field_entries: &[Value], tech_entries: &[Value]) -> Result<Self> {
        let mut fields = Vec::new();
        let mut field_ids = HashMap::new();
        let mut diagnostics = Vec::new();
        let mut links = Vec::new();
        for entry in field_entries {
            let (Some(name), Some(identifier)) =
                (entry["name"].as_str(), entry["identifier"].as_str())
            else {
                diagnostics.push("Skipped science field without string name/identifier".into());
                continue;
            };
            if field_ids
                .insert(identifier.to_owned(), FieldId(fields.len()))
                .is_some()
            {
                diagnostics.push(format!(
                    "Duplicate science field {identifier}: last lookup wins"
                ));
            }
            fields.push(Field {
                identifier: identifier.into(),
                name: name.into(),
                description: text(entry, "description"),
                parents: Vec::new(),
                adjacent: Vec::new(),
            });
            links.push((strings(entry, "parent"), strings(entry, "adjacent")));
        }
        for (field, (parents, adjacent)) in fields.iter_mut().zip(links) {
            for (names, target) in [
                (parents, &mut field.parents),
                (adjacent, &mut field.adjacent),
            ] {
                for name in names {
                    if name == field.identifier {
                        diagnostics.push(format!("{}: skipped self-reference", field.identifier));
                    } else if let Some(&id) = field_ids.get(&name) {
                        target.push(id);
                    } else {
                        diagnostics.push(format!(
                            "{}: unknown science field {name}",
                            field.identifier
                        ));
                    }
                }
            }
        }
        let mut technologies = Vec::new();
        for entry in tech_entries {
            let identifier = entry["identifier"]
                .as_str()
                .context("Technology has no identifier")?;
            let name = entry["name"].as_str().context("Technology has no name")?;
            let difficulty = entry["difficulty"].as_f64().unwrap_or(10.0);
            ensure!(
                difficulty.is_finite()
                    && difficulty >= i32::MIN as f64
                    && difficulty <= i32::MAX as f64,
                "{identifier}: invalid research difficulty"
            );
            let tech_fields = strings(entry, "fields")
                .iter()
                .map(|name| {
                    field_ids
                        .get(name)
                        .copied()
                        .with_context(|| format!("{identifier}: unknown science field {name}"))
                })
                .collect::<Result<_>>()?;
            technologies.push(Technology {
                identifier: identifier.into(),
                name: name.into(),
                description: text(entry, "description"),
                fields: tech_fields,
                actions: strings(entry, "actions"),
                difficulty: difficulty as i32,
                tags: strings(entry, "tags"),
            });
        }
        Ok(Self {
            fields,
            technologies,
            diagnostics,
            field_ids,
        })
    }
}

fn text(entry: &Value, key: &str) -> Option<String> {
    entry[key].as_str().map(str::to_owned)
}
fn strings(entry: &Value, key: &str) -> Vec<String> {
    entry[key].as_array().map_or_else(Vec::new, |items| {
        items
            .iter()
            .filter_map(Value::as_str)
            .map(str::to_owned)
            .collect()
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn original_science_graph_and_actions_load_without_a_research_simulator() -> Result<()> {
        let science = ScienceCatalog::load(&crate::default_data_path())?;
        assert_eq!(science.fields.len(), 11);
        assert_eq!(science.technologies.len(), 7);
        assert!(science.diagnostics.is_empty());
        let physics = &science.fields[science.field_id("physics").unwrap().0];
        assert_eq!(physics.parents, [science.field_id("science").unwrap()]);
        assert_eq!(
            physics.adjacent,
            [
                science.field_id("mathematics").unwrap(),
                science.field_id("algebra").unwrap()
            ]
        );
        assert!(
            science.fields[science.field_id("mathematics").unwrap().0]
                .adjacent
                .is_empty()
        );
        let mining = science
            .technologies
            .iter()
            .find(|tech| tech.identifier == "basic_mining")
            .unwrap();
        assert_eq!(mining.difficulty, 5000);
        assert_eq!(mining.actions.len(), 5);
        assert!(mining.fields.is_empty());
        Ok(())
    }

    #[test]
    fn graph_references_keep_direction_duplicates_and_source_skip_rules() -> Result<()> {
        let science = ScienceCatalog::from_entries(
            &[
                json!({"name":"A", "identifier":"a", "parent":["b","b","a","missing"]}),
                json!({"name":"B", "identifier":"b", "adjacent":["a"]}),
                json!({"identifier":"skipped"}),
            ],
            &[json!({"name":"T", "identifier":"t", "fields":["a","a","b"], "difficulty":10.9})],
        )?;
        assert_eq!(science.fields[0].parents, [FieldId(1), FieldId(1)]);
        assert_eq!(science.fields[1].adjacent, [FieldId(0)]);
        assert_eq!(science.technologies[0].fields.len(), 2);
        assert_eq!(science.technologies[0].difficulty, 10);
        assert_eq!(science.diagnostics.len(), 3);
        let default = ScienceCatalog::from_entries(&[], &[json!({"name":"T", "identifier":"t"})])?;
        assert_eq!(default.technologies[0].difficulty, 10);
        assert!(
            ScienceCatalog::from_entries(
                &[],
                &[json!({"name":"T", "identifier":"t", "fields":["missing"]})]
            )
            .is_err()
        );
        Ok(())
    }
}
