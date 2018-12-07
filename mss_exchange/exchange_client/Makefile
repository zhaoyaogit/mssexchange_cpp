
# Linux version
.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++ 


INC_DIR = -I./ -I ../hiredis -I ../mssjson -I ../mssmatch

CPPDEFS = -fPIC -DLINUX -DUNIX -DNDEBUG -DTHREAD_SAFE -Wall

.cpp.o:
	$(CPP) -c $(CPPDEFS) $(INC_DIR) $< -o $@

OBJ = exchange_client.o 

all : exchange_client clean  

exchange_client:$(OBJ)
	$(CPP) -g -O2 -o $@ $(CPPDEFS) $(OBJ) -ldl -pthread -levent -lhiredis -ljsonpack
	#mv $@ $(HOME)/bin/

clean:
	rm *.o
