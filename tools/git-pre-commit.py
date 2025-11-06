# You can create the file `git-hook.cfg` in the root directory to activate clang tidy for the hook.
# Just config it with:
# ```
# [CLANG_TIDY]
# clang_tidy = yes
# ```
# The hook will automatically fix any issues that show up
import subprocess
import sys
import clang_format
import configparser
import requests
import os

output = subprocess.run(['git', 'diff', '--cached', '--name-only', '--diff-filter=ACM'], stdout=subprocess.PIPE)
git_diff_list = output.stdout.decode('utf-8')

if len(git_diff_list) == 0:
    exit(0)

error = False

cpp_file_list = [s for s in git_diff_list.splitlines() if s is not None and clang_format.is_cpp_file(s)]
for line in cpp_file_list:
    # Clang format the files
    clang_format.format_file(line)

cfg = configparser.ConfigParser()
try:
    cfg.read('git-hook.cfg')
except:
    pass

RUN_CLANG_TIDY_URL='https://raw.githubusercontent.com/llvm/llvm-project/release/17.x/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py'
CLANG_TIDY_PATH='tools/run-clang-tidy.py'
if 'CLANG_TIDY' in cfg and 'clang_tidy' in cfg['CLANG_TIDY']:
    # We don't do linux for now
    if (cfg['CLANG_TIDY']['clang_tidy'] == 'yes' or cfg['CLANG_TIDY']['clang_tidy'] == 'true') and sys.platform == 'win32':
        print('Generating \'compile_commands.json\'')
        subprocess.run('"tools/makefile_gen.bat"')
        # run makefile
        if not os.path.exists(CLANG_TIDY_PATH):
            request = requests.get(RUN_CLANG_TIDY_URL)
            with open(CLANG_TIDY_PATH, 'wb') as f:
                f.write(request.content)
        # Now run clang-tidy fix

        # Get the git diff files
        if len(cpp_file_list) > 0:
            s = " ".join(cpp_file_list)
            print(f"Processing {s}")
            val = subprocess.run('python tools/run-clang-tidy.py -config-file=.clang-tidy -quiet -fix -p build-makefile ' + s)
            if val.returncode != 0:
                error = True
            # Also format file
            clang_format.format_file(cpp_file_list)

for line in cpp_file_list:
    subprocess.run(['git', 'add', line])

if error:
    sys.exit(1)
else:
    sys.exit(0)
