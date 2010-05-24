#! /bin/sh -f

[ -e checker ] || mkdir checker
rm -rf checker/*
cp basemodules.tar checker
cp jp.tar checker
cp entrega.tar checker
cp infochecker.txt checker
cp avaluador.cc checker
cd checker
tar xf basemodules.tar
tar xf jp.tar
tar xf entrega.tar cl.g codegen.cc codegen.hh semantic.cc semantic.hh ptype.cc ptype.hh symtab.cc symtab.hh
./compile.sh
g++ -ansi -Wall -o avaluador avaluador.cc
touch outputexecution
./avaluador
