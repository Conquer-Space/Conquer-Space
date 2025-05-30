import requests
import hjson
import parse_tle
import argparse

tle_source = "https://celestrak.org/NORAD/elements/gp.php?GROUP=stations&FORMAT=tle"

def download_satellites(tle_source, tags, output, model):
    content = requests.get(tle_source, stream=True).content
    lines = content.decode().splitlines()
    satellites = [parse_tle.parse_tle(lines[k:k+3]) for k in range(0, len(lines), 3)]
    # Also add the center bodies for each satellite?
    satellite_entities = []
    for sat in satellites:
        name = sat["name"]
        del sat["name"]
        # Add units to all tags
        orbit = {
            "eccentricity": sat["eccentricity"],
            "semi_major_axis": f"{sat["semi_major_axis"]} m",
            "inclination": f"{sat["semi_major_axis"]} deg",
            "LAN": f"{sat["LAN"]} deg",
            "arg_periapsis": f"{sat["arg_periapsis"]} deg",
            "M0": f"{sat["M0"]} deg",
            "epoch": sat["epoch"], # Time since 1970
            "reference": "earth"
        }
        di = {
            "name": name,
            "orbit": orbit,
            "tags": tags
        }
        if model:
            di["model"] = model
        satellite_entities.append(di)
    if output is None:
        print(hjson.dumps(satellite_entities))
    else:
        with open(output, "w") as f:
            hjson.dump(satellite_entities, f)

def process_arb_json(json_input: dict | list):
    # Now process the json file
    # If it's an array then we process the array one by one, if it's not then we process the entire thing
    if isinstance(json_input, list):
        for dictionary in json_input:
            process_json_dict(dictionary)
    else:
        process_json_dict(json_input)

def process_json_dict(json_dict: dict):
    download_satellites(json_dict["input"], json_dict["tags"], json_dict["output"], json_dict["model"])

# Ran the following command line arguments:
# -i "https://celestrak.org/NORAD/elements/gp.php?GROUP=gps-ops&FORMAT=tle" -t "navigation gps"
#       -o ../binaries/data/core/data/planet_data/earth/satellites/NAVSTAR.hjson
# -i "https://celestrak.org/NORAD/elements/gp.php?GROUP=galileo&FORMAT=tle" -t "navigation galileo"
#       -o ../binaries/data/core/data/planet_data/earth/satellites/galileo.hjson
# -i "https://celestrak.org/NORAD/elements/gp.php?GROUP=stations&FORMAT=tle" -t "station"
#       -o ../binaries/data/core/data/planet_data/earth/satellites/stations.hjson
if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="Satellite Parser")
    parser.add_argument("-i", "--input", default=tle_source, help="Input text file url of the TLE input")
    parser.add_argument("-o", "--output", default=None, help="Output source. If left empty will output to stdout")
    parser.add_argument("-t", "--tags", default=None, help="Tags to add to each satellite")
    parser.add_argument("-m", "--model", default=None, help="Model to set each satellite")
    parser.add_argument("-j", "--hjson", default=None, help="Input hjson file name")
    args = parser.parse_args()

    if args.hjson is not None:
        # then unload json and parse
        with open(args.hjson) as f:
            input_text = f.read()
            process_arb_json(hjson.loads(input_text))

    tle_source = args.input
    tags = []
    if args.tags != None:
        tags = args.tags.split()
    print("Reading from", tle_source)
    download_satellites(tle_source, tags, args.output, args.model)
