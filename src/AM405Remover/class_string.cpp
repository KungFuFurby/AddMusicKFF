#include "class_string.h"

#if !defined(WIN32) || defined(__MINGW32__)
	#define strcpy_s(dest, num, src) strncpy((dest), (src), (num))
	#define strcat_s(dest, num, src) strncat((dest), (src), (num))
#endif

/********************************************
constructor and deconstructor
********************************************/
cstring::cstring()
{
	size=0;
	string=(char *)malloc( size + 1);
	string[0]=0;
}
cstring::cstring(const char c)
{
	size=1;
	string=(char *)malloc( size + 1);
	string[0]=c;
	string[size]=0;
}
cstring::cstring(const char c, unsigned int _size)
{
	size=_size;
	string=(char *)malloc(size+1);
	memset(string, c, size);
	string[size]=0;
}
cstring::cstring(const char *source)
{
	size=strlen(source);
	string=(char *)malloc( size + 1);
	strcpy_s(string, size+1, source);
}
cstring::cstring(cstring& source)
{
	size=source.size;
	string=(char *)malloc(size+1);
	strcpy_s(string, size+1, source.c_str());
}

cstring::~cstring()
{
	if(string) free(string);
}

/********************************************
routine
********************************************/
char * cstring::c_str(unsigned int index) { return &string[index]; }
unsigned int cstring::length(void) { return size; }
/*******************************************/
/*******************************************/
void cstring::append(char c)
{
	*this += c;
}
void cstring::append(const char *source, int _size)
{
	if(!_size) return;
	if(_size<0) _size=strlen(source);
	cstring tmp(source);
	tmp[_size]=0;
	*this += tmp;
}
void cstring::append(cstring&source, int _size)
{
	if(!_size) return;
	if(_size<0) _size=source.length();
	cstring tmp(source);
	tmp[_size]=0;
	*this += tmp;
}
void cstring::append(short dw)
{
	reserve(size+2);
	*(short*)&string[size-2] = dw;
}
void cstring::append(int dd)
{
	reserve(size+4);
	*(int*)&string[size-4] = dd;
}

/*******************************************/
/*******************************************/
void cstring::erase(int pos, unsigned int _size)
{
	if(_size<0) _size=size-pos;
	_size = min(size-pos, _size);
	memset(this->c_str(pos), 0, _size);
	memmove(this->c_str(pos), this->c_str(pos+_size), size-(pos+_size));
	unsigned int new_size = size - _size;
	setmemory(new_size);
}

/*******************************************/
/*******************************************/
void cstring::insert(int pos, const char c)
{
	pos = min((unsigned int)pos, size);
	unsigned int old_size = size;
	reserve(size+1);
	memmove(this->c_str(pos+1), this->c_str(pos), old_size-pos);
	string[pos]=c;
}
void cstring::insert(int pos, const char *source, unsigned int _size)
{
	pos = min((unsigned int)pos, size);
	unsigned int old_size = size;
	reserve(size+_size);
	memmove(this->c_str(pos+_size), this->c_str(pos), old_size-pos);
	memmove(this->c_str(pos), source, _size);
}
void cstring::insert(int pos, cstring& source, unsigned int _size)
{
	pos = min((unsigned int)pos, size);
	unsigned int old_size = size;
	reserve(size+_size);
	memmove(this->c_str(pos+_size), this->c_str(pos), old_size-pos);
	memmove(this->c_str(pos), source.c_str(), _size);
}

int cstring::find(const char * source, int pos)
{
	unsigned int s_len = strlen(source);
	if( (size - pos) < s_len) return -1;
	for(unsigned int i=pos; i<=(size-s_len) ;i++)
		if( strncmp(&string[i], source, s_len) == 0) return i;
	return -1;
}

/*******************************************/
/*******************************************/
void cstring::reserve(unsigned int new_size)
{
	if(new_size > size){
		size = new_size;
		string = (char *)realloc(string, size+1);
		string[size]=0;
	}
}
void cstring::setmemory(unsigned int new_size)
{
	size = new_size;
	string = (char *)realloc(string, size+1);
	string[size]=0;
}

/********************************************
operator
********************************************/
bool cstring::operator==(const char * str)	const{ return strcmp(string, str		) == 0; }
bool cstring::operator==(cstring& str)		const{ return strcmp(string, str.c_str()) == 0; }
bool cstring::operator!=(const char * str)	const{ return strcmp(string, str		) != 0; }
bool cstring::operator!=(cstring& str)		const{ return strcmp(string, str.c_str()) != 0; }
/*******************************************/
/*******************************************/
char& cstring::operator[](int index)
{
	return string[index];
}
/*******************************************/
/*******************************************/
cstring& cstring::operator=(char c)
{
	reserve(1);
	string[0]=c;
	string[1]=0;
	return *this;
}
cstring& cstring::operator=(const char * source)
{
	unsigned int s_len = strlen(source);
	reserve(s_len);
	strcpy_s(string, s_len+1, source);
	return *this;
}
cstring& cstring::operator=(cstring& source)
{
	reserve(source.size);
	strcpy_s(string, source.size+1, source.string);
	return *this;
}
/*******************************************/
/*******************************************/
cstring cstring::operator+(char c)
{
	cstring tmp=*this;
	tmp+=c;
	return tmp;
}
cstring cstring::operator+(const char * source)
{
	cstring tmp=*this;
	tmp+=source;
	return tmp;
}
cstring cstring::operator+(cstring& source)
{
	cstring tmp=*this;
	tmp+=source;
	return tmp;
}

/*******************************************/
/*******************************************/
cstring& cstring::operator+=(char c)
{
	reserve(size+1);
	string[size-1]=c;
	return *this;
}
cstring& cstring::operator+=(const char * source)
{
	unsigned int s_len = strlen(source);
	reserve(size+s_len);
	strcat_s(string, size+1, source);
	return *this;
}
cstring& cstring::operator+=(cstring& source)
{
	unsigned int old_size=size;
	reserve(size+source.size);
	memmove(string+old_size, source.string, source.length());
	return *this;
}
