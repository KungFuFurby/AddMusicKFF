#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

class cstring{
private:
	char *string;
	unsigned int size;


public:

	//routine
	char * c_str(unsigned int index = 0);	//ptr to string
	unsigned int length(void);				//get length
	void append(char c);
	void append(const char *source, int _size = -1);
	void append(cstring&source, int _size = -1);
	void append(short dw);
	void append(int dd);
	void erase(int pos=0, unsigned int _size = -1);
	void insert(int pos, const char c);
	void insert(int pos, const char * source, unsigned int _size);
	void insert(int pos, cstring& source, unsigned int _size);
	int find(const char * source, int pos=0);

	//opeartor
	bool operator==(const char *) const;
	bool operator==(cstring&) const;
	bool operator!=(const char *) const;
	bool operator!=(cstring&) const;

	char& operator[](int);		//access to char

	cstring& operator=(char);
	cstring& operator=(const char *);
	cstring& operator=(cstring&);

	cstring operator+(char);		//add char
	cstring operator+(const char *)	;	//add string
	cstring operator+(cstring&);	//add this string-class

	cstring& operator+=(char);		//add char
	cstring& operator+=(const char *)	;	//add string
	cstring& operator+=(cstring&);	//add this string-class

	//constructor and deconstructor
	cstring();
	cstring(const char c);
	cstring(const char c, unsigned int _size);
	cstring(const char * source);
	cstring(cstring& source);
	~cstring();

private:
	void reserve(unsigned int new_size);
	void setmemory(unsigned int new_size);
};
