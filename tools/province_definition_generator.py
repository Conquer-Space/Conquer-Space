import csv
import hjson

FILENAME = "binaries/data/core/data/planet_data/earth/map/definitions.csv"

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
            "country": country_name
        }
    )

# print(countries)
# Now split into files and output the 
# Now make the folders
FOLDER = "binaries/data/core/data/planet_data/earth/map/provinces"
for country, province_list in countries.items():
    with open(FOLDER + "/" + country + ".hjson", "w") as f:
        f.write(hjson.dumps(province_list, indent = 4))
