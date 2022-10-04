# Read the satellite data
from cmath import pi
import math
import datetime
def get_year(line):
    if int(line) > 56:
        return 19
    else:
        return 20

with open("satellites.txt", "r") as file:
    lines = file.readlines()
    for k in range(int(len(lines)/3)):
        print(lines[k].strip())
        #print(lines[k + 1].strip())
        line_one = lines[k + 1].strip().split()
        # 	International Designator
        int_desig = line_one[2]
        year = get_year(int_desig[0:2])
        print("Launch year: {}{}".format(year, int_desig[0:2]))
        epoch = line_one[3]
        year = get_year(epoch[0:2]) * 100 + int(epoch[0:2])
        date = datetime.datetime(year, 1, 1) + datetime.timedelta(float(epoch[2:]) - 1)
        print("Epoch: {}".format(date))
        #print(lines[k + 2].strip())

        # get line two
        line_two = lines[k + 2].strip().split()
        e = float("0." + line_two[4])
        print("Inclination: {}".format(line_two[2]))
        print("LAN: {}".format(line_two[3]))
        print("Eccentricity: {}".format(e))
        print("w: {}".format(line_two[5]))
        print("Mean anomaly: {}".format(line_two[6]))
        mean_motion = float(line_two[7][:11])
        # Semi major axis
        T = (24 * 3600)/mean_motion
        mu = 3.9860044188e14
        a = (T**2*mu/(4.0*pi**2))**(1/3.0)
        # Focus

        print("Orbit time: {}".format(24/float(line_two[7][:11])))
        print("Semi major axis: {}km".format(a/1000))
        # Calculate apsis and periapsis
        earth_radius = 6371
        semi_minor = a * (1 - e**2) ** (1/2)
        c = (a**2 - semi_minor**2) ** (0.5)
        print("Semi minor: ", semi_minor/1000)

        print("Apoapsis:", (a + c)/1000 - earth_radius)
        print("Periapsis:", (a - c)/1000 - earth_radius)
        break
#20831903.6009
