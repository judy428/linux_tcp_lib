all:
	g++ -g -Wall -pthread -std=c++11 -fpic -Wl,-rpath=./ *.cpp -o server.exe -L../../../out/lib/debug -lbase -lcommon -lrt
	cp server.exe ../../../out/lib/debug/server.exe
clean:
	rm -rf *.out
	rm -rf *.exe

cleanall:
	rm -rf *.out
	rm -rf *.exe
	rm -rf *.so*