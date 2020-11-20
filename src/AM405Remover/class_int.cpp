#include "class_int.h"

cint::cint()
{
	size=0;
	int_=NULL;
}

cint::cint(int s_int)
{
	size=1;
	int_=(int*)malloc( sizeof(int));
	int_[0]=s_int;
}
cint::~cint()
{
	if(int_) free(int_);
}

int& cint::operator[](int index)
{
	return int_[index];
}

int cint::length(void)
{
	return size;
}
void cint::append(int s_int)
{
	reserve(size + 1);
	int_[size-1]=s_int;
}

void cint::erase(int pos, int _size)
{
	if(_size<0) _size = size-pos;
	_size = min( (int)size-pos, _size);
	memset(int_+pos*sizeof(int), 0, _size*sizeof(int));
	memmove(&int_[pos], &int_[(pos+_size)], (size-_size)*sizeof(int));
	unsigned int new_size = size - _size;
	memrealloc(new_size);
}
void cint::insert(int pos, int s_int)
{
	pos = min(pos, (int)size);
	unsigned int old_size = size;
	reserve(size+1);
	memmove(&int_[(pos+1)], &int_[pos], (old_size-pos)*sizeof(int));
	int_[pos]=s_int;
}


void cint::reserve(int _size)
{
	if(_size>(int)size){
		size = _size;
		int_=(int*)realloc(int_, size*sizeof(int));
	}
}
void cint::memrealloc(int _size)
{
	if(_size!=(int)size){
		size = _size;
		int_=(int*)realloc(int_, size*sizeof(int));
	}
}
