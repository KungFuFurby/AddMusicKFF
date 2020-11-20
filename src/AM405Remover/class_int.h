#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif


class cint{
private:
	int *int_;
	unsigned int size;

public:
	cint();
	cint(int s_int);

	~cint();

	int length(void);
	void append(int s_int);
	void erase(int pos = 0, int _size = -1);
	void insert(int pos, int s_int);

	int& operator[](int);
private:
	void reserve(int _size);
	void memrealloc(int _size);
};