# Makefile template for shared library

CC=clang
CXX=clang++
RM=rm -f
INCDIR=-I../ -I../include
COMMON_FLAGS=$(INCDIR) -g -fPIC -Wall -Wextra -O2 -DPD
LDFLAGS=-g -shared -undefined dynamic_lookup
TARGET_LIB = $(NAME).pd_darwin # target lib
SRCS=$(NAME).cpp
OBJS=$(subst .cpp,.o,$(SRCS))
CFLAGS=$(COMMON_FLAGS) -std=c11
CXXFLAGS=$(COMMON_FLAGS) -std=c++11

all: ${TARGET_LIB} clean

${TARGET_LIB}: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET_LIB) $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend