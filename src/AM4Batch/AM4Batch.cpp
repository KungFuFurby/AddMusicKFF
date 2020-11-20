// AM4Batch.cpp : Defines the entry point for the console application.
//
#include "../AddmusicK/Directory.h"
#include <fstream>

void openTextFile(const File &fileName, std::string &s) 
{
	std::ifstream is(fileName.cStr());

	if (!is)
		return;

	s.assign( (std::istreambuf_iterator<char>(is)), (std::istreambuf_iterator<char>()) );
}

void writeTextFile(const File &fileName, const std::string &string)
{
	std::ofstream ofs;
	ofs.open(fileName.cStr(), std::ios::binary);

	std::string n = string;

#ifdef _WIN32
	unsigned int i = 0;
	while (i < n.length())
	{
		if (n[i] == '\n')
		{
			n = n.insert(i, "\r");
			i++;
		}
		i++;
	}
#endif
	ofs.write(n.c_str(), n.size());

	ofs.close();
}


int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string s;
		openTextFile(argv[i], s);
		if (s.find("#amm") == -1 && s.find("#amk") == -1 && s.find("#am4") == -1) 
		{
			s.append("\n\n#am4");
			writeTextFile(argv[i], s);
		}

	}

	return 0;
}

