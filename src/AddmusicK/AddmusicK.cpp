#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include "globals.h"
#include <ctime>
#include "../AM405Remover/AM405Remover.h"
#include "fs.h"		// // //
#include <cstdint>
#include "lodepng.h"
#include <thread>


bool waitAtEnd = true;
File ROMName;

std::vector<uint8_t> romHeader;

void cleanROM();
void tryToCleanSampleToolData();
void tryToCleanAM4Data();
void tryToCleanAMMData();

void assembleSNESDriver();
void assembleSPCDriver();
void loadMusicList();
void loadSampleList();
void loadSFXList();
void compileSFX();
void compileGlobalData();
void compileMusic();
void fixMusicPointers();
void generateSPCs();
void assembleSNESDriver2();
void generateMSC();
void cleanUpTempFiles();

void generatePNGs();

void checkMainTimeStamps();
bool recompileMain = true;

time_t mostRecentMainModification = 0;		// The most recent modification to any sound effect file, any global song file, any list file, or any asm file


bool justSPCsPlease = false;
std::vector<std::string> textFilesToCompile;

int main(int argc, char* argv[]) try		// // //
{
	std::clock_t startTime = clock();

	std::cout << "AddmusicK version " << AMKVERSION << "." << AMKMINOR << "." << AMKREVISION << " Release Candidate by Kipernal" << std::endl;
	std::cout << "Parser version " << PARSER_VERSION << std::endl << std::endl;
	std::cout << "Protip: Be sure to read the readme! If there's an error or something doesn't\nseem right, it may have your answer!\n\n" << std::endl;


	std::vector<std::string> arguments;

	if (fileExists("Addmusic_options.txt"))
	{
		std::string optionsString;
		openTextFile("Addmusic_options.txt", optionsString);
		unsigned int osPos = 0;
		while (osPos < optionsString.size())
		{
			// This is probably a catastrophicly bad idea on several levels, but I don't have the time do redo this entire section of code.
			// AddmusicK 2.0: Now with actually good programming habits! (probably not)


			std::string opsubstr;
			if (optionsString.find('\n', osPos) == -1)
				opsubstr = optionsString.substr(osPos);
			else
				opsubstr = optionsString.substr(osPos, optionsString.find('\n', osPos) - osPos);

			arguments.push_back(opsubstr);
			osPos += opsubstr.size() + 1;
		}
	}
	else
	{
		for (int i = 1; i < argc; i++)
			arguments.push_back(argv[i]);
	}

	for (int i = 0; i < arguments.size(); i++)
	{
		waitAtEnd = false;			// If the user entered a command line argument, chances are they don't need the "Press Any Key To Continue . . ." prompt.
		if (arguments[i] == "-c")
			convert = false;
		else if (arguments[i] == "-e")
			checkEcho = false;
		else if (arguments[i] == "-b")
			bankStart = 0x080000;
		else if (arguments[i] == "-v")
			verbose = true;
		else if (arguments[i] == "-a")
			aggressive = true;
		else if (arguments[i] == "-d")
			dupCheck = false;
		else if (arguments[i] == "-h")
			validateHex = false;
		else if (arguments[i] == "-p")
			doNotPatch = true;
		else if (arguments[i] == "-u")
			optimizeSampleUsage = false;
		else if (arguments[i] == "-s")
			allowSA1 = false;
		else if ((arguments[i] == "-dumpsfx") || (arguments[i] == "-sfxdump"))
			sfxDump = true;
		else if (arguments[i] == "-visualize")
			visualizeSongs = true;
		//else if (arguments[i] == "-g")
			//Removed because it was de-facto never functional due to the code this relied on being dummied out.
			//forceSPCGeneration = true;
		else if (arguments[i] == "-noblock")
			forceNoContinuePrompt = true;
		else if (arguments[i] == "-streamredirect")
		{
			redirectStandardStreams = true;
			std::freopen("AddmusicK_stdout", "w+", stdout);
			std::freopen("AddmusicK_stderr", "w+", stderr);
		}
		else if (arguments[i] == "-norom")
		{
			if (ROMName.size() != 0)
				printError("Error: -norom cannot be used after a filepath has already been used. Input your text files /after/ the -norom option.", true);
			justSPCsPlease = true;
		}
		else if (ROMName.size() == 0 && arguments[i][0] != '-')
		{
			if (!justSPCsPlease)
				ROMName = arguments[i];
			else
				textFilesToCompile.push_back(arguments[i]);
		}
		else
		{
			if (arguments[i] !=  "-?")
			{
				printf("Unknown argument \"%s\".", arguments[i].c_str());
			}

			puts("Options:\n\t-e: Turn off echo buffer checking.\n\t-c: Force off conversion from Addmusic 4.05 and AddmusicM\n\t-b: Do not attempt to save music data in bank 0x40 and above.\n\t-v: Turn verbosity on.  More information will be displayed using this.\n\t-a: Make free space finding more aggressive.\n\t-d: Turn off duplicate sample checking.\n\t-h: Turn off hex command validation.\n\t-p: Create a patch, but do not patch it to the ROM.\n\t-norom: Only generate SPC files, no ROM required.\n\t-?: Display this message.\n\n");

			if (arguments[i] != "-?")
			{
				quit(1);
			}
		}
	}

	if (justSPCsPlease == false)
	{

		if (ROMName.size() == 0)
		{
			printf("Enter your ROM name: ");
			std::getline(std::cin, ROMName.filePath);
			puts("\n\n");
		}

		if (asar_init() == false)
			useAsarDLL = false;
		else
			useAsarDLL = true;



		std::string tempROMName = ROMName.cStr();
		if (fileExists(tempROMName + ".smc") && fileExists(tempROMName + ".sfc"))
			printError("Error: Ambiguity detected; there were two ROMs with the specified name (one\nwith a .smc extension and one with a .sfc extension). Either delete one or\ninclude the extension in your filename.", true);
		else if (fileExists(tempROMName + ".smc"))
			tempROMName += ".smc";
		else if (fileExists(tempROMName + ".sfc"))
			tempROMName += ".sfc";
		else if (fileExists(tempROMName)) ;
		else
			printError("ROM not found.", true);
		ROMName = tempROMName;
		openFile(ROMName, rom);


		tryToCleanAM4Data();
		tryToCleanAMMData();

		if (rom.size() % 0x8000 != 0)
		{
			romHeader.assign(rom.begin(), rom.begin() + 0x200);
			rom.erase(rom.begin(), rom.begin() + 0x200);
		}
		//rom.openFromFile(ROMName);

		if (rom.size() <= 0x80000)
			printError("Error: Your ROM is too small. Save a level in Lunar Magic or expand it with\nLunar Expand, then try again.", true);

		if (rom[SNESToPC(0xFFD5)] == 0x23 && allowSA1)
			usingSA1 = true;
		else
			usingSA1 = false;


		cleanROM();
	}

	loadSampleList();
	loadMusicList();
	loadSFXList();

	//checkMainTimeStamps();

	assembleSNESDriver();		// We need this for the upload position, where the SPC file's PC starts.  Luckily, this function is very short.

	//if (recompileMain || forceSPCGeneration)
	//{
	assembleSPCDriver();
	compileSFX();
	compileGlobalData();
	//}

	if (justSPCsPlease)
	{
		// We start loading CLI songs from highestGlobalSong + 1. If no global songs are
		// present, highestGlobalSong = 0 and we start loading songs from slot 1. We
		// leave slot 0 empty, to match the SNES driver which treats song 0 as a NOP rather
		// than a song number, with the song ID also being unsendable and unplayable on the
		// SPC under its raw ID because that also acts as a NOP.
		int firstLocalSong = highestGlobalSong + 1;

		// Unset local songs loaded from Addmusic_list.txt.
		for (int i = firstLocalSong; i < 256; i++)
			musics[i].exists = false;

		// Load local songs from command-line arguments.
		for (int i = 0; i < textFilesToCompile.size(); i++)
		{
			if (firstLocalSong + i >= 256)
				printError("Error: The total number of requested music files to compile exceeded 255.", true);
			musics[firstLocalSong + i].exists = true;
			musics[firstLocalSong + i].name = textFilesToCompile[i];
		}
	}

	compileMusic();
	fixMusicPointers();

	generateSPCs();

	if (visualizeSongs)
		generatePNGs();

	if (justSPCsPlease == false)
	{
		assembleSNESDriver2();
		generateMSC();
#ifndef _DEBUG
			cleanUpTempFiles();
#endif
	}

	std::cout << "\nSuccess!\n" << std::endl;


	std::clock_t endTime = clock();
	if (((endTime - startTime) / (float)CLOCKS_PER_SEC) - 1 < 0.02)
		std::cout << "Completed in 1 second." << std::endl;
	else
		std::cout << "Completed in " << std::dec << std::setprecision(2) << std::fixed << (endTime - startTime) / (float)CLOCKS_PER_SEC << " seconds." << std::endl;

	if (waitAtEnd)
		quit(0);
}
catch (const std::exception &e) {		// // //
	std::cerr << "Uncaught C++ exception: " << e.what() << std::endl;
	exit(1);
}
catch (...) {
	std::cerr << "Unknown exception." << std::endl;
	exit(1);
}

