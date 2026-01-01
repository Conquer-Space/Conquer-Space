# This was a tool used to convert csv province definitions into a hjson file.
# This is likely to be deprecated
import csv
import hjson

FILENAME = "binaries/data/core/map/moon/province_def.csv"

csv_reader = csv.reader(open(FILENAME))
countries = {}
for row in csv_reader:
    # So it's
    # Identifier, color r, color g, color b, then country
    country_name = row[4]
    if country_name not in countries:
        countries[country_name] = []

    # Then add the province?
    countries[country_name].append(
        {
            "identifier": row[0],
            "color": [
                int(row[1]), int(row[2]), int(row[3])
            ],
            "country": country_name,
            "planet": "moon"
        }
    )

# Now split into files by country and output the hjson
# Now make the folders
FOLDER = "binaries/data/core/data/planet_data/moon/map/provinces"
for country, province_list in countries.items():
    if not country:
        country = "not_claimed"
    with open(FOLDER + "/" + country + ".hjson", "w") as f:
        f.write(hjson.dumps(province_list, indent = 4))
