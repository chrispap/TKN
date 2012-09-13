#########################################
# Project: TKN							#
# Makefile created by Chris Papapavlou  #
#########################################

### Specifically for win ###
ifdef SystemRoot
	#windows compilers
	CPP  = g++.exe
	CC   = gcc.exe
	WINDRES = windres.exe
	#windows paths
	INCS =  -I"C:/Dev-Cpp/include" 
	CXXINCS =  -I"C:/Dev-Cpp/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 	
	THREADLIB = 
### For linux only
else
	THREADLIB = -lpthread 
endif


### ... ###
OBJ = rs232.o TKN.o TKN_Util.o TKN_Queue.o ListSerial.o
LINKOBJ = 
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS) -Wall -O2  -g
RM = rm -f


### Actions ###
.PHONY: all

all: $(OBJ)

cleanObj:
	${RM} $(OBJ) $(LINKOBJ)

clean:
	${RM} $(OBJ) $(LINKOBJ) $(BIN)


### Object files ###
rs232.o: rs232.c rs232.h
	$(CC) -c rs232.c -o rs232.o $(CFLAGS)

TKN.o: TKN.c TKN.h
	$(CC) -c TKN.c -o TKN.o $(CFLAGS)

TKN_Util.o: TKN_Util.c TKN_Util.h
	$(CC) -c TKN_Util.c -o TKN_Util.o $(CFLAGS)

TKN_Queue.o: TKN_Queue.c TKN_Queue.h
	$(CC) -c TKN_Queue.c -o TKN_Queue.o $(CFLAGS)

ListSerial.o: ListSerial.c
	$(CC) -c ListSerial.c -o ListSerial.o $(CFLAGS)
