import os
import shutil

# Install hook to .git folder
if not os.path.exists(".git/hooks"):
    print("Not a git folder. Please run this from the top level directory of the project")
    exit()

shutil.copy("tools/pre-commit", ".git/hooks/pre-commit")
print("Hook installed!")
