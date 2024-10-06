#! /bin/bash

#================================================================
# File: setup.sh
#
# Usage: source setup.sh
#
# This script will setup any dependency need by 
# this project.

# It will create local directories:
#    ./ac_types
#    ./ac_simutils
#================================================================

# Configure AC Datatypes
if [ ! -d ./ac_types ]; then
  echo "Downloading AC_Types..."
  git clone https://github.com/hlslibs/ac_types.git
fi
export AC_TYPES=$(pwd)/ac_types

# Configure AC Simutils
if [ ! -d ./ac_simutils ]; then
  echo "Downloading AC_Simutils..."
  git clone https://github.com/hlslibs/ac_simutils.git
fi
export AC_SIMUTILS=$(pwd)/ac_simutils

# TODO: Add python dependencies
