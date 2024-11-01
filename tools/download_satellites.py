import requests
import hjson
import parse_tle
import sys

if __name__ == "__main__":
    tle_source = "https://celestrak.org/NORAD/elements/gp.php?GROUP=stations&FORMAT=tle"
    if len(sys.argv) > 1:
        tle_source = sys.argv[1]
    # Get all space stations
    content = requests.get(tle_source, stream=True).content
    lines = content.decode().splitlines()
    print(lines)
    satellites = [parse_tle.parse_tle(lines[k:k+3]) for k in range(0, len(lines), 3)]
    print(hjson.dumps(satellites))

