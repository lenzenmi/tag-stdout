CC=g++
CFLAGS=-c -Wall -O3 -static -std=c++11
LDFLAGS=-Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static
SOURCES=src/tag_stdout.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=bin/tag_stdout

all: $(SOURCES) $(EXECUTABLE) strip
	    
$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	    $(CC) $(CFLAGS) $< -o $@

.PHONY: strip clean

strip: $(EXECUTABLE)
	strip --strip-all $(EXECUTABLE)

clean: 
	rm $(EXECUTABLE)
	rm $(OBJECTS)
