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

ifeq ($(OS),Windows_NT)
#Windows setting (made for MinGW, though in hindsight this may be a bad call)...
	CXX = x86_64-w64-mingw32-g++
else
	CXX = g++
endif

CXXFLAGS = -Wall -pedantic -std=c++17 -Os
#Commented out for now
#with libboost (this specifically targets MacPorts inclusions)
#CXXFLAGS += -I/opt/local/include

ifeq ($(OS),Windows_NT)
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

AM4SRCS = src/AM4Batch/*.cpp

AMMSRCS = src/AMMBatch/*.cpp

all: addmusick

addmusick: $(SRCS) src/AddmusicK/*.h
	cd src/AddmusicK; \
	$(CXX) $(CXXFLAGS) -c $(patsubst %,../../%,$(SRCS))
	$(CXX) -o AddmusicK src/AddmusicK/*.o $(LDFLAGS)
	cd src/AM4Batch; \
	$(CXX) $(CXXFLAGS) -c $(patsubst %,../../%,$(AM4SRCS))
	$(CXX) -o AM4Batch src/AM4Batch/*.o $(LDFLAGS)
	cd src/AMMBatch; \
	$(CXX) $(CXXFLAGS) -c $(patsubst %,../../%,$(AMMSRCS))
	$(CXX) -o AMMBatch src/AMMBatch/*.o $(LDFLAGS)

clean:
	rm -rf src/AddmusicK/*.o ./addmusick

.PHONY: all clean
