# Reads orbital information from NASA JPL Horizons database
# https://ssd.jpl.nasa.gov/horizons/ # API gateway
# https://ssd-api.jpl.nasa.gov/doc/horizons.html#command # Docs
# Information on the body information
# https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/Tutorials/pdf/individual_docs/06_naif_ids.pdf
# https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/FORTRAN/req/naif_ids.html

import requests
import re
import hjson

# parent of 10 is the sun
def read_planet(planet, parent=10):
    payload = {
        "format": "json",
        "EPHEM_TYPE": "ELEMENTS",
        "COMMAND": planet,
        "START_TIME": "2000-01-01",
        "STOP_TIME": "2000-01-02",
        "STEP_SIZE": "2d",
        "OBJ_DATA": "NO",
        "CENTER": parent
    }
    # ?format=json&COMMAND='499'&OBJ_DATA='YES'&MAKE_EPHEM='YES'&EPHEM_TYPE='OBSERVER'&CENTER='500@399'&START_TIME='2006-01-01'&STOP_TIME='2006-01-20'&STEP_SIZE='1%20d'&QUANTITIES='1,9,20,23,24,29'
    api = requests.get("https://ssd.jpl.nasa.gov/api/horizons.api", payload)
    res = str(api.json()["result"])
    # Look for $$OE ad look for the end at $$EOE
    """
    JDTDB    Julian Day Number, Barycentric Dynamical Time
        EC     Eccentricity, e
        QR     Periapsis distance, q (km)
        IN     Inclination w.r.t X-Y plane, i (degrees)
        OM     Longitude of Ascending Node, OMEGA, (degrees)
        W      Argument of Perifocus, w (degrees)
        Tp     Time of periapsis (Julian Day Number)
        N      Mean motion, n (degrees/sec)
        MA     Mean anomaly, M (degrees)
        TA     True anomaly, nu (degrees)
        A      Semi-major axis, a (km)
        AD     Apoapsis distance (km)
        PR     Sidereal orbit period (sec)

    Sample:
    2451544.500000000 = A.D. 2000-Jan-01 00:00:00.0000 TDB 
    EC= 1.519356354936738E-02 QR= 1.466531779777597E+08 IN= 1.180412883841571E-02
    OM= 8.994459355558011E+00 W = 6.504439592195953E+01 Tp=  2451518.816485451069
    N = 1.149361846814009E-05 MA= 2.550497908151671E+01 TA= 2.626770133243085E+01
    A = 1.489157387174645E+08 AD= 1.511782994571693E+08 PR= 3.132172874868846E+07
    """

    # Make the struct, we need the variables
    orbital_params = {}
    params = (res[res.find("$$SOE") + len("$$SOE") + 1:res.find("$$EOE")])
    for start in re.finditer("=", params):
        id = params[start.start() - 2 : start.start()].strip()
        if id == "0":
            continue
        orb_param = float(params[start.end() + 1 : start.end() + 22].strip())
        orbital_params[id] = float(orb_param)
    return orbital_params
# Get the GM of the body too?
planet_list = {
    "mercury": (1, 10),
    "venus": (2, 10),
    "earth": (3, 10),
    "mars": (4, 10),
    "jupiter": (5, 10),
    "saturn": (6, 10),
    "uranus": (7, 10),
    "neptune": (8, 10),
    "pluto": (9, 10),
    "moon": (301, 3)
}

hj = hjson.loads(str(open("binaries/data/core/data/planets/planets.hjson").read()))
for k, p in planet_list.items():
    disp = read_planet(p[0], p[1])
    # Output into the list, somehow
    orbit = {}
    orbit["eccentricity"] = disp["EC"]
    orbit["semi_major_axis"] = str(disp["A"]) + " km"
    orbit["M0"] = str(disp["TA"]) + " deg"
    orbit["inclination"] = str(disp["IN"]) + " deg"
    orbit["LAN"] = str(disp["OM"]) + " deg"
    orbit["arg_periapsis"] = str(disp["W"]) + " deg"
    # Look for item in hjson
    for p in hj:
        if p["identifier"] == k:
            print(p["name"])
            p["orbit"] = orbit
            break

with open("out.hjson", "w") as f:
    f.write(hjson.dumps(hj, indent=4))