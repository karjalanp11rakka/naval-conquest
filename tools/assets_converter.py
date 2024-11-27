#This file is designed to turn the assets into single header file ('include/assets.hpp'), which contains content of the files as std::string_views.

import os
import re

paths = []
assets_strings = []

ROOT_DIRECTORY = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSETS_DIRECTORY = os.path.join(ROOT_DIRECTORY, "assets")
INCLUDE_DIRECTORY = os.path.join(ROOT_DIRECTORY, "include")

for dirpath, _, filenames in os.walk(ASSETS_DIRECTORY):
    if(dirpath == ASSETS_DIRECTORY):
        continue
    for filename in filenames:
        full_path = os.path.join(dirpath, filename)
        paths.append(full_path)

for path in paths:
    with open(path, "r") as read_file:
        assets_strings.append("\tinline constexpr std::string_view " 
        + os.path.relpath(path, ASSETS_DIRECTORY).replace(os.path.sep, "_").replace(".", "_").upper() 
        + " {\"" + 
        re.sub(r"(\\n+)([{};])|([{};])(\\n+)", r"\2\3", re.sub(r"\s*(=|\+|\*|/|,|;|<|>|\\n|\(|\))\s*", lambda m: m.group(0).strip(), " ".join(re.sub(r"\n+", "\n", read_file.read()).replace("\r\n", "\\n").replace("\n", "\\n").split())))
        + "\\0\"};\n")

with open(os.path.join(INCLUDE_DIRECTORY, "assets.hpp"), "w") as write_file:
    write_file.write("// Generated with 'tools/assets_converter.py'\n\n")
    write_file.write("#pragma once\n\n#include <string_view>\n\nnamespace assets\n{\n")
    write_file.writelines(assets_strings)
    write_file.write("}\n")

print("Asset generation done")