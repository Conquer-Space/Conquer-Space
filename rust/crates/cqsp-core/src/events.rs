//! Native replacement for EventQueue/SysEvent; the bundled tutorial remains unregistered.
use anyhow::{Context, Result, ensure};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Effect {
    TutorialStage(u8),
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Choice {
    pub name: String,
    pub tooltip: String,
    pub effect: Option<Effect>,
}
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Notification {
    pub title: String,
    pub content: String,
    pub image: String,
    pub choices: Vec<Choice>,
}
impl Notification {
    /// Explicit fixtures/native callers only: default.lua disables tutorial registration.
    pub fn tutorial(stage: u8) -> Option<Self> {
        let (title, content) = match stage {
            0 => (
                "Our Future is in the Stars",
                concat!(
                    "Since antiquity, our ancestors have always looked to ",
                    "the stars for guidance, inspiration, and hope. But now, ",
                    "we have finally reached the technological advancement to touch the stars. ",
                    "This promises to bring untold changes to our culture, economy, and technology. ",
                    "This will change our future forever. For the first time in history, we ",
                    "will no longer solely reside on Earth, and will be an interplanetary civilization."
                ),
            ),
            1 => (
                "Conquer The Stars 1",
                "We need the technology to conquer the stars. First, let's research stuff.",
            ),
            _ => return None,
        };
        Some(Self {
            title: title.into(),
            content: content.into(),
            image: "core:rocket-event".into(),
            choices: vec![Choice {
                name: "Begin!".into(),
                tooltip: "Starts the \"Conquer The Stars\" event chain".into(),
                effect: Some(Effect::TutorialStage(stage + 1)),
            }],
        })
    }
}
#[derive(Default, Debug)]
pub struct EventQueue(Vec<Notification>);
impl EventQueue {
    pub fn front(&self) -> Option<&Notification> {
        self.0.first()
    }
    pub fn len(&self) -> usize {
        self.0.len()
    }
    pub fn is_empty(&self) -> bool {
        self.0.is_empty()
    }
    pub fn push(&mut self, event: Notification) {
        self.0.push(event);
    }
    /// Execute before clearing, including events the chosen effect itself appends.
    pub fn choose(
        &mut self,
        choice: Option<usize>,
        execute: impl FnOnce(Effect, &mut Self),
    ) -> Result<()> {
        let event = self.front().context("No event to acknowledge")?;
        let effect = match choice {
            None => {
                ensure!(event.choices.is_empty(), "Choose an event action first");
                None
            }
            Some(index) => {
                event
                    .choices
                    .get(index)
                    .context("Unknown event action")?
                    .effect
            }
        };
        if let Some(effect) = effect {
            execute(effect, self);
        }
        self.0.clear(); // Original SysEvent clears the whole queue, not just its front.
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn notice() -> Notification {
        Notification {
            title: "Notice".into(),
            content: "No effects".into(),
            image: String::new(),
            choices: vec![],
        }
    }
    #[test]
    fn acknowledgements_clear_all_events_after_the_selected_effect_including_newly_pushed_events()
    -> Result<()> {
        let mut queue = EventQueue::default();
        queue.push(Notification::tutorial(0).unwrap());
        queue.push(notice());
        let mut stage = 0;
        queue.choose(Some(0), |Effect::TutorialStage(value), queue| {
            assert_eq!(queue.len(), 2);
            stage = value;
            queue.push(notice());
        })?;
        assert_eq!(stage, 1);
        assert!(queue.is_empty());
        queue.push(notice());
        queue.push(Notification::tutorial(1).unwrap());
        queue.choose(None, |_, _| panic!("A notice has no effect"))?;
        assert!(queue.is_empty());
        let mut event = notice();
        event.choices = vec![Choice {
            name: "Dismiss".into(),
            tooltip: String::new(),
            effect: None,
        }];
        queue.push(event);
        queue.choose(Some(0), |_, _| panic!("This action has no effect"))?;
        assert!(queue.is_empty());
        Ok(())
    }
    #[test]
    fn invalid_choices_do_not_execute_clear_or_mutate_the_queue() {
        let mut queue = EventQueue::default();
        assert!(queue.choose(None, |_, _| panic!()).is_err());
        let event = Notification::tutorial(0).unwrap();
        queue.push(event.clone());
        for choice in [None, Some(1), Some(usize::MAX)] {
            assert!(queue.choose(choice, |_, _| panic!()).is_err());
            assert_eq!(queue.len(), 1);
            assert_eq!(queue.front(), Some(&event));
        }
    }
    #[test]
    fn native_tutorial_preserves_source_content_and_disabled_registration() {
        let source = include_str!("../../../../binaries/data/core/scripts/events/default.lua");
        assert!(source.contains("-- events:insert(starting_event)"));
        assert!(
            !source
                .lines()
                .any(|line| line.trim_start().starts_with("events:insert"))
        );
        for stage in [0, 1] {
            let event = Notification::tutorial(stage).unwrap();
            assert!(source.contains(&format!("title = \"{}\"", event.title)));
            let content: String = source
                .split(&format!("title = \"{}\"", event.title))
                .nth(1)
                .unwrap()
                .split("actions =")
                .next()
                .unwrap()
                .split('"')
                .enumerate()
                .filter(|(index, _)| index % 2 == 1)
                .map(|(_, text)| text)
                .collect();
            assert_eq!(event.content, content);
            assert_eq!(
                event.choices[0].effect,
                Some(Effect::TutorialStage(stage + 1))
            );
        }
        assert!(Notification::tutorial(2).is_none());
    }
}
