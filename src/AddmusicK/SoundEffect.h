#ifndef _SOUNDEFFECT_H
#define _SOUNDEFFECT_H

#include "globals.h"

class SoundEffect
{
	int getHex();
	int getInt();
	int getPitch(int i, int octave);
	int getNoteLength(int i);
public:
	std::string &getEffectiveName();		// Returns name or pointName.
	int bank;
	int index;
	std::string name;
	std::vector<uint8_t> data;
	std::string text;
	bool add0;
	std::string pointName;
	int pointsTo;
	bool exists;

	int posInARAM;

	SoundEffect(void)
	{
		add0 = true;
		pointsTo = 0;
		exists = false;
	}

	void compile();
	void parseASM();
	void compileASM();
	void parseJSR();

	void parseDefine();
	void parseIfdef();
	void parseIfndef();
	void parseEndif();
	void parseUndef();

	std::vector<std::string> defineStrings;

	std::vector<std::string> asmStrings;
	std::vector<uint8_t> code;
	std::vector<std::string> asmNames;
	std::vector<std::string> jmpNames;
	std::vector<int> jmpPoses;

};

#endif
