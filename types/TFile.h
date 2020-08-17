#ifndef TKLB_STACK
#define TKLB_STACK

#ifdef _WIN32
	#include "../external/dirent.h"
#else
	#include "dirent.h"
	#include <sys/stat.h>
#endif

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cassert>

namespace tklb {

const std::string PATH_DELIMITER =
#ifdef _WIN32
	"\\";
#else
	"/";
#endif

struct FileInfo {
	std::string name;
	std::string relative;
	std::string absolute;
	bool isFolder = false;
	std::vector<FileInfo> children;

	std::vector<FileInfo> scanDir(FileInfo& root, bool recursive = false) {
		std::vector<FileInfo> ret;
		struct dirent** files = nullptr;
		const int count = scandir(root.absolute.c_str(), &files, nullptr, alphasort);
		if (count >= 0) {
			for (int i = 0; i < count; i++) {
				const struct dirent* ent = files[i];
				if (ent->d_name[0] != '.') {
					FileInfo info;
					info.isFolder = ent->d_type == DT_DIR;
					info.name = ent->d_name;
					info.relative = root.relative + info.name + (info.isFolder ? PATH_DELIMITER : "");
					info.absolute = root.absolute + info.name + (info.isFolder ? PATH_DELIMITER : "");
					root.children.push_back(info);
					if (recursive && info.isFolder) {
						scanDir(info, true);
					}
					ret.push_back(info);
				}
			free(files[i]);
		}
		free(files);
		} else {
			root.isFolder = false;
		}
		return ret;
	}

	std::vector<FileInfo> scanDir(const std::string path, const bool recursive = false) {
		absolute = path;
		relative = "." + PATH_DELIMITER;
		name = "";
		return scanDir(*this, recursive);
	}

	bool isWaveName() {
		return name.length() - name.find_last_of(".WAV") == 4
			|| name.length() - name.find_last_of(".wav") == 4;
	}

	bool isJSONName() {
		return name.length() - name.find_last_of(".JSON") == 5
			|| name.length() - name.find_last_of(".json") == 5;
	}

	bool createFolder() {
		const char* path = absolute.c_str();
		bool success = true;
#ifdef _WIN32
		bool ok = CreateDirectory(path, nullptr);
		if (!ok) {
			if (GetLastError() != ERROR_ALREADY_EXISTS) {
				success = false; // Probably permission, locked file etc
			}
		}
#else
		mode_t process_mask = umask(0);
		int result_code = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
		umask(process_mask);
		success = !result_code; // TODO Existing folder might throw an error
#endif
		if (success) { isFolder = true; }
		return success;
	}

	bool remove() {
		const char* path = absolute.c_str();
		if (!isFolder) {
			return std::remove(path) == 0;
		}
		return false;
		// TODO Folder deleteion
	}

	bool write(const char* path, const char* data, const size_t length) {
		try {
			auto file = std::fstream(path, std::ios::out | std::ios::binary);
			file.write(data, length);
			file.close();
			return true;
		} catch (...) {
			return false;
		}
	}

	std::string hashFile() {
		const char* path = absolute.c_str();

		if (isFolder) {
			return ""; // Can't has folder
		}
		std::ifstream fp(path);
		std::stringstream ss;
		// Unable to hash file, return an empty hash.
		if (!fp.is_open()) {
			return "";
		}

		// Hashing
		uint32_t magic = 5381;
		char c;
		while (fp.get(c)) {
			magic = ((magic << 5) + magic) + c; // magic * 33 + c
		}

		ss << std::hex << std::setw(8) << std::setfill('0') << magic;
		return ss.str();
	}

	std::string platformPath(std::string path) {
		for (size_t i = 1; i < path.size(); i++) {
			if ((path[i - 1] == '/' || path[i - 1] == '\\') && (path[i] == '/' || path[i] == '\\')) {
				// need to get rid of double slashes
				assert(false);
			}
		}

		for (size_t i = 0; i < path.size(); i++) {
			if (path[i] == '/' || path[i] == '\\') {
				path[i] = PATH_DELIMITER[0];
			}
		}
		return path;
	}

	bool isRelative(const std::string path) {
		return path[0] == '.';
	}
};

} // namespace

#endif // TKLB_STACK