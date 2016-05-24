CXXFLAGS = -Wall -Werror -Wno-write-strings -std=c++03 -pedantic-errors
#CXXFLAGS += -g
CXXFLAGS += $(shell pkg-config --cflags opencv)
LIBS += $(shell pkg-config --libs opencv)

debayer: debayer.o 
	$(CXX) -o ../../bin/debayer debayer.o $(CXXFLAGS) $(LIBS) 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f ../../bin/debayer

