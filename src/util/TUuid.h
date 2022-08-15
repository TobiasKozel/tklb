#ifndef _TKLB_UUID
#define _TKLB_UUID

// TODO tklb maybe rand without stdlib
#include <ctime>
#include <cstdlib>

namespace tklb { namespace uuid {
	static constexpr int DashPos[] = { 9, 14, 19, 24 };
	static constexpr char CharacterSet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	static constexpr int UUIDLength = 36;

	/**
	 * @brief Puts a uuid in the provided string,
	 * @param uuid string of length UUIDLength or UUIDLength + 1
	 * @param ensureEscaped if true escape character will be written and UUIDLength + 1 is needed!
	 */
	void generate(char* uuid, bool ensureEscaped = true)
	#ifdef TKLB_IMPL
	{
		std::srand(std::time(nullptr));

		for (int i = 0; i < UUIDLength; i++) {
			uuid[i] = CharacterSet[std::rand() % (sizeof(CharacterSet) - 1)];
		}
		for (auto i : DashPos) { uuid[i] = '-'; }

		if (ensureEscaped) {
			uuid[UUIDLength] = '\0';
		}
	}
	#endif // TKLB_IMPL
	;

	/**
	* @brief Check if the provided string is in the uuid format, needs to be lower case
	* @param uuid
	* @param ensureEscaped Whether to also check for the escape character at the end
	*/
	static bool isValid(const char* uuid, bool ensureEscaped = true)
	#ifdef TKLB_IMPL
	{
		auto shouldBeDash = [](int i) {
			for (auto j : DashPos) {
				if (j == i) { return true; }
			}
			return false;
		};
		// First make sure only valid characters are contained
		for (int i = 0; i < UUIDLength; i++) {
			const char c = uuid[i];
			if (shouldBeDash(i)) {
				if (c != '-') {
					return false;
				}
				continue;
			}
			if ((('a' <= c) && (c <= 'z')) || (('0' <= c) && (c <= '9'))) {
				continue;
			} else {
				return false; // invalid character
			}
		}

		// Check the excape character if needed
		if (ensureEscaped && uuid[UUIDLength] != '\0') { return false; }
		return true;
	}
	#endif
	;

	bool isValid(const char (&uuid)[UUIDLength])
	#ifdef TKLB_IMPL
	{
		// skips length check if the length is matched
		return isValid(reinterpret_cast<const char*>(uuid), false);
	}
	#endif // TKLB_IMPL
	;
} } // tklb::uuid

#endif // _TKLB_UUID
