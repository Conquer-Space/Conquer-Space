import re
import datetime
import os
import glob

dirs = ["src", "test"]

# Open the file, replace the line?
copyright = f"Copyright (C) 2021-{datetime.date.today().year} Conquer Space"
#print(copyright)

def replace_file(data :str):
    # Get the index of the license
    # Match that, and then replace
    gex = re.search(r"Copyright \(C\) 2021(?:-[2][0][0-9]{2}|) Conquer Space", data)
    if gex is None:
        return None
    #print(data[gex.start():gex.end()])
    text = data[gex.start():gex.end()]
    data = data.replace(text, copyright)
    # Verify that the file starts with "Conquer Space"
    if not data.startswith("/* Conquer Space"):
        # Then rip we'll need to replace the stuff
        data = data[:2] + " Conquer Space" + data[2:]
    # Check the first couple of lines if they start with 
    data = re.sub(r"\n\*", "\n *", data)
    return data

def file(fn):
    with open(fn, "r") as f:
        data = f.read()
        data = replace_file(data)
        f.close()
    if data is None:
        return

    # Reopen file and rewrite?
    with open(fn, "w") as f:
        f.write(data)
        f.close()

for dir in dirs:
    for filepath in glob.glob(f"{dir}/**", recursive=True):
        if not (filepath.endswith(".cpp") or filepath.endswith(".h")):
            continue
        file(filepath)
