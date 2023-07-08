#!/bin/bash

MOTES=("mote-1" "mote-2" "mote-3")

for mote in "${MOTES[@]}"
do
  if [ -d "$mote" ]; then
    echo "Cleaning $mote"
    (cd "$mote" && make distclean)
  else
    echo "Directory $mote not found"
  fi
done
