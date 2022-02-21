#AddmusicK Makefile
#For use with Mac OS X and Linux
#NOTE: C++17 support is recommended to ensure your program compiles
#successfully!
#(for Windows, use Visual Studio 2019 instead to compile)

CXX = g++
CXXFLAGS = -Wall -pedantic -std=c++17 -O3
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#CXXFLAGS += -I/opt/local/include
LDFLAGS = -ldl
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#LDFLAGS += -L /opt/local/lib/ -lboost_system -lboost_filesystem

SRCS = src/AddmusicK/*.cpp src/AddmusicK/asardll.c src/AM405Remover/AddMusic.cpp

all: addmusick

addmusick: $(SRCS) src/AddmusicK/*.h
	cd src/AddmusicK; \
	$(CXX) $(CXXFLAGS) -c $(patsubst %,../../%,$(SRCS))
	$(CXX) -o addmusick src/AddmusicK/*.o $(LDFLAGS)


clean:
	rm -rf src/AddmusicK/*.o ./addmusick

.PHONY: all clean