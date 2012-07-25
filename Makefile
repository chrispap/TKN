# Project: TKN
# Makefile created by Chris Papapavlou

OBJ  = rs232.o TKN.o TKN_Main.o TKN_Boot.o TKN_Test.o $(RES)
LINKOBJ  = rs232.o TKN.o TKN_Main.o TKN_Boot.o  TKN_Test.o $(RES)
BIN  = TKN TKN_BOOT TKN_TEST
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

Test: TKN_Test.o TKN.o rs232.o
	$(CC) -o TKN_TEST TKN_Test.o TKN.o rs232.o


rs232.o: src/rs232.c
	$(CC) -c src/rs232.c -o rs232.o $(CFLAGS)

TKN.o: src/TKN.c
	$(CC) -c src/TKN.c -o TKN.o $(CFLAGS)

TKN_Main.o: src/TKN_Main.c
	$(CC) -c src/TKN_Main.c -o TKN_Main.o $(CFLAGS)

TKN_Boot.o: src/TKN_Boot.c
	$(CC) -c src/TKN_Boot.c -o TKN_Boot.o $(CFLAGS)

TKN_Test.o: src/TKN_Test.c
	$(CC) -c src/TKN_Test.c -o TKN_Test.o $(CFLAGS)
