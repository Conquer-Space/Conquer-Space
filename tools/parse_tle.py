# Read the satellite data
from cmath import pi
import math
import datetime
import hjson

def get_year(line):
    if int(line) > 56:
        return 19
    else:
        return 20

def parse_tle(satellite):
    # The satellite is an array of 3 strings for each line of the tle.
    lines = [s.strip() for s in satellite]
    satellite_name = lines[0].strip()
    line_one = lines[1].strip()
    line_two = lines[2]
    if line_one[0] != '1' or line_two[0] != '2':
        return None # Invalid TLE
    # print(satellite_name)
    # 	International Designator
    int_desig = line_one[9:17]
    year = get_year(int_desig[0:2])
    # print("Launch year: {}{}".format(year, int_desig[0:2]))
    epoch = line_one[18:33]
    year = get_year(epoch[0:2]) * 100 + int(epoch[0:2])
    date = datetime.datetime(year, 1, 1) + datetime.timedelta(float(epoch[2:]) - 1)
    # print("Epoch: {}".format(date))
    #print(lines[k + 2].strip())

    # # get line two
    # line_two = lines[k + 2].strip().split()
    inclination = float(line_two[8:16])
    raan = float(line_two[17:25])
    e = float("0." + line_two[26:33])
    w = float(line_two[34:42])
    mean_anomaly = float(line_two[43:51])
    # print("Inclination: {}".format(inclination))
    # print("LAN: {}".format(raan))
    # print("Eccentricity: {}".format(e))
    # print("w: {}".format(w))
    # print("Mean anomaly: {}".format(mean_anomaly))
    mean_motion = float(line_two[52:63])
    # # Semi major axis
    T = (24 * 3600)/mean_motion
    mu = 3.9860044188e14 # Gravitational constant m3 s-2
    # Semi major axis
    a = (T**2*mu/(4.0*pi**2))**(1/3.0)

    # print("Orbit time: {} hours".format(24/float(mean_motion)))
    # print("Semi major axis: {} km".format(a/1000))
    # # Calculate apsis and periapsis
    earth_radius = 6371
    semi_minor = a * (1 - e**2) ** (1/2)
    c = (a**2 - semi_minor**2) ** (0.5)
    # print("Semi minor: {} km".format(semi_minor/1000))

    # print("Apoapsis: {} km".format((a + c)/1000 - earth_radius))
    # print("Periapsis: {} km".format((a - c)/1000 - earth_radius))
    return {
        "name": satellite_name,
        "eccentricity": e,
        "semi_major_axis": a,
        "inclination": inclination,
        "LAN": raan,
        "arg_periapsis": w,
        "M0": mean_anomaly,
        "epoch": date.timestamp() # Time since 1970
    }

if __name__ == "__main__":
    with open("satellites.txt", "r") as file:
        lines = file.readlines()
        satellites = [parse_tle(lines[k:k+3]) for k in range(0, len(lines), 3)]
        print(hjson.dumps(satellites))
