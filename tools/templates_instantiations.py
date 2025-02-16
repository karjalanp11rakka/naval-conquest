#This file is designed to add explicit template class instantiations of the classes, so that template classes definition can be located in the .cpp file.
#This allows having the template classes definition outside the header file. It's currently a simple script and doesn't have advanced features.
#It is currently only used to add explicit template class instantiations of the classes in 'include/game/action.hpp' to the end of 'src/game/action.cpp'.

import os
from collections import deque
import re

#The following data can be edited if the usage is to be extended
TEMPLATE_CLASSES_DEFINE_PATHS = (("include", "game", "action.hpp"),)
TEMPLATE_CLASSES_USE_PATHS = ((("src", "game", "gridObject.cpp"),),)#Single template classes directory can have several use directories. 
TEMPLATE_CLASS_WRITE_PATHS = (("src", "game", "action.cpp"),)

assert(len(TEMPLATE_CLASSES_DEFINE_PATHS) == len(TEMPLATE_CLASSES_USE_PATHS) == len(TEMPLATE_CLASS_WRITE_PATHS))

COMMENT_FIRST_LINE = "// Generated with 'tools/templates_instantiations.py'"

class TemplateClass:
    def __init__(self, min_template_count, name = None):
        self.name = name
        self.min_template_count = min_template_count
    def __eq__(self, other):
        return self.name == other.name
    def __lt__(self, other):
        return len(self.name) > len(other.name) 
    def __hash__(self):
        return hash(self.name)

class TemplateClassInstantiation:
    def __init__(self, template_class, template_parameters):
        self.template_class = template_class
        self.template_parameters = template_parameters
    def __eq__(self, other):
        return other.template_class == self.template_class and other.template_parameters == self.template_parameters
    def __hash__(self):
        return hash((self.template_class, self.template_parameters))

def ProcessLine(line: str) -> str:
    comment_start_index = line.find("//")
    if(comment_start_index != -1):
        line = line[:comment_start_index]
    line = line.strip()
    return line

def getTemplateClassesData(file):
    returnValue = deque()
    istemplate = False
    for line in file:
        line = ProcessLine(line)
        if not line: continue
        if istemplate:
            istemplate = False
            name_start_index = line.find("class ")
            if name_start_index == -1:
                returnValue.popleft()
            else:
                name_start_index += len("class ")
                name_end_index = line.find(" ", name_start_index)
                returnValue[-1].name = line[name_start_index:name_end_index] if name_end_index != -1 else line[name_start_index:]

        if "template" in line:
            istemplate = True
            returnValue.append(TemplateClass(line.count(",") - line.count("=") + 1))
    return sorted(returnValue)

ROOT_DIRECTORY = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

math_regex = re.compile(r'[+\-*/]')

for index, define_path in enumerate(TEMPLATE_CLASSES_DEFINE_PATHS):
    #read template definitions and save the data as a list of TemplateClasses
    with open(os.path.join(ROOT_DIRECTORY, *define_path), "r") as define_file:
        template_classes_data = getTemplateClassesData(define_file)

    template_instantations = set()
    constant_variables = {}
    for use_path in TEMPLATE_CLASSES_USE_PATHS[index]:
        with open(os.path.join(ROOT_DIRECTORY, *use_path), "r") as use_file:
            for line in use_file:
                line = ProcessLine(line)
                if not line: continue
                #get template classes to be instantiated
                for template_class in template_classes_data:
                    #find all occurances of the current template class
                    template_class_use_indices = []
                    while True:
                        index = line.find(template_class.name)
                        if index == -1: break
                        template_class_use_indices.append(index)
                        line = line[:index] + ' ' * len(template_class.name) + line[index + len(template_class.name):]#avoid taking the same name multiple times

                    #save the instantiation template parameters
                    for template_class_use_index in template_class_use_indices:
                        instantiation_definition = "".join(line[template_class_use_index + len(template_class.name):].split())
                        if not instantiation_definition.startswith("<"): continue
                        instantiation_definition_end_index = instantiation_definition.find(">")
                        if instantiation_definition_end_index == -1: continue
                        instantiation_definition = instantiation_definition[1:instantiation_definition_end_index]
                        
                        instantiation_template_argument_count = instantiation_definition.count(",") + 1
                        if instantiation_template_argument_count < template_class.min_template_count: raise Exception("Template class usage cannot have less parameters than the template class definition: " + line)
                        template_instantations.add(TemplateClassInstantiation(template_class, instantiation_definition))
                #get constants
                use_file_line_words = line.split()
                if "#define" == use_file_line_words[0]:
                    constant_variables[use_file_line_words[1]] = use_file_line_words[2]
                elif ";" not in line: continue
                elif "using" in use_file_line_words:
                    equal_index = line.find('=')
                    if equal_index == -1: continue

                    variable_name = re.search(r"(\S+)(?=\s*$)", line[:equal_index]).group(0)
                    variable_value = re.search(r".+(?=;)", line[equal_index + 1:]).group(0).strip()
                    constant_variables[variable_name] = variable_value
                elif "const" in use_file_line_words or "constexpr" in use_file_line_words:
                    variable_name = ""
                    variable_value = ""
                    equal_index = line.find('=')
                    if equal_index != -1:
                        variable_name = re.search(r"(\S+)(?=\s*$)", line[:equal_index]).group(0)
                        variable_value = re.search(r".+(?=;)", line[equal_index + 1:]).group(0).strip()
                    else:
                        opening_curly_brace_index = line.find('{')
                        if(opening_curly_brace_index == -1): continue
                        
                        variable_name = re.search(r"(\S+)(?=\s*$)", line[:opening_curly_brace_index]).group(0)
                        variable_value = re.search(r".+(?=}\s*;)", line[opening_curly_brace_index + 1:]).group(0).strip()
                    constant_variables[variable_name] = variable_value

        with open(os.path.join(ROOT_DIRECTORY, *TEMPLATE_CLASS_WRITE_PATHS[index]), "r") as read_file:
            lines = read_file.readlines()
            index_to_clear = -1
            for i, line in enumerate(lines):
                if COMMENT_FIRST_LINE in line:
                    index_to_clear = i
                    break
            if index_to_clear != -1:
                lines = lines[:index_to_clear]
            else: lines.append("\n" * 3)

        lines.append(f"{COMMENT_FIRST_LINE}\n")
        lines.append("// Do not add or modify anything after these comments\n")

        final_template_instantiations = set()

        #replace constants with their matching values
        for template_inst in template_instantations:
            for constant_name in sorted(constant_variables, key=len, reverse=True):
                template_inst.template_parameters = template_inst.template_parameters.replace(constant_name, constant_variables[constant_name])
            expressions = template_inst.template_parameters.split(',')
            evaluated_values = []
            #try to evaluate mathematical expressions like 2+1
            for expr in expressions:
                if math_regex.search(expr):
                    evaluated_values.append(str(eval(expr)))
                else: evaluated_values.append(expr)
            template_inst = f"{template_inst.template_class.name}<{','.join(evaluated_values)}>"
            if template_inst in final_template_instantiations: continue
            final_template_instantiations.add(template_inst)
            #add template instantiation line
            lines.append(f"template class {template_inst};\n")

        #write
        with open(os.path.join(ROOT_DIRECTORY, *TEMPLATE_CLASS_WRITE_PATHS[index]), "w") as write_file:
            write_file.writelines(lines)

print("Template classes instantiation done")