#######################################
# This script is used to pack all the
# compress files into a .tar.gz file

import subprocess as sbp
from argparse import ArgumentParser

# Specify all the files that need to be compressed
FILES2CPY = [
  "../src/main.cc",
  "../src/*.h",
  "../bert/*.txt"
]

if __name__ == "__main__":
  parser = ArgumentParser()
  parser.add_argument("dest_dir", help="The destination directory. E.g. running `python3 pack.py test` will generate `test.tar.gz` file.", default="hls")
  args = parser.parse_args()

  sbp.call(f"mkdir {args.dest_dir}", shell=True)

  for file in FILES2CPY:
    sbp.call(f"cp {file} {args.dest_dir}", shell=True)

  sbp.call(f"tar -czvf {args.dest_dir}.tar.gz {args.dest_dir}", shell=True)
  sbp.call(f"rm -rf {args.dest_dir}", shell=True)