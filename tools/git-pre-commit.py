import subprocess
import sys
import clang_format

output = subprocess.run(['git', 'diff', '--cached', '--name-only', '--diff-filter=ACM'], stdout=subprocess.PIPE)
out = output.stdout.decode('utf-8')

error = False
for line in out.splitlines():
    # Run cpplint for the files
    if (line is None):
        continue
    if not clang_format.is_cpp_file(line):
        continue
    # Needs to be cpp file
    # Clang format the files
    clang_format.format_file(line)
    # Add file
    subprocess.run(['git', 'add', line])

    val = subprocess.run(['cpplint', '--quiet', '--recursive', line], stdout=subprocess.PIPE)
    if val.returncode != 0:
        error = True

if error:
    sys.exit(1)
else:
    sys.exit(0)
