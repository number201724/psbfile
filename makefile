psb.o : psb.cpp
	g++ -c common/psb.cpp -I"/usr/local/include" -o psb.o

decompiler.o: decompiler.cpp
	g++ -c  scenes/decompiler/decompiler.cpp -I"/usr/local/include" -o decompiler.o


decompiler : psb.o decompiler.o
	g++ -ljsoncpp psb.o decompiler.o -o decompiler -L"/usr/local/lib"
