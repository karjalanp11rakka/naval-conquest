#This file is designed to turn the assets into single header file ('include/assets.hpp'), which contains content of the files as std::string_views.

import os
import re
import ctypes

ROOT_DIRECTORY = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ASSETS_DIRECTORY = os.path.join(ROOT_DIRECTORY, "assets")
INCLUDE_DIRECTORY = os.path.join(ROOT_DIRECTORY, "include")

#initialise 'objLoader.iso' ('objLoader.cpp')
obj_loader_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'objLoader.so')
obj_loader = ctypes.CDLL(obj_loader_path)
obj_loader.loadFromObj.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
obj_loader.loadFromObj.restype = ctypes.c_char_p
obj_loader.free.argtypes = None
obj_loader.free.restype = None

paths = []
assets_strings = []
obj_assets_strings = []

for dirpath, _, filenames in os.walk(ASSETS_DIRECTORY):
    if(dirpath == ASSETS_DIRECTORY):
        continue
    for filename in filenames:
        full_path = os.path.join(dirpath, filename)
        paths.append(full_path)

for path in paths:
    with open(path, "r") as read_file:
        if path.endswith("obj"):
            name = os.path.relpath(path, ASSETS_DIRECTORY).replace(os.path.sep, "_").upper()
            name = name[:name.find(".")]
            obj_assets_strings.append(obj_loader.loadFromObj(str.encode(read_file.read()), str.encode(name)).decode('utf-8'))
        else:
            assets_strings.append(
            "\tinline constexpr std::string_view " 
            + os.path.relpath(path, ASSETS_DIRECTORY).replace(os.path.sep, "_").replace(".", "_").upper() 
            + " {\"" + 
            re.sub(r"(\\n+)([{};])|([{};])(\\n+)", r"\2\3", re.sub(r"\s*(=|\+|\*|/|,|;|<|>|\\n|\(|\))\s*", lambda m: m.group(0).strip(), " ".join(re.sub(r"\n+", "\n", read_file.read()).replace("\r\n", "\\n").replace("\n", "\\n").split())))
            + "\\0\"};\n")

with open(os.path.join(INCLUDE_DIRECTORY, "assets.hpp"), "w") as write_file:
    write_file.write("// Generated with 'tools/assets_converter.py'\n\n")
    write_file.write("#pragma once\n\n#include <string_view>\n#include <array>\n\nnamespace assets\n{\n")
    write_file.writelines(assets_strings)
    write_file.write("}\n")
    write_file.writelines(obj_assets_strings)

print("Asset generation done")