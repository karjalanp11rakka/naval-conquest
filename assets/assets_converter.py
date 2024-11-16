#THIS FILE IS DESIGNED TO TURN THE ASSETS INTO SINGLE HEADER FILE, WHICH CONTAINS CONTENT OF THE FILES AS STRINGS

import os

paths = []
assets_strings = []

root_directory = os.path.dirname(os.path.abspath(__file__))
for dirpath, _, filenames in os.walk(root_directory):
    if(dirpath == root_directory):
        continue
    for filename in filenames:
        full_path = os.path.join(dirpath, filename)
        paths.append(full_path)

for path in paths:
    with open(path, "r") as read_file:
        assets_strings.append("\tinline const std::string " + os.path.relpath(path, root_directory).replace(os.path.sep, "_").replace(".", "_").upper() + " {\"" + ' '.join(read_file.read().replace("\n", "\\n").split()) + "\\0\"};\n")

with open(os.path.join(root_directory, "assets.hpp"), "w") as write_file:
    write_file.write("// GENERATED WITH https://github.com/karjalanp11rakka/3d-project/blob/b6d930e32fa5b06349dec2d44edc661a32d697a3/assets/assets_converter.py\n\n")
    write_file.write("#pragma once\n\n#include <string>\n\nnamespace assets\n{\n")
    write_file.writelines(assets_strings)
    write_file.write("}\n")