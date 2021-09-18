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
import os


with open("src/common/scripting/luafunctions.cpp", "r") as f:
    data = f.read()
    # Parse data, look for 'REGISTER_FUNCTION' in the file
    # The register function marco looks like this:
    # REGISTER_FUNCTION(name, lambda)
    # But will probably need more arguments in the future for documentation and other things
    func_name = "REGISTER_FUNCTION"
    indexes = [m.start() for m in re.finditer(func_name, data)]
    # Then read the next few characters for the name
    variables = []
    for ind in indexes:
        # Check if it's the macro definiton
        if data[ind + len(func_name) + 1] == "\"":
            # Skip forward 2 characters for the bracket and open
            k = 2
            var_name = ""
            while data[ind + len(func_name) + k] != "\"":
                # Read the content
                var_name += data[ind + len(func_name) + k]
                k += 1
            variables.append(var_name)
    # Write to file
    # Add other variables that are not functions. We will probably have to have a system in the
    # future to better and automatically determine these
    variables.append("goods")
    variables.append("recipes")
    variables.append("generators")
    variables.append("events")
    variables.append("civilizations")
    variables.append("date")

    print("Variables list:", variables)
    if not os.path.exists("temp"):
        os.makedirs("temp")
    with open("temp/luaconfig.lua", "w") as output:
        output.write("globals = {")
        # Write the varable list
        variables = ["\"" + element + "\"" for element in variables]
        output.write(", ".join(variables))
        output.write("}")
