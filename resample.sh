#!/bin/sh

COUNT=0
REDUCTION_FACTOR=300
while IFS="" read -r p || [ -n "$p" ]
do
  # printf '%d\n' "$COUNT"
  COUNT=$((COUNT+1))
  if [[ ${p::1} == "f" ]]
  then
      if [[ $((COUNT % REDUCTION_FACTOR)) == 0 ]]
      then
          echo $p >> cat_simple.obj;
      fi;
  else
      echo $p >> cat_simple.obj;
  fi
done < chickenjockey.obj
