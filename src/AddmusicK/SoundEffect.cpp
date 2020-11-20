#include "SoundEffect.h"
#include <cmath>
#include <sstream>
#include <iomanip>

static unsigned int pos;
static int line;
static bool triplet;
static int defaultNoteValue;
static bool inDefineBlock;

#define skipSpaces		\
while (isspace(text[pos]))	\
{				\
	if (text[pos] == '\n')	\
		line++;		\
	pos++;			\
}				\

#define error(str) {				\
	printError(str, false, name, line);	\
	continue; }				\

#define error2(str) {				\
	printError(str, false, name, line);	\
	return; }				\


#define append(value) soundEffects[bank][index].data.push_back(value)

std::string &SoundEffect::getEffectiveName()
{
	if (name == "")
		return pointName;
	else
		return name;
}

int SoundEffect::getHex()
{
	int i = 0;
	int d = 0;
	int j;

	while (pos < text.length())
	{
		if      ('0' <= text[pos] && text[pos] <= '9') j = text[pos] - 0x30;
		else if ('A' <= text[pos] && text[pos] <= 'F') j = text[pos] - 0x37;
		else if ('a' <= text[pos] && text[pos] <= 'f') j = text[pos] - 0x57;
		else break;
		pos++;
		d++;
		i = (i*16) + j;
	}
	if (d == 0) return -1;

	return i;
}

int SoundEffect::getInt()
{
	if (pos >= text.length()) return -1;
	//if (text[pos] == '$') { pos++; return getHex(); }	// Allow for things such as t$20 instead of t32.

	int i = 0;
	int d = 0;

	while (text[pos] >= '0' && text[pos] <= '9')
	{
		d++;
		i = (i * 10) + text[pos] - '0';
		pos++;
		if (pos >= text.length()) break;
	}

	if (d == 0) return -1; else return i;
}

int SoundEffect::getPitch(int letter, int octave)
{
	static const int pitches[] = {9, 11, 0, 2, 4, 5, 7};

	letter = pitches[letter - 0x61] + (octave - 1) * 12 + 0x80;

	pos++;
	if (text[pos] == '+') {letter++; pos++;}
	else if (text[pos] == '-') {letter--; pos++;}
	if (letter < 0x80)
		return -1;
	if (letter >= 0xC6)
		return -2;

	return letter;
}

int SoundEffect::getNoteLength(int i)
{
	i = getInt();
	if (i == -1 && text[pos] == '=')
	{
		pos++;
		i = getInt();
		if (i == -1) printError("Error parsing note length.", false, name, line);
		return i;
	}

	if (i < 1 || i > 192) i = defaultNoteValue;
	i = 192 / i;

	int frac = i;

	while (text[pos] == '.')
	{
		frac = frac / 2;
		i += frac;
		pos++;
	}

	if (triplet)
		i = (int)floor(((double)i * 2.0 / 3.0) + 0.5);
	return i;
}

