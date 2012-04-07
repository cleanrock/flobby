#!/bin/sh
# arg: unitsync_api.h

echo "Attributes (function pointers):"
# generate attribute definitions
#                  return  name   args
sed -n 's/^EXPORT(\(.*\)) \(.*\)(\(.*\));/\1 (*\2)(\3);/p' < $1


echo "BIND calls:"
# generate BIND lines
#                  return  name   args
sed -n 's/^EXPORT(\(.*\)) \(.*\)(\(.*\));/BIND(\2);/p' < $1
