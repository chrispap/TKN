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
	THREADLIB = 
### For linux only
else
	THREADLIB = -lpthread 
endif

### ... ###
OBJ  =  TKN_Interactive.o TKN_Step.o TKN_Boot.o TKN_PacketCreator.o TKN_Elapse.o 
LINKOBJ = ../lib/rs232.o ../lib/TKN.o ../lib/TKN_Util.o ../lib/TKN_Queue.o 
BIN = TKN_Interactive TKN_Step TKN_Elapse  TKN_Boot 
CXXFLAGS = $(CXXINCS)  
CFLAGS = $(INCS) -Wall -O2  -g
RM = rm -f


### Actions ###
.PHONY: all

all: $(BIN)

cleanObj:
	${RM} $(OBJ) $(LINKOBJ)

clean:
	${RM} $(OBJ) $(LINKOBJ) $(BIN)


### Executables ###
TKN_Interactive: TKN_Interactive.o $(LINKOBJ)
	$(CC) -o TKN_Interactive TKN_Interactive.o $(LINKOBJ) $(THREADLIB)

TKN_Step: TKN_Step.o $(LINKOBJ)
	$(CC) -o TKN_Step TKN_Step.o $(LINKOBJ) $(THREADLIB)

TKN_Elapse: TKN_Elapse.o $(LINKOBJ)
	$(CC) -o TKN_Elapse TKN_Elapse.o $(LINKOBJ) $(THREADLIB) 

TKN_Boot: TKN_Boot.o $(LINKOBJ)
	$(CC) -o TKN_Boot TKN_Boot.o $(LINKOBJ) $(THREADLIB)


### Object files (for executables) ###
TKN_Interactive.o: TKN_Interactive.c
	$(CC) -c TKN_Interactive.c -o TKN_Interactive.o $(CFLAGS)

TKN_Step.o: TKN_Step.c
	$(CC) -c TKN_Step.c -o TKN_Step.o $(CFLAGS)

TKN_Elapse.o: TKN_Elapse.c
	$(CC) -c TKN_Elapse.c -o TKN_Elapse.o $(CFLAGS)
	
TKN_PacketCreator.o: TKN_PacketCreator.c
	$(CC) -c TKN_PacketCreator.c -o TKN_PacketCreator.o $(CFLAGS)

TKN_Boot.o: TKN_Boot.c
	$(CC) -c TKN_Boot.c -o TKN_Boot.o $(CFLAGS)