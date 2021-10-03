#ifndef _MUSIC_H
#define _MUSIC_H
#include <vector>
#include <string>
#include <map>
#include "globals.h"

struct SpaceInfo {
	int songStartPos;
	int songEndPos;
	int sampleTableStartPos;
	int sampleTableEndPos;
	std::vector<int> individualSampleStartPositions;
	std::vector<int> individualSampleEndPositions;
	std::vector<bool> individialSampleIsImportant;
	int importantSampleCount;
	int echoBufferEndPos;
	int echoBufferStartPos;

};

class Music
{


public:
	double introSeconds;
	double mainSeconds;

	int noteParamaterByteCount;

	int tempoRatio;
	int divideByTempoRatio(int, bool fractionIsError);		// Divides a value by tempoRatio.  Errors out if it can't be done without a decimal (if the parameter is set).
	int multiplyByTempoRatio(int);					// Multiplies a value by tempoRatio.  Errors out if it goes higher than 255.
	bool nextHexIsArpeggioNoteLength;

	std::string name;
	std::string pathlessSongName;
	std::vector<uint8_t> data[9];
	std::vector<unsigned short> loopLocations[9];	// With remote loops, we can have remote loops in standard loops, so we need that ninth channel.
	bool playOnce;
	bool hasIntro;
	unsigned short phrasePointers[8][2];
	unsigned short loopPointers[0x10000];
	//unsigned int loopLengths[0x10000];		// How long, in ticks, each loop is.
	std::string text;
	int totalSize;
	int spaceForPointersAndInstrs;
	std::vector<uint8_t> allPointersAndInstrs;
	std::vector<uint8_t> instrumentData;
	std::vector<uint8_t> finalData;

	SpaceInfo spaceInfo;


	unsigned int introLength;
	unsigned int mainLength;

	unsigned int seconds;

	bool hasYoshiDrums;

	bool knowsLength;

	int index;

	//uint8_t mySamples[255];
	std::vector<unsigned short> mySamples;
	//int mySampleCount;
	int echoBufferSize;

	std::string statStr;

	std::string title;
	std::string author;
	std::string game;
	std::string comment;

	bool usedSamples[256];		// Holds a record of which samples have been used for this song.


	int minSize;

	bool exists;

	int posInARAM;

	void compile();

	int remoteDefinitionType;
	bool inRemoteDefinition;
	//int remoteDefinitionArg;

	std::map<std::string, std::string> replacements;
	std::vector<const std::pair<const std::string, std::string> *> sortedReplacements;
	Music();

	void init();
	bool doReplacement();
private:
	void pointersFirstPass();
	void parseComment();
	void parseQMarkDirective();
	void parseExMarkDirective();
	void parseChannelDirective();
	void parseLDirective();
	void parseGlobalVolumeCommand();
	void parseVolumeCommand();
	void parseQuantizationCommand();
	void parsePanCommand();
	void parseIntroDirective();
	void parseT();
	void parseTempoCommand();
	void parseTransposeDirective();
	void parseOctaveDirective();
	void parseInstrumentCommand();
	void parseOpenParenCommand();
	void parseLabelLoopCommand();
	void parseSampleLoadCommand();
	void parseLoopCommand();
	void parseLoopEndCommand();
	void parseStarLoopCommand();
	void parseVibratoCommand();
	void parseTripletOpenDirective();
	void parseTripletCloseDirective();
	void parseRaiseOctaveDirective();
	void parseLowerOctaveDirective();
	void parsePitchSlideCommand();
	void parseHexCommand();
	void parseNote();
	void parseHDirective();
	void parseReplacementDirective();
	void parseNCommand();

	void parseOptionDirective();

	void parseSpecialDirective();
	void parseInstrumentDefinitions();
	void parseSampleDefinitions();
	void parsePadDefinition();
	void parseASMCommand();
	void parseJSRCommand();
	void parseLouderCommand();
	void parseTempoImmunityCommand();
	void parsePath();
	void compileASM();

	void parseDefine();
	void parseIfdef();
	void parseIfndef();
	void parseEndif();
	void parseUndef();

	void parseSPCInfo();


	//std::vector<std::string> defineStrings;

	void printChannelDataNonVerbose(int);
	void parseHFDHex();
	void parseHFDInstrumentHack(int addr, int bytes);
	void insertedZippedSamples(const std::string &path);

	int getInt();
	int getInt(const std::string &str, int &p);
	int getIntWithNegative();
	int getHex(bool anyLength = false);
	int getPitch(int j);
	int getNoteLength(int);
	int getNoteLengthModifier(int, bool);

	bool guessLength;
	int resizedChannel;

	double channelLengths[8];				// How many ticks are in each channel.
	double loopLengths[0x10000];				// How many ticks are in each loop.
	double normalLoopLength;				// How many ticks were in the most previously declared normal loop.
	double superLoopLength;					// How many ticks were in the most previously declared super loop.
	std::vector<std::pair<double, int>> tempoChanges;	// Where any changes in tempo occur. A negative tempo marks the beginning of the main loop, if an intro exists.

	std::vector<std::vector<uint8_t>> remoteGainConversion;	// Containers that hold data for anticipation gain and rest/gain replacement so that we can convert it to a remote command.
	std::vector<unsigned int> remoteGainPositions[9];	// Container that holds the positions for the pointers that we have to go back and correct during old gain and remote command conversion.

	bool baseLoopIsNormal;
	bool baseLoopIsSuper;
	bool extraLoopIsNormal;
	bool extraLoopIsSuper;

	void handleNormalLoopEnter();					// Call any time a definition of a loop is entered.
	void handleSuperLoopEnter();					// Call any time a definition of a super loop is entered.
	void handleNormalLoopRemoteCall(int loopCount);			// Call any time a normal loop is called remotely.
	void handleNormalLoopExit(int loopCount);			// Call any time a definition of a loop is exited.
	void handleSuperLoopExit(int loopCount);			// Call any time a definition of a super loop is exited.

	void addNoteLength(double ticks);				// Call this every note.  The correct channel/loop will be automatically updated.
};

#endif
