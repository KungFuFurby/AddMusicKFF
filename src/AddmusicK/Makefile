CXX = g++
CXXFLAGS = -Wall -pedantic -std=c++17 -O3
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#CXXFLAGS += -I/opt/local/include
LDFLAGS = -ldl
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#LDFLAGS += -L /opt/local/lib/ -lboost_system -lboost_filesystem

SRCS = *.cpp asardll.c ../AM405Remover/AddMusic.cpp

all: addmusick

addmusick: $(SRCS) *.h
	$(CXX) $(CXXFLAGS) -c $(SRCS)
	$(CXX) -o addmusick *.o $(LDFLAGS)

clean:
	rm -rf *.o ./addmusick

.PHONY: all clean

#addmusick:
