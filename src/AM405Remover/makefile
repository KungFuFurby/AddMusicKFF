CC = g++

CFLAGS = -O2

COMPILE = $(CC) $(CFLAGS) -c

OBJFILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

PROG_NAME = addmusic



all: compile
compile: $(OBJFILES)
	$(CC) -o $(PROG_NAME) $(OBJFILES)
%.o: %.cpp
	$(COMPILE) -o $@ $<
clean:
	rm -rf *.o
run:
	./$(PROG_NAME)
install:
	mv $(PROG_NAME) /usr/bin/
move:
	rm -rf ../$(PROG_NAME)
	mv $(PROG_NAME) ../$(PROG_NAME)
