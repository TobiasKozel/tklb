#ifndef TKLBZ_UUID
#define TKLBZ_UUID

// TODO tklb maybe rand without stdlib
#include <ctime>
#include <cstdlib>

namespace tklb { namespace uuid {
	constexpr int UUIDLength = 36;
	constexpr int DashPos[] = { 9, 14, 19, 24 };
	constexpr char CharacterSet[] = "abcdefghijklmnopqrstuvwxyz0123456789";

	void generate(char (&uuid)[UUIDLength]) {
		std::srand(std::time(nullptr));
		for (int i = 0; i < UUIDLength; i++) {
			uuid[i] = CharacterSet[std::rand() % (sizeof(CharacterSet) - 1)];
		}
		for (int i : DashPos) { uuid[i] = '-'; }
	}

	bool isValid(const char* uuid, bool checkLength = true) {
		if (checkLength && uuid[UUIDLength] != '\0') { return false; } // too long
		for (int i : DashPos) { if (uuid[i] != '-') { return false; } }

		for (int i = 0; i < UUIDLength; i++) {
			const char c = uuid[i];
			if ((('a' <= c) && (c <= 'z')) || (('0' <= c) && (c <= '9')) || (c == '-')) {
				// valid character
			} else { return false; } // invalid character
		}
		return true;
	}

	bool isValid(const char (&uuid)[UUIDLength]) {
		// skips length check if the length is matched
		return isValid(reinterpret_cast<const char*>(uuid), false);
	}

} } // tklb::uuid

#endif // TKLBZ_UUID
