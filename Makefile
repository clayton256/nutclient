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
#

#WXDIR=/home/mark/Projects/wxWidgets-master/gtk-debug/
NUTBASE=nut-2.7.4
CC = gcc

PROGRAM = nutclient

OBJECTS = $(PROGRAM).o myhalt.o

# implementation

.SUFFIXES:	.o .cpp

.c.o :
	$(CC) -c -g -DMYTASKBARICON `wx-config --cflags` -I$(NUTBASE)/clients -I$(NUTBASE)/include -o $@ $<

.cpp.o :
	$(CC) -c -g -DMYTASKBARICON `wx-config --cxxflags` -I$(NUTBASE)/clients -I$(NUTBASE)/include -o $@ $<

all:    $(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CC) -g -o $(PROGRAM) $(OBJECTS) `wx-config --libs` $(NUTBASE)/common/.libs/libcommon.a $(NUTBASE)/clients/.libs/upsclient.o -lpthread -lstdc++

clean: 
	rm -f *.o $(PROGRAM)