void displayNewUserMessage()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif

	if (forceNoContinuePrompt == false)
	{
		std::cout << "This is a clean ROM you're using AMK on.";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\n\nSo here's a message for new users.";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\n\nIf there's some error you don't understand,";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\nOr if something weird happens and you don't know why,";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\n\n\nRead the whole ";
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
		std::cout << "buggin' ";
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
		std::cout << "ever ";
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
		std::cout << "lovin' ";
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
		std::cout << "README!";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\n\n\nReally, it has answers to some of the most basic questions.";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "\nIf for no one else than yourself, read the readme first.";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "\nThat way you don't get chastised for asking something answered by it.";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "\nNot every possible answer is in there,";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\nBut save yourself some time and at least make an effort.";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		std::cout << "\n\nDo we have a deal?";
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		std::cout << "\nAlright. Cool. Now go out there and use/make awesome music.";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		std::cout << "\n\n(Power users: Use -noblock to skip this on future new ROMs.)";
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
}

void cleanROM()
{
	//tryToCleanAM4Data();
	//tryToCleanAMMData();
	tryToCleanSampleToolData();


	if (rom[0x70000] == 0x3E && rom[0x70001] == 0x0E)	// If this is a "clean" ROM, then we don't need to do anything.
	{


		//displayNewUserMessage();


		writeFile("asm/SNES/temp.sfc", rom);
		return;
	}
	else
	{
		//"New Super Mario World Sample Utility 2.0 by smkdan"

		std::string romprogramname;
		romprogramname += (char)*(rom.data() + SNESToPC(0x0E8000));
		romprogramname += (char)*(rom.data() + SNESToPC(0x0E8001));
		romprogramname += (char)*(rom.data() + SNESToPC(0x0E8002));
		romprogramname += (char)*(rom.data() + SNESToPC(0x0E8003));

		if (romprogramname != "@AMK")
		{
			std::stringstream s;
			s << "Error: The identifier for this ROM, \"" << romprogramname << "\", could not be identified. It should\n"
				"be \"@AMK\". This either means that some other program has modified this area of\n"
				"your ROM, or your ROM is corrupted.";		// // //
			if (forceNoContinuePrompt) {
				std::cerr << s.str() << std::endl;
				quit(1);
			}
			std::cout << s.str() << " Continue? (Y or N)" << std::endl;


			int c = '\0';
			while (c != 'y' && c != 'Y' && c != 'n' && c != 'N')
				c = getchar();

			if (c == 'n' || c == 'N')
				quit(1);

		}

		int romversion = *(unsigned char *)(rom.data() + SNESToPC(0x0E8004));

		if (romversion > DATA_VERSION)
		{
			std::stringstream s;		// // //
			s << "WARNING: This ROM was modified using a newer version of AddmusicK." << std::endl;
			s << "You can continue, but it is HIGHLY recommended that you upgrade AMK first." << std::endl;
			if (forceNoContinuePrompt) {
				std::cerr << s.str() << std::endl;
				quit(1);
			}
			std::cout << s.str() << "Continue anyway? (Y or N)" << std::endl;

			int c = '\0';
			while (c != 'y' && c != 'Y' && c != 'n' && c != 'N')
				c = getchar();

			if (c == 'n' || c == 'N')
				quit(1);
		}

		int address = SNESToPC(*(unsigned int *)(rom.data() + 0x70005) & 0xFFFFFF);	// Address, in this case, is the sample numbers list.
		clearRATS(address - 8);								// So erase it all.




		int baseAddress = SNESToPC(*(unsigned int *)(rom.data() + 0x70008));		// Address is now the address of the pointers to the songs and samples.
		bool erasingSamples = false;

		int i = 0;
		while (true)
		{
			address = *(unsigned int *)(rom.data() + baseAddress + i*3) & 0xFFFFFF;
			if (address == 0xFFFFFF)						// 0xFFFFFF indicates an end of pointers.
			{
				if (erasingSamples == false)
					erasingSamples = true;
				else
					break;
			}
			else
			{
				if (address != 0)
					clearRATS(SNESToPC(address - 8));
			}

			i++;
		}
	}

	writeFile("asm/SNES/temp.sfc", rom);

}

void assembleSNESDriver()
{
	std::string patch;
	openTextFile("asm/SNES/patch.asm", patch);
	programUploadPos = scanInt(patch, "!DefARAMRet = ");
}

void assembleSPCDriver()
{
	remove(File("temp.log"));

	remove(File("asm/main.bin"));
	std::string patch;
	openTextFile("asm/main.asm", patch);
	programPos = scanInt(patch, "base ");
	if (verbose)
		std::cout << "Compiling main SPC program, pass 1." << std::endl;

	//execute("asar asm/main.asm asm/main.bin 2> temp.log > temp.txt");

	//if (fileExists("temp.log"))
	if (!asarCompileToBIN("asm/main.asm", "asm/main.bin"))
		printError("asar reported an error while assembling asm/main.asm. Refer to temp.log for\ndetails.\n", true);

	std::string temptxt;
	openTextFile("temp.txt", temptxt);
	mainLoopPos = scanInt(temptxt, "MainLoopPos: ");
	reuploadPos = scanInt(temptxt, "ReuploadPos: ");
	SRCNTableCodePos = scanInt(temptxt, "SRCNTableCodePos: ");
	noSFX = (temptxt.find("NoSFX is enabled") != -1);
	if (sfxDump && noSFX) {
		printWarning("The sound driver build does not support sound effects due to the !noSFX flag\r\nbeing enabled in asm/UserDefines.asm, yet you requested to dump SFX. There will\r\nbe no new SPC dumps of the sound effects since the data is not included by\r\ndefault, nor is the playback code for the sound effects.");
		sfxDump = false;
	}

	remove("temp.log");

	programSize = getFileSize("asm/main.bin");
}

void loadMusicList()
{
	std::string musicFile;
	openTextFile("Addmusic_list.txt", musicFile);

	if (musicFile[musicFile.length()-1] != '\n')
		musicFile += '\n';

	unsigned int i = 0;

	bool inGlobals = false;
	bool inLocals = false;
	bool gettingName = false;
	int index = -1;
	int shallowSongCount = 0;

	std::string tempName;

	while (i < musicFile.length())
	{
		if (isspace(musicFile[i]) && !gettingName)
		{
			i++;
			continue;
		}

		if (strncmp(musicFile.c_str() + i, "Globals:", 8) == 0)
		{
			inGlobals = true;
			inLocals = false;
			i+=8;
			continue;
		}

		if (strncmp(musicFile.c_str() + i, "Locals:", 7) == 0)
		{
			inGlobals = false;
			inLocals = true;
			i+=7;
			continue;
		}

		if (!inGlobals && !inLocals)
			printError("Error: Could not find \"Globals:\" label in list.txt", true);

		if (index < 0)
		{
			if      ('0' <= musicFile[i] && musicFile[i] <= '9') index = musicFile[i++] - '0';
			else if ('A' <= musicFile[i] && musicFile[i] <= 'F') index = musicFile[i++] - 'A' + 10;
			else if ('a' <= musicFile[i] && musicFile[i] <= 'f') index = musicFile[i++] - 'a' + 10;
			else printError("Invalid number in list.txt.", true);

			index <<= 4;


			if      ('0' <= musicFile[i] && musicFile[i] <= '9') index |= musicFile[i++] - '0';
			else if ('A' <= musicFile[i] && musicFile[i] <= 'F') index |= musicFile[i++] - 'A' + 10;
			else if ('a' <= musicFile[i] && musicFile[i] <= 'f') index |= musicFile[i++] - 'a' + 10;
			else if (isspace(musicFile[i])) index >>= 4;
			else printError("Invalid number in list.txt.", true);

			if (!isspace(musicFile[i]))
				printError("Invalid number in list.txt.", true);
			if (inGlobals)
				highestGlobalSong = std::max(highestGlobalSong, index);
			if (inLocals)
				if (index <= highestGlobalSong)
					printError("Error: Local song numbers must be greater than the largest global song number.", true);
		}
		else
		{
			if (musicFile[i] == '\n' || musicFile[i] == '\r')
			{
				musics[index].name = tempName;
				if (inLocals && justSPCsPlease == false)
				{
					openTextFile((std::string("music/") + tempName), musics[index].text);
				}
				musics[index].exists = true;
				index = -1;
				i++;
				shallowSongCount++;
				gettingName = false;
				tempName.clear();
				continue;

			}
			gettingName = true;
			tempName += musicFile[i++];
		}
	}

	if (verbose)
		printf("Read in all %d songs.\n", shallowSongCount);

	for (int i = 255; i >= 0; i--)
	{
		if (musics[i].exists)
		{
			songCount = i+1;
			break;
		}
	}
}

