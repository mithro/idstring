all: main

main: main.cpp ../../idstring.h a50t_all_features.fasm
	g++ -o main -I../../ main.cpp

a50t_all_features.fasm: a50t_all_features.fasm.gz
	gunzip --keep $<

run: main
	/usr/bin/time -v ./main

.PHONY: run
