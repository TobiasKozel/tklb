#ifndef TKLB_UUID
#define TKLB_UUID

#include <string>

namespace tklb {
	std::string generateUUID() {
		srand(time(nullptr));
		const int charLength = 10 + 26;
		const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
		const int UUIDLength = 36;
		char out[UUIDLength];
		for (int i = 0; i < UUIDLength; i++) {
			out[i] = chars[rand() % charLength];
		}
		out[9] = out[14] = out[19] = out[24] = '-';
		std::string ret;
		ret.append(out, UUIDLength);
		return ret;
	}

	bool isUUID(const std::string id) {
		if (id.size() != 36) { return false; }
		if ((id[9] & id[14] & id[19] & id[24]) != '-') { return false; }
		if ((id[9] | id[14] | id[19] | id[24]) != '-') { return false; }
		for (size_t i = 0; i < id.size(); i++) {
			const char c = id[i];
			if ((('a' <= c) && (c <= 'z')) || (('0' <= c) && (c <= '9')) || (c == '-')) {}
				else { return false; }
			}
		return true;
	}
}

#endif