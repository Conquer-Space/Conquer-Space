import requests
import hjson
import parse_tle
import sys
import argparse

tle_source = "https://celestrak.org/NORAD/elements/gp.php?GROUP=stations&FORMAT=tle"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="Satellite Parser")
    parser.add_argument("-i", "--input", default=tle_source, help="Input text file url of the TLE input")
    parser.add_argument("-o", "--output", default=None, help="Output source. If left empty will output to stdout")
    args = parser.parse_args()

    tle_source = args.input
    print("Reading from", tle_source)
    # Get all space stations
    content = requests.get(tle_source, stream=True).content
    lines = content.decode().splitlines()
    satellites = [parse_tle.parse_tle(lines[k:k+3]) for k in range(0, len(lines), 3)]
    # Also add the center bodies for each satellite?
    satellite_entities = []
    for sat in satellites:
        sat["reference"] = "earth"
        name = sat["name"]
        del sat["name"]
        di = {
            "name": name,
            "orbit": sat,
            "tags": []
        }
        satellite_entities.append(di)
    if args.output is None:
        print(hjson.dumps(satellite_entities))
    else:
        with open(args.output, "w") as f:
            hjson.dump(satellite_entities, f)
