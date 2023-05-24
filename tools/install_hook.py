import os
import shutil

# Install hook to .git folder
if not os.path.exists(".git/hooks"):
    print("Not a git folder. Please run this from the top level directory of the project")
    exit()


# Check if they installed anything that they need
if shutil.which("cpplint") is None:
    print("Cpplint was not installed, run `pip install cpplint` for the hook to function correctly")
    exit()

if os.path.exists(".git/hooks/pre-commit"):
    print("Hook already exists, our custom hook is presumed to be installed")
    exit()

shutil.copy("tools/pre-commit", ".git/hooks/pre-commit")
print("Hook installed!")
