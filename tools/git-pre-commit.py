import subprocess
import sys

output = subprocess.run(['git', 'diff', '--cached', '--name-only', '--diff-filter=ACM'], stdout=subprocess.PIPE)
out = output.stdout.decode('utf-8')

error = False
for line in out.splitlines():
    # Run cpplint for the files
    if (line is None):
        continue

    val = subprocess.run(['cpplint', '--quiet', '--linelength=120', '--recursive',
    '--filter=-readability/casting,-runtime/references,-build/c++11,-readability/check,-whitespace/comments,-runtime/int',
    '--verbose=0', line], stdout=subprocess.PIPE)
    if val.returncode != 0:
        error = True

if error:
    sys.exit(1)
else:
    sys.exit(0)
