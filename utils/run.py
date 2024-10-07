#######################################
# This script is used to compile and run
# the ./src/main.cc file.

import subprocess as sbp
import re

INC_FLAGS = "-I$AC_TYPES/include -I$AC_SIMUTILS/include"

def map_dtype(dtype):
  if dtype == "fp32":
    return "FP32"

  if dtype == "bf16":
    return "BF16"

  if len(re.findall(r'e[0-9]*m[0-9]*', dtype)) > 0:
    bits = re.findall(r'[0-9][0-9]|[0-9]', dtype)
    if len(bits) == 2:
      e, m = bits
      tot_bits = 1 + int(e) + int(m)

      return f"FP{tot_bits}_E{e}M{m}"

'''Todo: Add all design configurations'''
designs = [
  {"dname":"fpaRed"         , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_82"   , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_44"   , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_28"   , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_242"  , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_422"  , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_224"  , "sz":16, "dts": ["bf16"]},
  {"dname":"fpaRed_ol_2222" , "sz":16, "dts": ["bf16"]},
]

samples = 1

for design in designs:
  dname, input_sz, dtypes = design.values()

  for dtype in dtypes:
    print(map_dtype(dtype))

    sbp.call(f"g++ ../src/main.cc {INC_FLAGS} -Ddesign={dname} -D{map_dtype(dtype)} -Dinput_size={input_sz} -o main.elf", shell=True)
    sbp.call(f"./main.elf", shell=True)