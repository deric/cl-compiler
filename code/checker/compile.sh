#! /bin/sh -f
antlr -gt cl.g
dlg -ci parser.dlg scan.c
g++ -Wno-write-strings -I/usr/local/pccts-1.33/include -I/usr/include/pccts -o cl cl.c symtab.cc semantic.cc codegen.cc codegest.cc ptype.cc scan.c err.c
