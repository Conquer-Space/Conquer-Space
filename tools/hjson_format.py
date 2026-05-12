import hjson
import os
import re
import sys

def format_file(file):
    with open(file, "r") as f:
        output = hjson.load(f)
    result = hjson.dumps(output, indent=4)
    result = re.sub(r':\n\s*(\{|\[)', r': \1', result)
    with open(file, "w") as f:
        f.write(result)

for root, subFolders, files in os.walk(os.path.join("binaries", "data")):
    for file in files:
        if not file.endswith(".hjson"):
            continue
        file_name = os.path.join(root, file)
        format_file(file_name)