void loadSampleList()
{
	std::string str;
	openTextFile("Addmusic_sample groups.txt", str);

	std::string groupName;
	std::string tempName;

	unsigned int i = 0;
	bool gettingGroupName = false;
	bool gettingSampleName = false;


	while (i < str.length())
	{
		if (gettingGroupName == false && gettingSampleName == false)
		{
			if (groupName.length() == 0)
			{
				if (isspace(str[i]))
				{
					i++;
					continue;
				}
				else if (str[i] == '#')
				{
					i++;
					gettingGroupName = true;
					groupName.clear();
					continue;
				}
			}
			else
			{
				if (isspace(str[i]))
				{
					i++;
					continue;
				}
				else if (str[i] == '{')
				{
					i++;
					gettingSampleName = true;
					continue;
				}
				else
				{
					printError("Error parsing sample groups.txt.  Expected opening curly brace.\n", true);
				}
			}
		}
		else if (gettingGroupName == true)
		{
			if (isspace(str[i]))
			{
				std::unique_ptr<BankDefine> sg = std::make_unique<BankDefine>();
				sg->name = groupName;
				bankDefines.push_back(std::move(sg));
				i++;
				gettingGroupName = false;
				continue;
			}
			else
			{
				groupName += str[i];
				i++;
				continue;
			}
		}
		else if (gettingSampleName)
		{
			if (tempName.length() > 0)
			{
				if (str[i] == '\"')
				{
					tempName.erase(tempName.begin(), tempName.begin() + 1);
					bankDefines[bankDefines.size() - 1]->samples.push_back(std::make_unique<std::string>(tempName));
					bankDefines[bankDefines.size() - 1]->importants.push_back(false);
					tempName.clear();
					i++;
					continue;
				}
				else
				{
					tempName += str[i];
					i++;
				}
			}
			else
			{
				if (isspace(str[i]))
				{
					i++;
					continue;
				}
				else if (str[i] == '\"')
				{
						i++;
						tempName += ' ';
						continue;
				}
				else if (str[i] == '}')
				{
					gettingSampleName = false;
					gettingGroupName = false;
					groupName.clear();
					i++;
					continue;
				}
				else if (str[i] == '!')
				{
					if (bankDefines[bankDefines.size() - 1]->importants.size() == 0)
						printError("Error parsing Addmusic_sample groups.txt: Importance specifier ('!') must come\nafter asample declaration, not before it.", true);
					bankDefines[bankDefines.size() - 1]->importants[bankDefines[bankDefines.size() - 1]->importants.size() - 1] = true;
					i++;
				}
				else
				{
					printError("Error parsing sample groups.txt.  Expected opening quote.\n", true);
				}
			}
		}
	}

//	for (i = 0; (unsigned)i < bankDefines.size(); i++)
//	{
//		for (unsigned int j = 0; j < bankDefines[i]->samples.size(); j++)
//		{
//			for (int k = 0; k < samples.size(); k++)
//			{
//				if (samples[k].name == bankDefines[i]->samples[j]->c_str()) goto sampleExists;
//				//if (strcmp(samples[k].name, bankDefines[i]->samples[j]->c_str()) == 0) goto sampleExists;
//			}
//
//			loadSample(bankDefines[i]->samples[j]->c_str(), &samples[samples.size()]);
//			samples[samples.size()].exists = true;
//sampleExists:;
//		}
//	}
}

void loadSFXList()		// Very similar to loadMusicList, but with a few differences.
{	std::string str;
	openTextFile("Addmusic_sound effects.txt", str);

	if (str[str.length()-1] != '\n')
		str += '\n';

	unsigned int i = 0;

	bool in1DF9 = false;
	bool in1DFC = false;
	bool gettingName = false;
	int index = -1;
	bool isPointer = false;
	bool doNotAdd0 = false;

	std::string tempName;

	int SFXCount = 0;

	while (i < str.length())
	{
		if (isspace(str[i]) && !gettingName)
		{
			i++;
			continue;
		}

		if (strncmp(str.c_str() + i, "SFX1DF9:", 8) == 0)
		{
			in1DF9 = true;
			in1DFC = false;
			i+=8;
			continue;
		}

		if (strncmp(str.c_str() + i, "SFX1DFC:", 8) == 0)
		{
			in1DF9 = false;
			in1DFC = true;
			i+=8;
			continue;
		}

		if (!in1DF9 && !in1DFC)
			printError("Error: Could not find \"SFX1DF9:\" label in sound effects.txt", true);

		if (index < 0)
		{
			if      ('0' <= str[i] && str[i] <= '9') index = str[i++] - '0';
			else if ('A' <= str[i] && str[i] <= 'F') index = str[i++] - 'A' + 10;
			else if ('a' <= str[i] && str[i] <= 'f') index = str[i++] - 'a' + 10;
			else printError("Invalid number in sound effects.txt.", true);

			index <<= 4;


			if      ('0' <= str[i] && str[i] <= '9') index |= str[i++] - '0';
			else if ('A' <= str[i] && str[i] <= 'F') index |= str[i++] - 'A' + 10;
			else if ('a' <= str[i] && str[i] <= 'f') index |= str[i++] - 'a' + 10;
			else if (isspace(str[i])) index >>= 4;
			else printError("Invalid number in sound effects.txt.", true);


			if (!isspace(str[i]))
				printError("Invalid number in sound effects.txt.", true);
		}
		else
		{
			if (str[i] == '*' && tempName.length() == 0)
			{
				isPointer = true;
				i++;
			}
			else if (str[i] == '?' && tempName.length() == 0)
			{
				doNotAdd0 = true;
				i++;
			}
			else if (str[i] == '\n' || str[i] == '\r')
			{

					if (in1DF9)
					{
						if (!isPointer)
							soundEffects[0][index].name = tempName;
						else
							soundEffects[0][index].pointName = tempName;

						soundEffects[0][index].exists = true;

						if (doNotAdd0)
							soundEffects[0][index].add0 = false;
						else
							soundEffects[0][index].add0 = true;

						if (!isPointer)
							openTextFile((std::string("1DF9/") + tempName), soundEffects[0][index].text);
					}
					else
					{
						if (!isPointer)
							soundEffects[1][index].name = tempName;
						else
							soundEffects[1][index].pointName = tempName;

						soundEffects[1][index].exists = true;

						if (doNotAdd0)
							soundEffects[1][index].add0 = false;
						else
							soundEffects[1][index].add0 = true;

						if (!isPointer)
							openTextFile((std::string("1DFC/") + tempName), soundEffects[1][index].text);
					}

					index = -1;
					i++;
					SFXCount++;
					gettingName = false;
					tempName.clear();
					isPointer = false;
					doNotAdd0 = false;
					continue;

			}
			else
			{
				gettingName = true;
				tempName += str[i++];

			}
		}
	}

	if (verbose)
		printf("Read in all %d sound effects.\n", SFXCount);


}

void compileSFX()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 1; j < 256; j++)
		{
			soundEffects[i][j].bank = i;
			soundEffects[i][j].index = j;
			if (soundEffects[i][j].pointName.length() > 0)
			{
				for (int k = 1; k < 256; k++)
				{
					if (soundEffects[i][j].pointName == soundEffects[i][k].name)
					{
						soundEffects[i][j].pointsTo = k;
						break;
					}
				}
				if (soundEffects[i][j].pointsTo == 0)
				{
					std::ostringstream r;
					r << std::hex << j;
					printError(std::string("Error: The sound effect that sound effect 0x") + r.str() + std::string(" points to could not be found."), true);
				}
			}
		}
	}
}

