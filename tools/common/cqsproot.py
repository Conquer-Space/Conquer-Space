import os

# Get conquer space base path, which should be the folder cqsp
# Can probably make this even more reliable if we change this to looking for a file in the root of
# the folder, but this is simpler to implement.

def GetCqspRoot():
    cqsp_root = os.getcwd()
    if "cqsp" not in cqsp_root:
        return ""
    else:
        cqsp_root = cqsp_root[:cqsp_root.rfind("cqsp")] + "cqsp/"
        return cqsp_root
