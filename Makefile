# Project: TKN
# Makefile created by Chris Papapavlou


#Specifically for win
ifdef SystemRoot
	#windows compilers
	CPP  = g++.exe
	CC   = gcc.exe
	WINDRES = windres.exe
	#windows paths
	INCS =  -I"C:/Dev-Cpp/include" 
	CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 	
endif


OBJ  = rs232.o TKN.o TKN_Main.o TKN_Boot.o TKN_CreatePackets.o $(RES)
LINKOBJ  = rs232.o TKN.o TKN_Main.o TKN_Boot.o  TKN_CreatePackets.o $(RES)
BIN  = TKN TKN_BOOT TKN_CreatePackets
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS) -O2 -g
RM = rm -f

all: Main Boot Test
.PHONY: all

cleanObj: 
	${RM} $(OBJ)

clean: 
	${RM} $(OBJ) $(BIN)

Main: TKN_Main.o TKN.o rs232.o
	$(CC) -o TKN TKN_Main.o TKN.o rs232.o

Boot: TKN_Boot.o TKN.o rs232.o
	$(CC) -o TKN_BOOT TKN_Boot.o TKN.o rs232.o

Test: TKN_CreatePackets.o TKN.o rs232.o
	$(CC) -o TKN_CreatePackets TKN_CreatePackets.o TKN.o rs232.o


rs232.o: src/rs232.c
	$(CC) -c src/rs232.c -o rs232.o $(CFLAGS)

TKN.o: src/TKN.c
	$(CC) -c src/TKN.c -o TKN.o $(CFLAGS)

TKN_Main.o: src/TKN_Main.c
	$(CC) -c src/TKN_Main.c -o TKN_Main.o $(CFLAGS)

TKN_Boot.o: src/TKN_Boot.c
	$(CC) -c src/TKN_Boot.c -o TKN_Boot.o $(CFLAGS)

TKN_CreatePackets.o: src/TKN_CreatePackets.c
	$(CC) -c src/TKN_CreatePackets.c -o TKN_CreatePackets.o $(CFLAGS)
