#include <experimental/filesystem>		// // //
namespace fs = std::experimental::filesystem;
#include <fstream>

void openTextFile(const fs::path &fileName, std::string &s) {		// // //
	if (auto is = std::ifstream(fileName))
		s.assign(std::istreambuf_iterator<char> {is}, std::istreambuf_iterator<char> { });
}

void writeTextFile(const fs::path &fileName, const std::string &string) {		// // //
	if (auto ofs = std::ofstream(fileName, std::ios::binary)) {
		std::string n = string;
#ifdef _WIN32
		unsigned int i = 0;
		while (i < n.length()) {
			if (n[i] == '\n')
				n = n.insert(i++, "\r");
			++i;
		}
#endif
		ofs.write(n.c_str(), n.size());
	}
}

int main(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
		std::string s;
		openTextFile(argv[i], s);
		if (s.find("#amm") == std::string::npos &&
			s.find("#amk") == std::string::npos &&
			s.find("#am4") == std::string::npos) {
			s.append("\n\n#amm");
			writeTextFile(argv[i], s);
		}
	}

	return 0;
}
