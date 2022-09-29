#AddmusicK Makefile
#Please unzip src.zip first before using this makefile.
#For Mac and Linux, should work with a standard issue G++
#For Windows, you need MinGW_w64 to compile. Project files exist to compile
#this program under Visual Studio 2019 inside src.zip as an alternative
#method for compiling this program.
#NOTE: C++17 support is recommended to ensure your program compiles
#successfully!
#Use Visual Studio 2019 instead to compile AMKGUI (there is no Mac/Linux
#version at this time unless you're using Wine or some other emulation
#layer)

UNAME := $(shell uname -s)

$(info $UNAME is $(UNAME))

ifneq (,$(findstring MSYS,$(UNAME)))
#Windows setting (made for MinGW, though in hindsight this may be a bad call)...
	CXX = x86_64-w64-mingw32-g++
else
	CXX = g++
endif

CXXFLAGS = -Wall -pedantic -std=c++17 -Os
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#CXXFLAGS += -I/opt/local/include

ifneq (,$(findstring MSYS,$(UNAME)))
#Windows setting...
	LDFLAGS = -static -static-libgcc -static-libstdc++ -s
else
#Mac/Linux setting...
	LDFLAGS = -ldl
endif

#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#LDFLAGS += -L /opt/local/lib/ -lboost_system -lboost_filesystem

SRCS = src/AddmusicK/*.cpp src/AddmusicK/asardll.c src/AM405Remover/AddMusic.cpp

all: addmusick

addmusick: $(SRCS) src/AddmusicK/*.h
	cd src/AddmusicK; \
	$(CXX) $(CXXFLAGS) -c $(patsubst %,../../%,$(SRCS))
	$(CXX) -o AddmusicK src/AddmusicK/*.o $(LDFLAGS)


clean:
	rm -rf src/AddmusicK/*.o ./addmusick

.PHONY: all clean