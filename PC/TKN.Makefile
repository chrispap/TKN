#########################################
# Project: TKN							#
# Makefile created by Chris Papapavlou  #
#########################################

.PHONY: all

all:
	cd lib; make all;
	cd console; make all

clean:
	cd lib; make clean;
	cd console; make clean;

cleanObj:
	cd lib; make cleanObj;
	cd console; make cleanObj;

