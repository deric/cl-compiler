#!/bin/sh

#compiling of Antlr

#antlr -gt example2.g
#dlg -ci parser.dlg scan.c 
#g++ -o example2 example2.c scan.c err.c 

if [ -z "$1" ]; then
  echo "usage: $0 file.g" >&2
  echo "Enter name of the file" >&2
  ls *.g
  echo -n "File : "
  read response
  if [ -n "$response" ]; then
      file=$response
  fi
else
 file=$1
fi


#execute the first command
antlr -gt $file".g"
dlg -ci parser.dlg scan.c
g++ -o $file $file".c" scan.c err.c -I /usr/include/pccts/
echo "Compiled"
