#!/bin/sh -f

[ -x /assig/cl1/generaid ] && /assig/cl1/generaid 1000111111

tar cf entrega.tar identificador cl.g codegen.cc codegen.hh semantic.cc semantic.hh ptype.cc ptype.hh symtab.cc symtab.hh