void compileGlobalData()
{
	int DF9DataTotal = 0;
	int DFCDataTotal = 0;
	int DF9Count = 0;
	int DFCCount = 0;

	std::ostringstream oss;

	std::vector<unsigned short> DF9Pointers, DFCPointers;

	for (int i = 255; i >= 0; i--)
	{
		if (soundEffects[0][i].exists)
		{
			DF9Count = i;
			break;
		}
	}

	for (int i = 255; i >= 0; i--)
	{
		if (soundEffects[1][i].exists)
		{
			DFCCount = i;
			break;
		}
	}

	for (int i = 0; i <= DF9Count; i++)
	{
		if (soundEffects[0][i].exists && soundEffects[0][i].pointsTo == 0)
		{
			soundEffects[0][i].posInARAM = DFCCount * 2 + DF9Count * 2 + programPos + programSize + DF9DataTotal;
			soundEffects[0][i].compile();
			DF9Pointers.push_back(DF9DataTotal + (DF9Count + DFCCount) * 2 + programSize + programPos);
			DF9DataTotal += soundEffects[0][i].data.size() + soundEffects[0][i].code.size();
		}
		else if (soundEffects[0][i].exists == false)
		{
			DF9Pointers.push_back(0xFFFF);
		}
		else
		{
			if (i > soundEffects[0][i].pointsTo)
				DF9Pointers.push_back(DF9Pointers[soundEffects[0][i].pointsTo]);
			else
				printError("Error: A sound effect that is a pointer to another sound effect must come after\nthe sound effect that it points to.", true);
		}
	}

	if (errorCount > 0)
		printError("There were errors when compiling the sound effects.  Compilation aborted.  Your\nROM has not been modified.", true);

	for (int i = 0; i <= DFCCount; i++)
	{
		if (soundEffects[1][i].exists && soundEffects[1][i].pointsTo == 0)
		{
			soundEffects[1][i].posInARAM = DFCCount * 2 + DF9Count * 2 + programPos + programSize + DF9DataTotal + DFCDataTotal;
			soundEffects[1][i].compile();
			DFCPointers.push_back(DFCDataTotal + DF9DataTotal + (DF9Count + DFCCount) * 2 + programSize + programPos);
			DFCDataTotal += soundEffects[1][i].data.size() + soundEffects[1][i].code.size();
		}
		else if (soundEffects[1][i].exists == false)
		{
			DFCPointers.push_back(0xFFFF);
		}
		else
		{
			if (i > soundEffects[1][i].pointsTo)
				DFCPointers.push_back(DFCPointers[soundEffects[1][i].pointsTo]);
			else
				printError("Error: A sound effect that is a pointer to another sound effect must come after\nthe sound effect that it points to.", true);
		}
	}

	if (errorCount > 0)
		printError("There were errors when compiling the sound effects.  Compilation aborted.  Your\nROM has not been modified.", true);

	if (verbose)
	{
		std::cout << "Total space used by 1DF9 sound effects: 0x" << std::hex << std::setw(4) << std::uppercase << std::setfill('0') << (DF9DataTotal + DF9Count * 2) << std::dec << std::endl;
		std::cout << "Total space used by 1DFC sound effects: 0x" << std::hex << std::setw(4) << std::uppercase << std::setfill('0') << (DFCDataTotal + DFCCount * 2) << std::dec << std::endl;
	}

	std::cout << "Total space used by all sound effects: 0x" << std::hex << std::setw(4) << std::uppercase << std::setfill('0') << (DF9DataTotal + DF9Count * 2 + DFCDataTotal + DFCCount * 2) << std::dec << std::endl;

	DF9Pointers.erase(DF9Pointers.begin(), DF9Pointers.begin() + 1);
	DFCPointers.erase(DFCPointers.begin(), DFCPointers.begin() + 1);

	writeFile("asm/SFX1DF9Table.bin", DF9Pointers);
	writeFile("asm/SFX1DFCTable.bin", DFCPointers);


	std::vector<uint8_t> allSFXData;

	for (int i = 0; i <= DF9Count; i++)
	{
		for (unsigned int j = 0; j < soundEffects[0][i].data.size(); j++)
			allSFXData.push_back(soundEffects[0][i].data[j]);
		for (unsigned int j = 0; j < soundEffects[0][i].code.size(); j++)
			allSFXData.push_back(soundEffects[0][i].code[j]);
	}



	for (int i = 0; i <= DFCCount; i++)
	{
		for (unsigned int j = 0; j < soundEffects[1][i].data.size(); j++)
			allSFXData.push_back(soundEffects[1][i].data[j]);
		for (unsigned int j = 0; j < soundEffects[1][i].code.size(); j++)
			allSFXData.push_back(soundEffects[1][i].code[j]);
	}

	writeFile("asm/SFXData.bin", allSFXData);

	std::string str;
	openTextFile("asm/main.asm", str);


	int pos;

	pos = str.find("SFXTable0:");
	if (pos == -1) printError("Error: SFXTable0 not found in main.asm.", true);
	str.insert(pos+10, "\r\nincbin \"SFX1DF9Table.bin\"\r\n");

	pos = str.find("SFXTable1:");
	if (pos == -1) printError("Error: SFXTable1 not found in main.asm.", true);
	str.insert(pos+10, "\r\nincbin \"SFX1DFCTable.bin\"\r\nincbin \"SFXData.bin\"\r\n");

	writeTextFile("asm/tempmain.asm", str);

	remove(File("asm/main.bin"));

	if (verbose)
		std::cout << "Compiling main SPC program, pass 2." << std::endl;

	//execute("asar asm/tempmain.asm asm/main.bin 2> temp.log > temp.txt");
	//if (fileExists("temp.log"))
	if (!asarCompileToBIN("asm/tempmain.asm", "asm/main.bin"))
		printError("asar reported an error while assembling asm/main.asm. Refer to temp.log for\ndetails.\n", true);

	programSize = getFileSize("asm/main.bin");

	std::string totalSizeStr;
	if (noSFX) {
		std::cout << "!noSFX is enabled in asm/UserDefines.asm, sound effects are not included" << std::endl;
		totalSizeStr = "Total size of main program: 0x";
	}
	else {
		totalSizeStr = "Total size of main program + all sound effects: 0x";
	}
	std::cout << totalSizeStr << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << programSize  << std::dec << std::endl;

}

void compileMusic()
{
	if (verbose)
		std::cout << "Compiling music..." << std::endl;

	int totalSamplecount = 0;
	int totalSize = 0;
	int maxGlobalEchoBufferSize = 0;
	for (int i = 0; i < 256; i++)
	{
		if (musics[i].exists)
		{
			//if (!(i <= highestGlobalSong && !recompileMain))
			//{
			musics[i].index = i;
			if (i > highestGlobalSong) {
				musics[i].echoBufferSize = std::max(musics[i].echoBufferSize, maxGlobalEchoBufferSize);
			}
			musics[i].compile();
			if (i <= highestGlobalSong) {
				maxGlobalEchoBufferSize = std::max(musics[i].echoBufferSize, maxGlobalEchoBufferSize);
			}
			totalSamplecount += musics[i].mySamples.size();
			//}
		}
	}

	//int songSampleListSize = 0;

	//for (int i = 0; i < songCount; i++)
	//{
	//	songSampleListSize += musics[i].mySampleCount + 1;
	//}

	std::stringstream songSampleList;
	std::string s;

	songSampleListSize = 8;

	songSampleList << "db $53, $54, $41, $52\t\t\t\t; Needed to stop Asar from treating this like an xkas patch.\n";
	songSampleList << "dw SGEnd-SampleGroupPtrs-$01\ndw SGEnd-SampleGroupPtrs-$01^$FFFF\nSampleGroupPtrs:\n\n";

	for (int i = 0; i < songCount; i++)
	{
		if (i % 16 == 0)
			songSampleList << "\ndw ";
		if (musics[i].exists == false)
			songSampleList << "$" << hex4 << 0;
		else
			songSampleList << "SGPointer" << hex2 << i;
		songSampleListSize += 2;

		if (i != songCount - 1 && (i & 0xF) != 0xF)
			songSampleList << ", ";
		//s = songSampleList.str();
	}

	songSampleList << "\n\n";

	for (int i = 0; i < songCount; i++)
	{
		if (!musics[i].exists) continue;

		songSampleListSize++;

		songSampleList << "\n" << "SGPointer" << hex2 << i << ":\n";

		if (i > highestGlobalSong)
		{
			songSampleList << "db $" << hex2 << musics[i].mySamples.size() << "\ndw";
			for (unsigned int j = 0; j < musics[i].mySamples.size(); j++)
			{
				songSampleListSize+=2;
				songSampleList << " $" << hex4 << (int)(musics[i].mySamples[j]);
				if (j != musics[i].mySamples.size() - 1)
					songSampleList << ",";
			}
		}

	}
	songSampleList << "\nSGEnd:";
	s = songSampleList.str();
	std::stringstream tempstream;

	tempstream << "org $" << hex6 << PCToSNES(findFreeSpace(songSampleListSize, bankStart, rom)) << "\n\n" << std::endl;

	s.insert(0, tempstream.str());

	writeTextFile("asm/SNES/SongSampleList.asm", s);
}