void SoundEffect::compile()
{
	text += "                   ";
	int version = 0;			// Unused for sound effects for now.
	preprocess(text, name, version);
	pos = 0;
	line = 0;
	triplet = false;
	defaultNoteValue = 8;
	inDefineBlock = false;


	unsigned int instr = -1;
	int lastNote = -1;
	bool firstNote = true;
	bool pitchSlide = false;
	int octave = 4;
	int lastNoteValue = -1;
	int volume[2] = {0x7F, 0x7F};
	int i, j;
	bool updateVolume = false;
	bool inComment = false;

	while (pos < text.size())
	{
		if (text[pos] == '\n')
			inComment = false;
		else if (inComment == true)
			pos++;

		if (inComment)
			continue;

		switch (text[pos])
		{
		case '#':
			if (text.substr(pos+1, 3) == "asm")
				parseASM();
			else if (text.substr(pos+1, 3) == "jsr")
				parseJSR();
			else if (text.substr(pos+1, 6) == "define")
				parseDefine();
			else if (text.substr(pos+1, 5) == "undef")
				parseUndef();
			else if (text.substr(pos+1, 5) == "ifdef")
				parseIfdef();
			else if (text.substr(pos+1, 6) == "ifndef")
				parseIfndef();
			else if (text.substr(pos+1, 5) == "endif")
				parseEndif();

			else
			{
				pos++;
				error("Channel declarations are not allowed in sound effects.");
			}
			continue;
		case '!':
			pos = ~0;
			continue;
		case 'v':
			pos++;
			i = getInt();
			if (i == -1)  error("Error parsing volume command.")
			if (i > 0x7F) error("Volume too high.  Only values from 0 - 127 are allowed.")

			volume[0] = i;
			volume[1] = i;
			skipSpaces;

			if (text[pos] == ',')
			{
				pos++;
				skipSpaces;
				i = getInt();
				if (i == -1) error("Error parsing volume command.")
				if (i > 0x7F) error("Illegal value for volume command.  Only values between 0 and 127 are allowed.")
				volume[1] = i;
			}

			updateVolume = true;
			break;
		case 'l':
			pos++;
			i = getInt();
			if (i == -1) { printError("Error parsing 'l' directive.", false, name, line); continue; }
			if (i > 192) { printError("Illegal value for 'l' directive.", false, name, line); continue; }
			defaultNoteValue = i;
			break;

		case '@':
			pos++;
			i = getInt();
			if (i <  0x00) error("Error parsing instrument ('@') command.");
			if (i > 0x7F) error("Illegal value for instrument ('@') command.")

			j = -1;

			skipSpaces;

			if (text[pos] == ',')
			{
				pos++;
				skipSpaces;
				j = getInt();
				if (j < 0) error("Error parsing noise instrument ('@,') command.")
				if (j > 0x1F) error("Illegal value for noise instrument ('@,') command.  Only values between 0 and 31")
			}


			append(0xDA);
			if (j != -1) append(0x80 | j);
			append(i);
			instr = i;
			break;

		case 'o':
			pos++;
			i = getInt();
			if (i == -1) error("Error parsing octave directive.")
			if (i < 0 || i > 6) error("Illegal value for octave command.")

			octave = i;
			break;

		case '$':
			pos++;
			i = getHex();
			if (i == -1) error("Error parsing hex command.")
			if (i > 0xFF) error("Illegal hex value.")

			append(i);

			break;

		case '>':
			pos++;
			if (++octave > 6) error("Illegal octave reached via '>' directive.")
			break;

		case '<':
			pos++;
			if (--octave < 1) error("Illegal octave reached via '<' directive.")
			break;

		case '{':
			if (triplet) error("Triplet enable directive specified in a triplet block.");
			triplet = true;
			break;

		case '}':
			if (!triplet) error("Triplet disable directive specified outside a triplet block.");
			triplet = false;
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'r': case '^':
			j = text[pos];

			if (j == 'r')
				i = 0xC7, pos++;
			else if (j == '^')
				i = 0xC6, pos++;
			else
				i = getPitch(j, octave);

			if (i < 0) i = 0xC7;


			j = getNoteLength(defaultNoteValue);

			if (i == lastNoteValue && !updateVolume) i = 0;

			if (i != 0xC6 && i != 0xC7 && (text[pos] == '&' || pitchSlide))
			{
				pitchSlide = true;
				if (firstNote == true)
				{
					if (lastNote == -1)
						lastNote = i;
					else
					{
						if (j > 0)
						{
							append(j);
							lastNoteValue = j;
						}
						if (updateVolume)
						{
							append(volume[0]);
							if (volume[0] != volume[1]) append(volume[1]);
							updateVolume = false;
						}

						append(0xDD);
						append(lastNote);
						append(0x00);
						append(lastNoteValue);
						append(i);
						firstNote = false;
					}
				}
				else
				{
					if (j > 0)
					{
						append(j);
						lastNoteValue = j;
					}

					if (updateVolume)
					{
						append(volume[0]);
						if (volume[0] != volume[1]) append(volume[1]);
						updateVolume = false;
					}

					append(0xEB);
					append(0x00);
					append(lastNoteValue);
					append(i);
				}

				if (j < 0) lastNoteValue = j;
				pos++;
				break;
			}
			else
			{
				firstNote = true;
				pitchSlide = false;
			}

			if (j >= 0x80)
			{
				append(0x7F);

				if (updateVolume)
				{
					append(volume[0]);
					if (volume[0] != volume[1]) append(volume[1]);
					updateVolume = false;
				}

				append(i);

				j -= 0x7F;

				while (j > 0x7F)
				{
					j -= 0x7F;
					append(0xC6);
				}

				if (j > 0)
				{
					if (j != 0x7F) append(j);
					append(0xC6);
				}

				lastNoteValue = j;
				break;


			}
			else if (j > 0)
			{
				append(j);
				lastNoteValue = j;
				if (updateVolume)
				{
					append(volume[0]);
					if (volume[0] != volume[1]) append(volume[1]);
					updateVolume = false;
				}

				append(i);
			}
			else
				append(i);
			break;

			// Phew...
		case '\n':
			pos++;
			line++;
			break;

		case ';':
			pos++;
			inComment = true;
			break;

		default:
			if (!isspace(text[pos]))
				printWarning(std::string("Warning: Unexpected symbol '") + text[pos] + std::string("'found."), name, line);
			pos++;
			break;

		}



	}
	if (soundEffects[bank][index].add0) append(0x00);
	compileASM();


}

