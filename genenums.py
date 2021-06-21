import re
import os
from sys import argv

patt = re.compile(r"(\w*)\((\w*)\)\[(.*)\]")

folder = argv[1]

with open(f'{folder}/enum_definitions.impl', 'r') as f:
	lines = f.readlines()

try:
	os.mkdir(f'{folder}/GeneratedEnums')
except:
	pass

for line in filter(lambda x: x.strip() and x.strip()[0] != '#', lines):
	match = patt.match(line)
	if match:
		cls_name = match.group(1)
		cls_type = match.group(2)
		cls_members = match.group(3).split(',')
		with open(f'{folder}/GeneratedEnums/{cls_name}.h', 'w') as f:
			f.write('#pragma once\n#include "../Types.h"\n#include "../EnumHelpers.h"\nnamespace ARLib {\n')
			f.write(f'\tenum class {cls_name} : {cls_type} {{\n')
			for i, member in enumerate(cls_members):
				f.write(f'\t\t{member.strip()} = {1 << (i - 1) if i > 0 else 0},\n')
			f.write('\t};\n')
			f.write(f'\tMAKE_BITFIELD_ENUM({cls_name})\n')
			f.write('}\n')