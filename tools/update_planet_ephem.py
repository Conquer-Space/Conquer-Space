# Reads orbital information from NASA JPL Horizons database
# https://ssd.jpl.nasa.gov/horizons/ # API gateway
# https://ssd-api.jpl.nasa.gov/doc/horizons.html#command # Docs
# Information on the body information
# https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/Tutorials/pdf/individual_docs/06_naif_ids.pdf
# https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/FORTRAN/req/naif_ids.html

import requests
import re
import hjson
import argparse
import os.path

# Input format: a dict containing the conquer space identifier that you want to collect, and a 2 tuple
# contianing the body and the reference body of the thing
# parent of 10 is the sun

def read_planet(planet, parent=10):
    """
    Reads a planet from the horizons database and outputs it into a dict

    Arguments:
     - planet: the planet or body that you want to collect data from
     - parent: the reference body that you want to collect from
    """
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
    # Get the GM of the body too?
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

def planet_output_to_dict(disp):
    """
    Convert read_planet's format to conquer space's data format
    """
    orbit = {}
    orbit["eccentricity"] = disp["EC"]
    orbit["semi_major_axis"] = str(disp["A"]) + " km"
    orbit["M0"] = str(disp["TA"]) + " deg"
    orbit["inclination"] = str(disp["IN"]) + " deg"
    orbit["LAN"] = str(disp["OM"]) + " deg"
    orbit["arg_periapsis"] = str(disp["W"]) + " deg"
    return orbit

def update_planet_ephems(planet_list: dict, output_directory: str):
    # We just want to generate the various files
    # so check if the file exists
    # Ensure the directory exists
    if not os.path.exists(output_directory):
        return False
    if not os.path.isdir(output_directory):
        return False

    for planet_name, orbit in planet_list.items():
        # the key is the planet name and the value is the orbit
        # Now check if that file exists
        # If it does, read it and replace the orbit
        # Otherwise, generate new file
        planet_file_name = os.path.join(output_directory, f"{planet_name}.hjson")
        planet_data = {}
        print(planet_file_name)
        if os.path.exists(planet_file_name):
            # Read hjson file. get the first value within the dict because it will be in a list

            with open(planet_file_name, "r") as data_file:
                planet_data = hjson.load(data_file)

            if not isinstance(planet_data, list):
                print(f"The file {planet_file_name} must be a type of array!")
                planet_data = {}
                continue
            planet_data = planet_data[0]
        # Now set the orbit
        planet_data["identifier"] = planet_name
        planet_data["orbit"] = orbit
        # Write to file
        with open(planet_file_name, "w") as data_file:
            output = hjson.dumps([planet_data], indent=4)
            data_file.write(output)
            data_file.write("\n")

def update_planet_ephem_dir(input_dir, output_dir):
    # Now read the file
    with open(input_dir) as f:
        planet_list = hjson.loads(f.read())

    planet_ephems = {}
    for k, p in planet_list.items():
        disp = read_planet(p[0], p[1])
        orbit = planet_output_to_dict(disp)
        planet_ephems[k] = orbit

    update_planet_ephems(planet_ephems, output_dir)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="Planet Ephemeris Parser")
    parser.add_argument("-i", "--input", help="Input hjson file containing a dict, with the SPICE ID as the value", required=True)
    parser.add_argument("-o", "--output", help="Output directory", required=True)
    args = parser.parse_args()
    update_planet_ephem_dir(args.input, args.output)