void fixMusicPointers()
{
	if (verbose)
		std::cout << "Fixing song pointers..." << std::endl;

	int pointersPos = programSize + 0x400;
	std::stringstream globalPointers;
	std::stringstream incbins;

	int songDataARAMPos = programSize + programPos + highestGlobalSong * 2 + 2;
	//                    size + startPos + pointer to each global song + pointer to local song.
	//int songPointerARAMPos = programSize + programPos;

	bool addedLocalPtr = false;

	for (int i = 0; i < 256; i++)
	{
		if (musics[i].exists == false) continue;

		musics[i].posInARAM = songDataARAMPos;

		int untilJump = -1;

		if (i <= highestGlobalSong)
		{
			globalPointers << "\ndw song" << hex2 << i;
			incbins << "song" << hex2 << i << ": incbin \"" << "SNES/bin/" << "music" << hex2 << i << ".bin\"\n";
		}
		else if (addedLocalPtr == false)
		{
			globalPointers << "\ndw localSong";
			incbins << "localSong: ";
			addedLocalPtr = true;
		}

		for (int j = 0; j < musics[i].spaceForPointersAndInstrs; j+=2)
		{
			if (untilJump == 0)
			{
				j += musics[i].instrumentData.size();
				untilJump = -1;
			}

			int temp = musics[i].allPointersAndInstrs[j] | musics[i].allPointersAndInstrs[j+1] << 8;

			if (temp == 0xFFFF)		// 0xFFFF = swap with 0x0000.
			{
				musics[i].allPointersAndInstrs[j] = 0;
				musics[i].allPointersAndInstrs[j+1] = 0;
				untilJump = 1;
			}
			else if (temp == 0xFFFE)	// 0xFFFE = swap with 0x00FF.
			{
				musics[i].allPointersAndInstrs[j] = 0xFF;
				musics[i].allPointersAndInstrs[j+1] = 0;
				untilJump = 2;
			}
			else if (temp == 0xFFFD)	// 0xFFFD = swap with the song's position (its first track pointer).
			{
				musics[i].allPointersAndInstrs[j] = (musics[i].posInARAM + 2) & 0xFF;
				musics[i].allPointersAndInstrs[j+1] = (musics[i].posInARAM + 2) >> 8;
			}
			else if (temp == 0xFFFC)	// 0xFFFC = swap with the song's position + 2 (its second track pointer).
			{
				musics[i].allPointersAndInstrs[j] = musics[i].posInARAM & 0xFF;
				musics[i].allPointersAndInstrs[j+1] = musics[i].posInARAM >> 8;
			}
			else if (temp == 0xFFFB)	// 0xFFFB = swap with 0x0000, but don't set untilSkip.
			{
				musics[i].allPointersAndInstrs[j] = 0;
				musics[i].allPointersAndInstrs[j+1] = 0;
			}
			else
			{
				temp += musics[i].posInARAM;
				musics[i].allPointersAndInstrs[j] = temp & 0xFF;
				musics[i].allPointersAndInstrs[j+1] = temp >> 8;
			}
			untilJump--;
		}

		int normalChannelsSize = musics[i].data[0].size() + musics[i].data[1].size() + musics[i].data[2].size() + musics[i].data[3].size() + musics[i].data[4].size() + musics[i].data[5].size() + musics[i].data[6].size() + musics[i].data[7].size();

		for (int j = 0; j < 9; j++)
		{
			for (unsigned int k = 0; k < musics[i].loopLocations[j].size(); k++)
			{
				int temp = (musics[i].data[j][musics[i].loopLocations[j][k]] & 0xFF) | (musics[i].data[j][musics[i].loopLocations[j][k] + 1] << 8);
				temp += musics[i].posInARAM + normalChannelsSize + musics[i].spaceForPointersAndInstrs;
				musics[i].data[j][musics[i].loopLocations[j][k]] = temp & 0xFF;
				musics[i].data[j][musics[i].loopLocations[j][k] + 1] = temp >> 8;
			}
		}


		std::vector<uint8_t> final;

		int sizeWithPadding = (musics[i].minSize > 0) ? musics[i].minSize : musics[i].totalSize;

		if (i > highestGlobalSong)
		{
			int RATSSize = musics[i].totalSize + 4 - 1;
			final.push_back('S');
			final.push_back('T');
			final.push_back('A');
			final.push_back('R');

			final.push_back(RATSSize & 0xFF);
			final.push_back(RATSSize >> 8);

			final.push_back(~RATSSize & 0xFF);
			final.push_back(~RATSSize >> 8);

			final.push_back(sizeWithPadding & 0xFF);
			final.push_back(sizeWithPadding >> 8);

			final.push_back(songDataARAMPos & 0xFF);
			final.push_back(songDataARAMPos >> 8);
		}


		for (unsigned int j = 0; j < musics[i].allPointersAndInstrs.size(); j++)
			final.push_back(musics[i].allPointersAndInstrs[j]);

		for (unsigned int j = 0; j < 9; j++)
			for (unsigned int k = 0; k < musics[i].data[j].size(); k++)
				final.push_back(musics[i].data[j][k]);

		if (musics[i].minSize > 0 && i <= highestGlobalSong)
			while (final.size() < musics[i].minSize)
				final.push_back(0);


		if (i > highestGlobalSong)
		{
			musics[i].finalData.resize(final.size() - 12);
			musics[i].finalData.assign(final.begin() + 12, final.end());
		}

		std::stringstream fname;
		fname << "asm/SNES/bin/music" << hex2 << i << ".bin";
		writeFile(fname.str(), final);



		if (i <= highestGlobalSong)
		{
			songDataARAMPos += sizeWithPadding;
		}
		else
		{
			if (checkEcho)
			{
				musics[i].spaceInfo.songStartPos = songDataARAMPos;
				musics[i].spaceInfo.songEndPos = musics[i].spaceInfo.songStartPos + sizeWithPadding;

				int checkPos = songDataARAMPos + sizeWithPadding;
				if ((checkPos & 0xFF) != 0) checkPos = ((checkPos >> 8) + 1) << 8;

				musics[i].spaceInfo.sampleTableStartPos = checkPos;

				checkPos += musics[i].mySamples.size() * 4;

				musics[i].spaceInfo.sampleTableEndPos = checkPos;

				int importantSampleCount = 0;
				for (unsigned int j = 0; j < musics[i].mySamples.size(); j++)
				{
					auto thisSample = musics[i].mySamples[j];
					auto thisSampleSize = samples[thisSample].data.size();
					bool sampleIsImportant = samples[thisSample].important;
					if (sampleIsImportant) importantSampleCount++;

					musics[i].spaceInfo.individualSampleStartPositions.push_back(checkPos);
					musics[i].spaceInfo.individualSampleEndPositions.push_back(checkPos + thisSampleSize);
					musics[i].spaceInfo.individialSampleIsImportant.push_back(sampleIsImportant);

					checkPos += thisSampleSize;
				}
				musics[i].spaceInfo.importantSampleCount = importantSampleCount;

				if ((checkPos & 0xFF) != 0) checkPos = ((checkPos >> 8) + 1) << 8;

				//musics[i].spaceInfo.echoBufferStartPos = checkPos;

				checkPos += musics[i].echoBufferSize << 11;

				//musics[i].spaceInfo.echoBufferEndPos = checkPos;

				musics[i].spaceInfo.echoBufferEndPos = 0x10000;
				if (musics[i].echoBufferSize > 0)
				{
					musics[i].spaceInfo.echoBufferStartPos = 0x10000 - (musics[i].echoBufferSize << 11);
					musics[i].spaceInfo.echoBufferEndPos = 0x10000;
				}
				else
				{
					musics[i].spaceInfo.echoBufferStartPos = 0xFF00;
					musics[i].spaceInfo.echoBufferEndPos = 0xFF04;
				}


				if (checkPos > 0x10000)
				{
					std::cerr << musics[i].name << ": Echo buffer exceeded total space in ARAM by 0x" << hex4 << checkPos - 0x10000 << " bytes." << std::dec << std::endl;
					quit(1);
				}
			}
		}
	}

	//if (recompileMain)
	//{
	std::string patch;
	openTextFile("asm/tempmain.asm", patch);

	patch += globalPointers.str() + "\n" + incbins.str();

	writeTextFile("asm/tempmain.asm", patch);

	if (verbose)
		std::cout << "Compiling main SPC program, final pass." << std::endl;

	//removeFile("asm/SNES/bin/main.bin");

	//execute("asar asm/tempmain.asm asm/SNES/bin/main.bin 2> temp.log > temp.txt");

	//if (fileExists("temp.log"))
	if (!asarCompileToBIN("asm/tempmain.asm", "asm/SNES/bin/main.bin"))
		printError("asar reported an error while assembling asm/main.asm. Refer to temp.log for\ndetails.\n", true);
	//}

	programSize = getFileSize("asm/SNES/bin/main.bin");

	std::vector<uint8_t> temp;
	openFile("asm/SNES/bin/main.bin", temp);
	std::vector<uint8_t> temp2;
	temp2.resize(temp.size() + 4);
	temp2[0] = programSize & 0xFF;
	temp2[1] = programSize >> 8;
	temp2[2] = programPos & 0xFF;
	temp2[3] = programPos >> 8;
	for (unsigned int i = 0; i < temp.size(); i++)
		temp2[4+i] = temp[i];
	writeFile("asm/SNES/bin/main.bin", temp2);

	if (verbose)
		std::cout << "Total space in ARAM left for local songs: 0x" << hex4 << (0x10000 - programSize - 0x400) << " bytes." << std::dec << std::endl;

	int defaultIndex = -1, optimizedIndex = -1;
	for (unsigned int i = 0; i < bankDefines.size(); i++)
	{
		if (bankDefines[i]->name == "default")
			defaultIndex = i;
		if (bankDefines[i]->name == "optimized")
			optimizedIndex = i;
	}

	// Can't do this now since we can't get a sample correctly outside of a song.

	/*if (defaultIndex != -1)
	{
		int groupSize = 0;
		for (unsigned int i = 0; i < bankDefines[defaultIndex]->samples.size(); i++)
		{
			int j = getSample(File(*(bankDefines[defaultIndex]->samples[i])));
			if (j == -1) goto end1;
			groupSize += samples[j].data.size() + 4;
		}

		std::cout << "Total space in ARAM for local songs using #default: 0x" << hex4 << (0x10000 - programSize - 0x400 - groupSize) << " bytes." << std::dec << std::endl;
	}
end1:

	if (optimizedIndex != -1)
	{
		int groupSize = 0;
		for (unsigned int i = 0; i < bankDefines[optimizedIndex]->samples.size(); i++)
		{
			int j = getSample(File(*(bankDefines[optimizedIndex]->samples[i])));
			if (j == -1) goto end2;
			groupSize += samples[j].data.size() + 4;
		}

		std::cout << "Total space in ARAM for local songs using #optimized: 0x" << hex4 << (0x10000 - programSize - 0x400 - groupSize) << " bytes." << std::dec << std::endl;
	}*/
end2:;
}

