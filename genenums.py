import re
import os
from sys import argv
import math

patt = re.compile(r"(\w*)(<>)?\((\w*)\)\[(.*)](F)?")

folder = argv[1]

print("Generating enums")

with open(f'{folder}/enum_definitions.impl', 'r') as f:
    lines = f.readlines()

try:
    os.mkdir(f'{folder}/include/GeneratedEnums')
except Exception as exc:
    pass


def power_bit_length(x):
    return 2 ** (x - 1).bit_length()


for line in filter(lambda x: x.strip() and x.strip()[0] != '#', lines):
    match = patt.match(line)
    if match:
        cls_name = match.group(1)
        is_bitfield = match.group(2)
        cls_type = match.group(3)
        cls_members = match.group(4).split(',')
        is_fancy = match.group(5)
        if not cls_type:
            required_bytes = math.ceil(len(cls_members) / 8)
            if required_bytes > 8:
                raise ValueError("Can't fit this many members in 8 bytes (which is the max)")
            cls_type = f'uint{power_bit_length(required_bytes) * 8}_t'
        with open(f'{folder}/include/GeneratedEnums/{cls_name}.hpp', 'w') as f:
            print(f"Generating {cls_name}")
            f.write('#pragma once\n#include "Types.hpp"\n#include "EnumHelpers.hpp"\nnamespace ARLib {\n')
            if is_fancy:
                f.write(f'\tMAKE_FANCY_ENUM({cls_name},{cls_type},\n')
            else:
                f.write(f'\tenum class {cls_name} : {cls_type} {{\n')
            for i, member in enumerate(cls_members):
                f.write(f'\t\t{member.strip()} = {1 << (i - 1) if i > 0 else 0},\n')
            if is_fancy:
                f.write('\t)\n')
            else:
                f.write('\t};\n')
            if is_bitfield:
                f.write(f'\tMAKE_BITFIELD_ENUM({cls_name})\n')
            f.write('}\n')

print("Generated enums")