void SoundEffect::parseASM()
{
	pos+=4;
	if (isspace(text[pos]) == false)
		error2("Error parsing asm directive.")

	skipSpaces;

	std::string tempname;

	while (isspace(text[pos]) == false)
	{
		if (pos >= text.length())
			break;

		tempname += text[pos++];
	}

	skipSpaces;

	if (text[pos] != '{')
		error2("Error parsing asm directive.")

	int startPos = ++pos;

	while (text[pos] != '}')
	{
		if (pos >= text.length())
			error2("Error parsing asm directive.")

		pos++;
	}

	int endPos = pos;

	pos++;

	asmStrings.push_back(text.substr(startPos, endPos - startPos));
	asmNames.push_back(tempname);

}

void SoundEffect::compileASM()
{
	//int codeSize = 0;

	std::vector<unsigned int> codePositions;

	for (unsigned int i = 0; i < asmStrings.size(); i++)
	{
		codePositions.push_back(code.size());

		std::stringstream asmCode;
		removeFile("temp.bin");
		removeFile("temp.asm");
		asmCode << "arch spc700-raw\n\norg $000000\nincsrc \"" << "asm/main.asm" << "\"\nbase $" << hex4 << posInARAM + code.size() + data.size() << "\n\norg $008000\n\n" << asmStrings[i];
		writeTextFile("temp.asm", asmCode.str());

		if (!asarCompileToBIN("temp.asm", "temp.bin"))
			error2("asar reported an error.  Refer to temp.log for details.");

		std::vector<uint8_t> temp;

		openFile("temp.bin", temp);

		temp.erase(temp.begin(), temp.begin() + 0x08000);

		for (unsigned int j = 0; j < temp.size(); j++)
			code.push_back(temp[j]);
	}

	for (unsigned int i = 0; i < asmStrings.size(); i++)
	{
		int k = -1;
		for (unsigned int j = 0; j < jmpNames.size(); j++)
		{
			if (asmNames[i] == jmpNames[j])
			{
				k = j;
				break;
			}
		}

		if (k == -1)
			error2("Could not match asm and jsr names.");

		data[jmpPoses[k]] = (posInARAM + data.size() + codePositions[k]) & 0xFF;
		data[jmpPoses[k]+1] = (posInARAM + data.size() + codePositions[k]) >> 8;
	}
}

void SoundEffect::parseJSR()
{
	pos+=4;
	if (isspace(text[pos]) == false)
		error2("Error parsing jsr command.")

	skipSpaces;

	std::string tempname;

	while (isspace(text[pos]) == false)
	{
		if (pos >= text.length())
			break;

		tempname += text[pos++];
	}

	jmpNames.push_back(tempname);
	append(0xFD);
	jmpPoses.push_back(data.size());
	append(0x00);
	append(0x00);
}

void SoundEffect::parseDefine()
{
	pos += 7;
	skipSpaces;
	std::string defineName;
	while (!isspace(text[pos]) && pos < text.length())
	{
		defineName += text[pos++];
	}

	for (unsigned int z = 0; z < defineStrings.size(); z++)
		if (defineStrings[z] == defineName)
			error2("A string cannot be defined more than once.");

	defineStrings.push_back(defineName);
}

void SoundEffect::parseUndef()
{
	pos += 6;
	skipSpaces;
	std::string defineName;
	while (!isspace(text[pos]) && pos < text.length())
	{
		defineName += text[pos++];
	}
	unsigned int z = -1;
	for (z = 0; z < defineStrings.size(); z++)
		if (defineStrings[z] == defineName)
			goto found;


	error2("The specified string was never defined.");

found:
	defineStrings[z].clear();
}

void SoundEffect::parseIfdef()
{
	pos+=6;
	inDefineBlock = true;
	skipSpaces;
	std::string defineName;
	while (!isspace(text[pos]) && pos < text.length())
	{
		defineName += text[pos++];
	}

	unsigned int z = -1;

	int temp;

	for (unsigned int z = 0; z < defineStrings.size(); z++)
		if (defineStrings[z] == defineName)
			goto found;

	temp = text.find("#endif", pos);

	if (temp == -1)
		error2("#ifdef was missing a matching #endif.");

	pos = temp;
found:
	return;
}

void SoundEffect::parseIfndef()
{
	pos+=7;
	inDefineBlock = true;
	skipSpaces;
	std::string defineName;
	while (!isspace(text[pos]) && pos < text.length())
	{
		defineName += text[pos++];
	}

	unsigned int z = -1;

	for (unsigned int z = 0; z < defineStrings.size(); z++)
		if (defineStrings[z] == defineName)
			goto found;

	return;

found:
	int temp = text.find("#endif", pos);

	if (temp == -1)
		error2("#ifdef was missing a matching #endif.");

	pos = temp;


}

void SoundEffect::parseEndif()
{
	pos += 6;
	if (inDefineBlock == false)
		error2("#endif was found without a matching #ifdef or #ifndef")
	else
		inDefineBlock = false;
}
