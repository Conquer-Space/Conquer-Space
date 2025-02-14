# Copyright (C) 2021 Conquer Space
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# This file checks src for definitions of functions in src
# Most of the defines are in luafunctions.cpp, so we'll read from there, but in the future, it
# would be helpful to read from other places, but we'll have to find a viable way to define
# functions a lot more fluidly
import re
import sys
import os
import os.path
import pathlib
sys.path.append(os.path.dirname(pathlib.Path(__file__).parent.resolve()))
from common.cqsproot import GetCqspRoot

cqsp_root = GetCqspRoot()
variables = ["goods",
            "core",
            "recipes",
            "generators",
            "events",
            "civilizations",
            "date",
            "terrain_colors",
            "fields",
            "technologies",
            "ImGui",
            "client",
            "interfaces",
            "ImGuiCond",
            "ImGuiMouseButton",
            "ImGuiSliderFlags",
            "ImGuiColorEditFlags",
            "ImGuiStyleVar",
            "ImGuiWindowFlags",
            "ImGuiChildFlags",
            "ImGuiItemFlags"]
REGISTER_FUNC_NAME = "REGISTER_FUNCTION"

def ParseFile(filename):
    global variables
    global cqsp_root
    global REGISTER_FUNC_NAME
    with open(cqsp_root + filename, "r") as f:
        data = f.read()
        # Parse data, look for 'REGISTER_FUNCTION' in the file
        # The register function marco looks like this:
        # REGISTER_FUNCTION(name, lambda)
        # But will probably need more arguments in the future for documentation and other things
        indexes = [m.start() for m in re.finditer(REGISTER_FUNC_NAME, data)]
        # Then read the next few characters for the name

        for ind in indexes:
            # Check if it's the macro definiton
            if data[ind + len(REGISTER_FUNC_NAME) + 1] == "\"":
                # Skip forward 2 characters for the bracket and open
                k = 2
                var_name = ""
                while data[ind + len(REGISTER_FUNC_NAME) + k] != "\"":
                    # Read the content
                    var_name += data[ind + len(REGISTER_FUNC_NAME) + k]
                    k += 1
                variables.append(var_name)

def WriteConfigFile():
    global variables
    global cqsp_root

    if not os.path.exists("temp"):
        os.makedirs("temp")
    with open(cqsp_root + "temp/luaconfig.lua", "w") as output:
        output.write("globals = {")
        # Write the varable list
        variables = ["\"" + element + "\"" for element in variables]
        output.write(", ".join(variables))
        output.write("}\n")
        output.write("unused = false")
        # Close file
    output.close()

def main():
    file_parse_list = ["src/common/scripting/luafunctions.cpp",
                       "src/client/scripting/clientscripting.cpp",
                       "src/common/scripting/luafunctions.cpp",
                       "src/client/scripting/clientuielements.cpp",
                       "src/client/scripting/clientscripting.cpp"]
    for file in file_parse_list:
        ParseFile(file)

    WriteConfigFile()

if __name__ == '__main__':
    main()
