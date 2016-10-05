# Make file for testing the pr_task and pr_taskset classess
CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=test.cpp 
OBJS=$(subst .cpp,.o,$(SRCS))

all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o test $(OBJS) 
	#$(LDLIBS) 

test.o: test.cpp pr_task.hpp pr_task.cpp pr_taskset.hpp pr_taskset.cpp

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) test