void generateSPCs()
{
	if (checkEcho == false)		// If echo buffer checking is off, then the overflow may be due to too many samples.
		return;			// In this case, trying to generate an SPC would crash.
	//uint8_t base[0x10000];

	std::vector<uint8_t> programData;
	openFile("asm/SNES/bin/main.bin", programData);
	programData.erase(programData.begin(), programData.begin() + 4);	// Erase the upload data.
	unsigned int i;

	unsigned int localPos;


	//for (i = 0; i < programPos; i++) base[i] = 0;

	//for (i = 0; i < programSize; i++)
	//	base[i + programPos] = programData[i];

	localPos = programData.size() + programPos;

	std::vector<uint8_t> SPC, SPCBase, DSPBase;
	openFile("asm/SNES/SPCBase.bin", SPCBase);
	openFile("asm/SNES/SPCDSPBase.bin", DSPBase);
	SPC.resize(0x10200);

	int SPCsGenerated = 0;

	bool forceAll = false;
	/*
	time_t recentMod = 0;			// If any main program modifications were made, we need to update all SPCs.
	for (int i = 1; i <= highestGlobalSong; i++)
		recentMod = std::max(recentMod, getTimeStamp((File)("music/" + musics[i].name)));

	recentMod = std::max(recentMod, getTimeStamp((File)"asm/main.asm"));
	recentMod = std::max(recentMod, getTimeStamp((File)"asm/commands.asm"));
	recentMod = std::max(recentMod, getTimeStamp((File)"asm/InstrumentData.asm"));
	recentMod = std::max(recentMod, getTimeStamp((File)"asm/CommandTable.asm"));
	recentMod = std::max(recentMod, getTimeStamp((File)"Addmusic_sound effects.txt"));
	recentMod = std::max(recentMod, getTimeStamp((File)"Addmusic_sample groups.txt"));
	recentMod = std::max(recentMod, getTimeStamp((File)"AddmusicK.exe"));

	for (int i = 1; i < 256; i++)
	{
		if (soundEffects[0][i].exists)
			recentMod = std::max(recentMod, getTimeStamp((File)((std::string)"1DF9/" + soundEffects[0][i].getEffectiveName())));
	}

	for (int i = 1; i < 256; i++)
	{
		if (soundEffects[1][i].exists)
			recentMod = std::max(recentMod, getTimeStamp((File)((std::string)"1DFC/" + soundEffects[1][i].getEffectiveName())));
	}
	*/
	int mode = 0;		// 0 = dump music, 1 = dump SFX1, 2 = dump SFX2
	int maxMode = 0;
	if (sfxDump == true) maxMode = 2;

	for (int mode = 0; mode <= maxMode; mode++)
	{

		for (unsigned int i = 0; i < 256; i++)
		{
			if (mode == 0 && musics[i].exists == false) continue;
			if (mode == 1 && soundEffects[0][i].exists == false) continue;
			if (mode == 2 && soundEffects[1][i].exists == false) continue;

			if (mode == 0 && i <= highestGlobalSong) continue;		// Cannot generate SPCs for global songs as required samples, SRCN table, etc. cannot be determined.


			//time_t spcTimeStamp = getTimeStamp((File)fname);

			/*if (!forceSPCGeneration)
				if (fileExists(fname))
				if (getTimeStamp((File)("music/" + musics[i].name)) < spcTimeStamp)
				if (getTimeStamp((File)"./samples") < spcTimeStamp)
				if (recentMod < spcTimeStamp)
				continue;*/

			SPCsGenerated++;
			int y = 2;					// Used to generate 2 SPCs for tracks with Yoshi drums.
			if (mode != 0) y = 1;
			while (y > 0)
			{
				if (mode == 0 && musics[i].hasYoshiDrums == false && y == 2)
				{
					y--;
					continue;
				}
				SPC.clear();
				SPC.resize(0x10200);

				for (unsigned int j = 0; j < SPCBase.size(); j++)
					SPC[j] = SPCBase[j];

				if (mode == 0) { for (unsigned int j = 0; j < 32; j++) if (j < musics[i].title.length())    SPC[j + 0x2E] = musics[i].title[j];    else SPC[j + 0x2E] = 0; }
				if (mode == 0) { for (unsigned int j = 0; j < 32; j++) if (j < musics[i].game.length())     SPC[j + 0x4E] = musics[i].game[j];     else SPC[j + 0x4E] = 0; }
				if (mode == 0) { for (unsigned int j = 0; j < 32; j++) if (j < musics[i].comment.length())  SPC[j + 0x7E] = musics[i].comment[j];  else SPC[j + 0x7E] = 0; }
				if (mode == 0) { for (unsigned int j = 0; j < 32; j++) if (j < musics[i].author.length())   SPC[j + 0xB1] = musics[i].author[j];   else SPC[j + 0xB1] = 0; }

				for (int j = 0; j < programSize; j++)
					SPC[0x100 + programPos + j] = programData[j];


				int backupIndex = i;
				if (mode != 0) {
					i = highestGlobalSong + 1;
					for (int j = highestGlobalSong+1; j < 256; j++) {
						if (musics[j].exists) {
							i = j;		// While dumping SFX, pretend that the current song is the lowest valid local song
							break;
						}
					}
				}

				if (mode == 0)
				{

					for (unsigned int j = 0; j < musics[i].finalData.size(); j++)
						SPC[localPos + 0x100 + j] = musics[i].finalData[j];


				}

				int tablePos = localPos + musics[i].finalData.size();

				if ((tablePos & 0xFF) != 0)
					tablePos = (tablePos & 0xFF00) + 0x100;

				int samplePos = tablePos + musics[i].mySamples.size() * 4;

				for (unsigned int j = 0; j < musics[i].mySamples.size(); j++)
				{
					SPC[tablePos + j * 4 + 0x100] = samplePos & 0xFF;
					SPC[tablePos + j * 4 + 0x101] = samplePos >> 8;
					unsigned short loopPoint = samples[musics[i].mySamples[j]].loopPoint;
					unsigned short newLoopPoint = loopPoint + samplePos;
					SPC[tablePos + j * 4 + 0x102] = newLoopPoint & 0xFF;
					SPC[tablePos + j * 4 + 0x103] = newLoopPoint >> 8;

					for (unsigned int k = 0; k < samples[musics[i].mySamples[j]].data.size(); k++)
						SPC[samplePos + 0x100 + k] = samples[musics[i].mySamples[j]].data[k];

					samplePos += samples[musics[i].mySamples[j]].data.size();
				}

				for (unsigned int j = 0; j < DSPBase.size(); j++)
					SPC[0x10100 + j] = DSPBase[j];

				SPC[0x1015D] = tablePos >> 8;

				SPC[0x15F] = 0x20;

				if (y == 2) SPC[0x01f5] = 2;

				SPC[0xA9] = (musics[i].seconds / 100 % 10) + '0';		// Why on Earth is the value stored as plain text...?
				SPC[0xAA] = (musics[i].seconds / 10 % 10) + '0';
				SPC[0xAB] = (musics[i].seconds / 1 % 10) + '0';

				SPC[0xAC] = '1';
				SPC[0xAD] = '0';
				SPC[0xAE] = '0';
				SPC[0xAF] = '0';
				SPC[0xB0] = '0';

				SPC[0x25] = mainLoopPos & 0xFF;	// Set the PC to the main loop.
				SPC[0x26] = mainLoopPos >> 8;	// The values of the registers (besides stack which is in the file) don't matter.  They're 0 in the base file.

				i = backupIndex;

				if (mode == 0)
				{
					SPC[0x1F6] = highestGlobalSong + 1;	// Tell the SPC to play this song.
				}
				else if (mode == 1)
				{
					SPC[0x1F4] = i;				// Tell the SPC to play this SFX
				}
				else if (mode == 2)
				{
					SPC[0x1F7] = i;				// Tell the SPC to play this SFX
				}




				char buffer[11];
				time_t t = time(NULL);
				strftime(buffer, 11, "%m/%d/%Y", localtime(&t));
				strncpy((char *)SPC.data() + 0x9E, buffer, 10);


				std::string pathlessSongName;
				if (mode == 0)
					pathlessSongName = musics[i].name;
				else if (mode == 1)
					pathlessSongName = soundEffects[0][i].name;
				else if (mode == 2)
					pathlessSongName = soundEffects[1][i].name;


				int extPos = pathlessSongName.find_last_of('.');
				if (extPos != -1)
					pathlessSongName = pathlessSongName.substr(0, extPos);


				if (pathlessSongName.find('/') != -1)
					pathlessSongName = pathlessSongName.substr(pathlessSongName.find_last_of('/') + 1);
				else if (pathlessSongName.find('\\') != -1)
					pathlessSongName = pathlessSongName.substr(pathlessSongName.find_last_of('\\') + 1);

				if (mode == 0) musics[i].pathlessSongName = pathlessSongName;
				if (mode == 1) pathlessSongName.insert(0, "1DF9/");
				if (mode == 2) pathlessSongName.insert(0, "1DFC/");

				std::string fname = "SPCs/" + pathlessSongName;


				if (y == 2)
					fname += " (Yoshi)";

				fname += ".spc";

				if (verbose)
					std::cout << "Wrote \"" << fname << "\" to file." << std::endl;

				writeFile(fname, SPC);
				y--;
			}

		}
	}

	if (verbose)
	{
		if (SPCsGenerated == 1)
			std::cout << "Generated 1 SPC file." << std::endl;
		else if (SPCsGenerated > 0)
			std::cout << "Generated " << SPCsGenerated << " SPC files." << std::endl;
	}
}

