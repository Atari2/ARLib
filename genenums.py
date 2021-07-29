import re
import os
from sys import argv
import math

patt = re.compile(r"(\w*)(<>)?\((\w*)\)\[(.*)]")

folder = argv[1]

print("Generating enums")

with open(f'{folder}/enum_definitions.impl', 'r') as f:
    lines = f.readlines()

try:
    os.mkdir(f'{folder}/GeneratedEnums')
except Exception as exc:
    print(str(exc))


def power_bit_length(x):
    return 2 ** (x - 1).bit_length()


for line in filter(lambda x: x.strip() and x.strip()[0] != '#', lines):
    match = patt.match(line)
    if match:
        cls_name = match.group(1)
        is_not_bitfield = match.group(2)
        cls_type = match.group(3)
        cls_members = match.group(4).split(',')
        if not cls_type:
            required_bytes = math.ceil(len(cls_members) / 8)
            if required_bytes > 8:
                raise ValueError("Can't fit these many members in 8 bytes (which is the max)")
            cls_type = f'uint{power_bit_length(required_bytes) * 8}_t'
        with open(f'{folder}/GeneratedEnums/{cls_name}.h', 'w') as f:
            print(f"Generating {cls_name}")
            f.write('#pragma once\n#include "../Types.h"\n#include "../EnumHelpers.h"\nnamespace ARLib {\n')
            f.write(f'\tenum class {cls_name} : {cls_type} {{\n')
            for i, member in enumerate(cls_members):
                f.write(f'\t\t{member.strip()} = {1 << (i - 1) if i > 0 else 0},\n')
            f.write('\t};\n')
            if not is_not_bitfield:
                f.write(f'\tMAKE_BITFIELD_ENUM({cls_name})\n')
            f.write('}\n')

print("Generated enums")
