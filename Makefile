all:
	g++ -g3 idstring.cpp -o idstring && gdb -ex run ./idstring