void assembleSNESDriver2()
{
	if (verbose)
		std::cout << "\nGenerating SNES driver...\n" << std::endl;

	std::string patch;

	//removeFile("asm/SNES/temppatch.sfc");
	//openTextFile("asm/SNES/patch.asm", patch);

	//writeTextFile("asm/SNES/temppatch.asm", patch);

	//execute("asar asm/SNES/temppatch.asm 2> temp.log");
	//if (fileExists("temp.log"))
	//{
	//	std::cout << "asar reported an error assembling patch.asm. Refer to temp.log for details." << std::endl;
	//	quit(1);
	//}

	//std::vector<uint8_t> patchBin;
	//openFile("asm/SNES/temppatch.sfc", patchBin);

	openTextFile("asm/SNES/patch.asm", patch);

	insertValue(reuploadPos, 4, "!ExpARAMRet = ", patch);
	insertValue(SRCNTableCodePos, 4, "!TabARAMRet = ", patch);
	insertValue(mainLoopPos, 4, "!DefARAMRet = ", patch);
	insertValue(songCount, 2, "!SongCount = ", patch);

	int pos;

	pos = patch.find("MusicPtrs:");
	if (pos == -1)
	{
		std::cerr << "Error: \"MusicPtrs:"" could not be found." << std::endl;		// // //
		quit(1);
	}

	patch = patch.substr(0, pos);

	{
		std::string patch2;
		openTextFile("asm/SNES/patch2.asm", patch2);
		patch += patch2;
	}

	std::stringstream musicPtrStr; musicPtrStr << "MusicPtrs: \ndl ";
	std::stringstream samplePtrStr; samplePtrStr << "\n\nSamplePtrs:\ndl ";
	std::stringstream sampleLoopPtrStr; sampleLoopPtrStr << "\n\nSampleLoopPtrs:\ndw ";
	std::stringstream musicIncbins; musicIncbins << "\n\n";
	std::stringstream sampleIncbins; sampleIncbins << "\n\n";

	if (verbose)
		std::cout << "Writing music files..." << std::endl;

	for (int i = 0; i < songCount; i++)
	{
		if (musics[i].exists == true && i > highestGlobalSong)
		{
			int requestSize;
			int freeSpace;
			std::stringstream musicBinPath;
			musicBinPath << "asm/SNES/bin/music" << hex2 << i << ".bin";
			requestSize = getFileSize(musicBinPath.str());
			freeSpace = findFreeSpace(requestSize, bankStart, rom);
			if (freeSpace == -1)
			{
				printError("Error: Your ROM is out of free space.", true);
			}

			freeSpace = PCToSNES(freeSpace);
			musicPtrStr << "music" << hex2 << i << "+8";
			musicIncbins << "org $" << hex6 << freeSpace << "\nmusic" << hex2 << i << ": incbin \"bin/music" << hex2 << i << ".bin\"" << std::endl;
		}
		else
		{
			musicPtrStr << "$" << hex6 << 0;

		}

		if ((i & 0xF) == 0xF && i != songCount-1)
			musicPtrStr << "\ndl ";
		else if (i != songCount-1)
			musicPtrStr << ", ";
	}

	if (verbose)
		std::cout << "Writing sample files..." << std::endl;

	for (int i = 0; i < samples.size(); i++)
	{
		if (samples[i].exists)
		{
			std::vector<uint8_t> temp;
			temp.resize(samples[i].data.size() + 10);
			temp[0] = 'S';
			temp[1] = 'T';
			temp[2] = 'A';
			temp[3] = 'R';

			temp[4] = (samples[i].data.size()+2-1) & 0xFF;
			temp[5] = (samples[i].data.size()+2-1) >> 8;

			temp[6] = (~(samples[i].data.size()+2-1) & 0xFF);
			temp[7] = (~(samples[i].data.size()+2-1) >> 8);

			temp[8] = samples[i].data.size() & 0xFF;
			temp[9] = samples[i].data.size() >> 8;

			for (unsigned int j = 0; j < samples[i].data.size(); j++)
				temp[j+10] = samples[i].data[j];
			std::stringstream filename;
			filename << "asm/SNES/bin/brr" << hex2 << i << ".bin";
			writeFile(filename.str(), temp);

			int requestSize = getFileSize(filename.str());
			int freeSpace = findFreeSpace(requestSize, bankStart, rom);
			if (freeSpace == -1)
			{
				printError("Error: Your ROM is out of free space.", true);
			}

			freeSpace = PCToSNES(freeSpace);
			samplePtrStr << "brr" << hex2 << i << "+8";
			sampleIncbins << "org $" << hex6 << freeSpace << "\nbrr" << hex2 << i << ": incbin \"bin/brr" << hex2 << i << ".bin\"" << std::endl;

		}
		else
			samplePtrStr << "$" << hex6 << 0;

		sampleLoopPtrStr << "$" << hex4 << samples[i].loopPoint;


		if ((i & 0xF) == 0xF && i != samples.size()-1)
		{
			samplePtrStr << "\ndl ";
			sampleLoopPtrStr << "\ndw ";
		}
		else if (i != samples.size()-1)
		{
			samplePtrStr << ", ";
			sampleLoopPtrStr << ", ";
		}
	}

	patch += "pullpc\n\n";

	musicPtrStr << "\ndl $FFFFFF\n";
	samplePtrStr << "\ndl $FFFFFF\n";

	patch += musicPtrStr.str();
	patch += samplePtrStr.str();
	patch += sampleLoopPtrStr.str();

	//patch += "";

	patch += musicIncbins.str();
	patch += sampleIncbins.str();

	insertValue(highestGlobalSong, 2, "!GlobalMusicCount = #", patch);

	std::stringstream ss;
	ss << "\n\norg !SPCProgramLocation" << "\nincbin \"bin/main.bin\"";
	patch += ss.str();

	remove("asm/SNES/temppatch.sfc");

	std::string undoPatch;
	openTextFile("asm/SNES/AMUndo.asm", undoPatch);
	patch.insert(patch.begin(), undoPatch.begin(), undoPatch.end());

	writeTextFile("asm/SNES/temppatch.asm", patch);

	if (verbose)
		std::cout << "Final compilation..." << std::endl;

	if (!doNotPatch)
	{

		//execute("asar asm/SNES/temppatch.asm asm/SNES/temp.sfc 2> temp.log");
		//if (fileExists("temp.log"))
		if (!asarPatchToROM("asm/SNES/temppatch.asm", "asm/SNES/temp.sfc"))
			printError("asar reported an error.  Refer to temp.log for details.", true);

		//execute("asar asm/SNES/tweaks.asm asm/SNES/temp.sfc 2> temp.log");
		//if (fileExists("temp.log"))
		//	printError("asar reported an error.  Refer to temp.log for details.", true);

		//execute("asar asm/SNES/NMIFix.asm asm/SNES/temp.sfc 2> temp.log");
		//if (fileExists("temp.log"))
		//	printError("asar reported an error.  Refer to temp.log for details.", true);


		std::vector<uint8_t> final;
		final = romHeader;

		std::vector<uint8_t> tempsfc;
		openFile("asm/SNES/temp.sfc", tempsfc);

		for (unsigned int i = 0; i < tempsfc.size(); i++)
			final.push_back(tempsfc[i]);

		fs::remove((std::string)ROMName + "~");		// // //
		fs::rename((std::string)ROMName, (std::string)ROMName + "~");		// // //

		writeFile(ROMName, final);

	}
}

void generateMSC()
{
	std::string mscname = ((std::string)ROMName).substr(0, (unsigned int)((std::string)ROMName).find_last_of('.'));

	mscname += ".msc";

	std::stringstream text;

	for (int i = 0; i < 256; i++)
	{
		if (musics[i].exists)
		{
			text << hex2 << i << "\t" << 0 << "\t" << musics[i].title << "\n";
			text << hex2 << i << "\t" << 1 << "\t" << musics[i].title << "\n";
			//fprintf(fout, "%2X\t0\t%s\n", i, musics[i].title.c_str());
			//fprintf(fout, "%2X\t1\t%s\n", i, musics[i].title.c_str());
		}
	}
	writeTextFile(mscname, text.str());
}

