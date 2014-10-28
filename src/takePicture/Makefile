
CXXFLAGS = -Wall -Werror -Wno-write-strings -std=c++03 -pedantic-errors
CXXFLAGS += -g
CXXFLAGS += $(shell pkg-config --cflags opencv)
LIBS = -lueye_api
LIBS += $(shell pkg-config --libs opencv)

takePicture: takePicture.o
	$(CXX) -o ../../bin/takePicture takePicture.o $(CXXFLAGS) $(LIBS) 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f ../../bin/takePicture

