# Clang Format all the files in src and test

import os
import subprocess

dirs = ['src', 'test']
extensions = ['h', 'cpp']

def scan(dir):
    for path, subdirs, files in os.walk(dir):
        for name in files:
            pt = str(os.path.join(path, name))
            to_break = True
            for k in extensions:
                if pt.endswith('.' + k):
                    to_break = False
            if to_break:
                continue
            # then run clang format
            print(pt)
            subprocess.run(['clang-format', '-i', pt])


if __name__ == "__main__":
    for dir in dirs:
        scan(dir)
