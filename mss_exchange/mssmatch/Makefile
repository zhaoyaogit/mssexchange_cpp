#create by zyao 20180929 for meishisi exchange
# linux version
 
#2018-12-07:
#forum:http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80
#author:zhaoyao
#QQ Group:959372362
#email:myzhaoyao@126.com

.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++ -m64


HIREDIS_INC = -I ../hiredis
MSSJSON_INC = -I ../mssjson

INC_DIR = -I./ $(HIREDIS_INC) $(MSSJSON_INC)
LIB_DIR = -Llib 
CPPDEFS = -fPIC -DLINUX -DUNIX -DNDEBUG -DTHREAD_SAFE -Wall -std=c++11

.cpp.o:
	$(CPP) -c $(CPPDEFS) $(INC_DIR) $< -o $@ 

OBJ = mssmatch.o DataDispose.o msslog.o MssPubfunc.o

all : mss_exchange.lexe clean  

mss_exchange.lexe:$(OBJ)
	$(CPP) -g -O2 -o $@ $(LIB_DIR) $(CPPDEFS) $(OBJ) -ldl -pthread -lhiredis -ljsonpack
	mv $@ ../bin/

clean:
	rm *.o

