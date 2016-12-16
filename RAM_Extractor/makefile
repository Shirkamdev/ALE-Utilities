COMP=g++
#OPTS=-lm -lncurses -l ale
INCL= -I../ArcadeLearningEnvironment/src
OPTS=-lale -lncurses -pthread -std=c++11 -w
OBJECTS=StdPijo.o 
CPP_FILE=extractRAM.cpp
EXE=main

#extractRAM: $(OBJECTS) $(EXE).cpp
#	$(COMP) $(OBJECTS) $(OPTS) $(EXE).cpp -o $(EXE) 
extractRAM: $(OBJECTS) $(CPP_FILE)
	$(COMP) $(INCL) $(OBJECTS) $(CPP_FILE) -o $(EXE) $(OPTS)

StdPijo.o: StdPijo.cc

clean:
	rm -rf *.o *~

tar:
	tar cvzf ppoo.tgz *.cc *.h makefile readme.txt
