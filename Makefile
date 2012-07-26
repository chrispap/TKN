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
	CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 	
endif

### ... ###
OBJ  = rs232.o TKN.o TKN_Interactive.o TKN_Boot.o TKN_PacketCreator.o TKN_Util.o TKN_Elapse.o $(RES)
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS) -O2 -g
RM = rm -f

### Actions ###
all: Interactive Elapse PacketCreator Boot
.PHONY: all

clean: 
	${RM} $(OBJ)

### Executables ###
Interactive: TKN_Interactive.o TKN.o rs232.o TKN_Util.o
	$(CC) -o bin/TKN_Interactive TKN_Interactive.o TKN.o rs232.o TKN_Util.o

Elapse: TKN_Elapse.o TKN.o rs232.o TKN_Util.o
	$(CC) -o bin/TKN_Elapse TKN_Elapse.o TKN.o rs232.o TKN_Util.o

PacketCreator: TKN_PacketCreator.o TKN.o rs232.o
	$(CC) -o bin/TKN_PacketCreator TKN_PacketCreator.o TKN.o rs232.o

Boot: TKN_Boot.o TKN.o rs232.o TKN_Util.o
	$(CC) -o bin/TKN_Boot TKN_Boot.o TKN.o rs232.o TKN_Util.o

### Object files ###
rs232.o: src/rs232.c src/rs232.h
	$(CC) -c src/rs232.c -o rs232.o $(CFLAGS)

TKN.o: src/TKN.c src/TKN.h
	$(CC) -c src/TKN.c -o TKN.o $(CFLAGS)

TKN_Util.o: src/TKN_Util.c src/TKN_Util.h
	$(CC) -c src/TKN_Util.c -o TKN_Util.o $(CFLAGS)

### Object files (for executables) ###
TKN_Interactive.o: src/TKN_Interactive.c
	$(CC) -c src/TKN_Interactive.c -o TKN_Interactive.o $(CFLAGS)

TKN_Elapse.o: src/TKN_Elapse.c
	$(CC) -c src/TKN_Elapse.c -o TKN_Elapse.o $(CFLAGS)
	
TKN_PacketCreator.o: src/TKN_PacketCreator.c
	$(CC) -c src/TKN_PacketCreator.c -o TKN_PacketCreator.o $(CFLAGS)

TKN_Boot.o: src/TKN_Boot.c
	$(CC) -c src/TKN_Boot.c -o TKN_Boot.o $(CFLAGS)
