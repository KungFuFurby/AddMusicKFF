#ifndef _SAMPLE_H
#define _SAMPLE_H

#include "globals.h"

class Sample
{
public:
	std::string name;
	std::vector<uint8_t> data;
	unsigned short loopPoint;
	bool exists;
	bool important;			// If a sample is important, then is should never be excluded.
					// Otherwise, it's only used if the song has an instrument or $E5/$F3 command that is using it.
	bool isBNK; //Samples generated from a BNK file have specific checks omitted from it due to using an auto-generated name.

	Sample(void)
	{
		loopPoint = 0;
		exists = false;
		important = true;
		isBNK = false;
	}
};

#endif
