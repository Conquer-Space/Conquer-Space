# Clang Format all the files in src and test

import os
import subprocess

dirs = ['src', 'test']
extensions = ['h', 'cpp']

def is_cpp_file(file):
    for k in extensions:
        if file.endswith('.' + k):
            return True
    return False

def format_file(file):
    print("Formatting", file)
    subprocess.run(['clang-format', '-i', file])

# Scans entire directory and runs clang format on it
def scan(dir):
    for path, subdirs, files in os.walk(dir):
        for name in files:
            pt = str(os.path.join(path, name))
            if not is_cpp_file(pt):
                continue
            # then run clang format
            print(pt)
            format_file(pt)


if __name__ == "__main__":
    for dir in dirs:
        scan(dir)
