all:
	g++ -g -Wall -pthread -std=c++11 -fpic -Wl,-rpath=./  -L../../../out/lib/debug -lbase -lcommon -lrt *.cpp -o client.exe
	cp client.exe ../../../out/lib/debug/client.exe 
clean:
	rm -rf *.out
	rm -rf *.exe

cleanall:
	rm -rf *.out
	rm -rf *.exe
	rm -rf *.so*