void cleanUpTempFiles()
{
	if (doNotPatch)		// If this is specified, then the user might need these temp files.  Keep them.
		return;

	removeFile("asm/tempmain.asm");
	removeFile("asm/main.bin");
	removeFile("asm/SFX1DF9Table.bin");
	removeFile("asm/SFX1DFCTable.bin");
	removeFile("asm/SFXData.bin");

	removeFile("asm/SNES/temppatch.asm");
	removeFile("asm/SNES/temp.sfc");
	removeFile("temp.log");
	removeFile("temp.txt");
}

void tryToCleanSampleToolData()
{
	unsigned int i;
	bool found = false;

	for (i = 0; i < rom.size() - 50; i++)
	{

		if (strncmp((char *)rom.data() + i, "New Super Mario World Sample Utility 2.0 by smkdan", 50) == 0)
		{
			found = true;
			break;
		}
	}

	if (found == false) return;

	std::cout << "Sample Tool detected.  Erasing data..." << std::endl;

	int hackPos = i - 8;

	i += 0x36;

	int sizeOfErasedData = 0;

	bool removed[0x100];
	for (int j = 0; j < 0x100; j++) removed[j] = false;

	for (int j = 0; j < 0x207; j++)
	{
		if (removed[rom[j+i]]) continue;
		sizeOfErasedData += clearRATS(SNESToPC(rom[j+i]* 0x10000 + 0x8000));
		removed[rom[j+i]] = true;
	}

	int sampleDataSize = sizeOfErasedData;


	sizeOfErasedData += clearRATS(hackPos);

	std::cout << "Erased 0x" << hex6 << sizeOfErasedData << " bytes, of which 0x" << sampleDataSize << " were sample data.";
}

void tryToCleanAM4Data()
{
	if ((rom.size() % 0x8000 != 0 && rom[0x1940] == 0x22) || (rom.size() % 0x8000 == 0 && rom[0x1740] == 0x22))
	{
		if (rom.size() % 0x8000 == 0)
			printError("Addmusic 4.05 ROMs can only be cleaned if they have a header. This does not\napply to any other aspect of the program.", true);

		char **am405argv;
		am405argv = (char **)malloc(sizeof(char **) * 2);
		am405argv[1] = (char *)malloc(ROMName.size() + 1);
		strcpy(am405argv[1], ROMName.cStr());
		am405argv[1][ROMName.size()] = 0;
		std::cout << "Attempting to erase data from Addmusic 4.05:" << std::endl;
		removeAM405Data(2, am405argv);
		openFile(ROMName, rom);					// Reopen the file.
		if (rom[0x255] == 0x5C)
		{
			int moreASMData = ((rom[0x255+3] << 16) | (rom[0x255+2] << 8) | (rom[0x255+1])) - 8;
			clearRATS(SNESToPC(moreASMData));
		}
		int romiSPCProgramAddress = (unsigned char)rom[0x2E9] | ((unsigned char)rom[0x2EE]<<8) | ((unsigned char)rom[0x2F3]<<16);
		clearRATS(SNESToPC(romiSPCProgramAddress) - 12 + 0x200);

	}
}

void tryToCleanAMMData()
{
	if ((rom.size() % 0x8000 != 0 && findRATS(0x078200)) || (rom.size() % 0x8000 == 0 && findRATS(0x078000)))		// Since RATS tags only need to be in banks 0x10+, a tag here signals AMM usage.
	{
		if (rom.size() % 0x8000 == 0)
			printError("AddmusicM ROMs can only be cleaned if they have a header. This does not\napply to any other aspect of the program.", true);

		if (fileExists("INIT.asm") == false)
			printError("AddmusicM was detected.  In order to remove it from this ROM, you must put\nAddmusicM's INIT.asm as well as xkasAnti and a clean ROM (named clean.smc) in\nthe same folder as this program. Then attempt to run this program once more.", true);

		std::cout << "AddmusicM detected.  Attempting to remove..." << std::endl;
		execute( ((std::string)("perl addmusicMRemover.pl ")) + (std::string)ROMName);
		execute( ((std::string)("xkasAnti clean.smc ")) + (std::string)ROMName + "INIT.asm");
	}
}


void checkMainTimeStamps()			// Disabled for now, as this only works if the ROM is linked to the program (so it wouldn't work if the program was used on multiple ROMs)
{						// It didn't save much time anyway...
	recompileMain = true;
	return;


	/*
	if (!fileExists("asm/SNES/bin/main.bin"))
	{
		goto recompile;				// Laziness!
	}
	if (strncmp((char *)(rom.data() + 0x70000), "@AMK", 4) != 0)
	{
		goto recompile;				// More laziness!
	}

	for (int i = 1; i <= highestGlobalSong; i++)
		mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)("music/" + musics[i].name)));

	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/main.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/commands.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/InstrumentData.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/CommandTable.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/SNES/patch.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/SNES/patch2.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"asm/SNES/tweaks.asm"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"Addmusic_list.txt"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"Addmusic_sound effects.txt"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"Addmusic_sample groups.txt"));
	mostRecentMainModification = std::max(mostRecentMainModification, getTimeStamp((File)"AddmusicK.exe"));

	for (int i = 1; i < 256; i++)
	{
		if (soundEffects[0][i].exists)
			std::max(mostRecentMainModification, getTimeStamp((File)((std::string)"1DF9/" + soundEffects[0][i].getEffectiveName())));
	}

	for (int i = 1; i < 256; i++)
	{
		if (soundEffects[1][i].exists)
			std::max(mostRecentMainModification, getTimeStamp((File)((std::string)"1DFC/" + soundEffects[1][i].getEffectiveName())));
	}

	if (mostRecentMainModification > getTimeStamp((File)"asm/SNES/bin/main.bin"))
	{
		std::cout << "Changes have been made to the global program.  Recompiling...\n" << std::endl;
recompile:
		recompileMain = true;
	}
	else
	{
		recompileMain = false;
	}
	*/
}

void generatePNGs()
{
	for (auto &current : musics)
	{
		if (current.index <= highestGlobalSong) continue;
		if (current.exists == false) continue;

		std::vector<unsigned char> bitmap;
		// 1024 pixels wide, 64 pixels tall, 4 bytes per pixel

		const int width = 1024;
		const int height = 64;

		bitmap.resize(width * height * 4);

		int x = 0;
		int y = 0;


		for (int i = 0; i < width * height; i++)
		{
			unsigned char r = 0;
			unsigned char g = 0;
			unsigned char b = 0;
			unsigned char a = 255;

			if (i >= 0 && i < programPos)
			{
				r = 255;
			}
			else if (i >= programPos && i < programPos + programSize)
			{
				r = 255;
				g = 255;
			}
			else if (i >= current.spaceInfo.songStartPos && i < current.spaceInfo.songEndPos)
			{
				g = 128;
			}
			else if (i >= current.spaceInfo.sampleTableStartPos && i < current.spaceInfo.sampleTableEndPos)
			{
				g = 255;
			}
			else if (i >= current.spaceInfo.individualSampleStartPositions[0] && i < current.spaceInfo.individualSampleEndPositions[current.spaceInfo.individualSampleEndPositions.size() - 1])
			{
				int currentSampleIndex = 0;

				for (auto currentSampleEndPos : current.spaceInfo.individualSampleEndPositions)
				{
					if (currentSampleEndPos > i) break;

					currentSampleIndex++;
				}

				bool sampleIsImportant = current.spaceInfo.individialSampleIsImportant[currentSampleIndex];

				int sampleCount = current.spaceInfo.individualSampleStartPositions.size();

				b = static_cast<unsigned char>(static_cast<double>(currentSampleIndex) / static_cast<double>(sampleCount)* 127.0 + 128.0);


				if (sampleIsImportant)
				{
					g = static_cast<unsigned char>(static_cast<double>(currentSampleIndex) / static_cast<double>(sampleCount)* 127.0 + 128.0);
				}
			}
			else if (i >= current.spaceInfo.echoBufferStartPos && i < current.spaceInfo.echoBufferEndPos)
			{
				r = 160;
				b = 160;
			}
			else if (i >= current.spaceInfo.echoBufferEndPos)
			{
				r = 63;
				b = 63;
				g = 63;
			}

			int bitmapIndex = y * width + x;


			if ((bitmapIndex*4)+3 >= bitmap.size())
				break;				// This should never happen, but let's be safe.

			bitmap[(bitmapIndex*4)+0] = r;
			bitmap[(bitmapIndex*4)+1] = g;
			bitmap[(bitmapIndex*4)+2] = b;
			bitmap[(bitmapIndex*4)+3] = a;

			y++;
			if (y >= height)
			{
				y = 0;
				x++;
			}
		}

		auto path = current.pathlessSongName;
		path = "Visualizations/" + path + ".png";
		lodepng::encode(path, bitmap, width, height);


	}


}


