#ifndef _FILE_H
#define _FILE_H

#include <string>
class File
{

public:

	std::string filePath;

	File() {}

	File(const char *str)
	{
		*this = str;
	}

	File(char *str)
	{
		*this = str;
	}

	File(const std::string &str)
	{
		*this = str.c_str();
	}

	File(std::string &str)
	{
		*this = str.c_str();
	}

	File &operator =(const char *str)
	{
		filePath = str;
		int i;
#ifdef _WIN32
		while ((i = filePath.find('/')) != -1)
			filePath[i] = '\\';
#else
		while ((i = filePath.find('\\')) != -1)
			filePath[i] = '/';
#endif
		return *this;
	}

	File &operator =(char *str)
	{
		filePath = str;
		int i;
#ifdef _WIN32
		while ((i = filePath.find('/')) != -1)
			filePath[i] = '\\';
#else
		while ((i = filePath.find('\\')) != -1)
			filePath[i] = '/';
#endif
		return *this;
	}

	//operator const std::string() const
	//{
	//	return filePath;
	//}

	//operator std::string() const
	//{
	//	return filePath;
	//}

	operator const char *() const
	{
		return filePath.c_str();
	}

	const char *cStr() const
	{
		return filePath.c_str();
	}

	int size() const
	{
		return filePath.size();
	}
};

#endif
