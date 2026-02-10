import hjson
import os
# read the file
# And then read our output files
output = hjson.load(open("output/market_log.hjson", "r"))
print(output)
# Read the market province window

def merge_province(province_dict, province_to_merge):
    province_dict["industry"] = province_to_merge

for root, subdirs, files in os.walk("binaries/data/core/data/planet_data/provinces"):
    for filename in files:
        country_contents = hjson.load(open(os.path.join(root, filename), "r"))
        for province_dict in country_contents:
            if province_dict["identifier"] not in output:
                continue
            merge_province(province_dict, output[province_dict["identifier"]])
        print(country_contents)
        # Write back to file
        f = open(os.path.join(root, filename), "w")
        hjson.dumps(country_contents, indent=4)
