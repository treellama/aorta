#
# File:		Makefile for samples
# Author:	Robert Roebling
# Created:	1999
# Updated:	
# Copyright:	(c) 1998 Robert Roebling
#
# This makefile requires a Unix version of wxWindows
# to be installed on your system. This is most often
# done typing "make install" when using the complete
# sources of wxWindows or by installing the two
# RPM packages wxGTK.XXX.rpm and wxGTK-devel.XXX.rpm
# under Linux.

# this makefile may also be used as a template for simple makefiles for your
# own programs, the comments explain which parts of it you may have to modify
# to use it for your own programs


# you may change WX_CONFIG value to use a specific wx-config and to pass it
# some options, for example "--inplace" to use an uninstalled wxWindows
# version
#
# by default, wx-config from the PATH is used
WX_CONFIG := wx-config

# set this to the name of the main executable file
PROGRAM = aorta

# if your program has more than one source file, add more .o files to the line
# below
OBJECTS = $(PROGRAM).o image_ext.o imagdds.o DDSOptionsDialog.o filter.o


# you shouldn't have to edit anything below this line
CXX = $(shell $(WX_CONFIG) --cxx)

OPTFLAGS=-g -O2

.SUFFIXES:	.o .cpp

.cpp.o :
	$(CXX) $(OPTFLAGS) -c `$(WX_CONFIG) --cxxflags` -o $@ $<

.c.o:
	$(CC) $(OPTFLAGS) -c -o $@ $<

all:    $(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CXX) $(OPTFLAGS) -o $(PROGRAM) $(OBJECTS) `$(WX_CONFIG) --libs std` -lsquish

clean: 
	rm -f *.o $(PROGRAM)

aorta.o: aorta.cpp aorta.h DDSOptionsDialog.h image_ext.h imagdds.h
imagdds.o: imagdds.h filter.o
image_ext.o: image_ext.cpp image_ext.h
DDSOptionsDialog.o: DDSOptionsDialog.h aorta.h
# DO NOT DELETE
