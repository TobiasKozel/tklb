#ifndef _TKLB_UUID
#define _TKLB_UUID

// TODO tklb maybe rand without stdlib
#include <ctime>
#include <cstdlib>

namespace tklb {
	class uuid {

	public:
		static constexpr int UUIDLength = 36;
		static constexpr int DashPos[] = { 9, 14, 19, 24 };
		static constexpr char CharacterSet[] = "abcdefghijklmnopqrstuvwxyz0123456789";

		/**
		 * @brief Puts a uuid in the provided string
		 */
		static void generate(char (&uuid)[UUIDLength]) {
			std::srand(std::time(nullptr));
			for (int i = 0; i < UUIDLength; i++) {
				uuid[i] = CharacterSet[std::rand() % (sizeof(CharacterSet) - 1)];
			}
			for (auto i : DashPos) { uuid[i] = '-'; }
		}

		/**
		 * @brief Check if the provided string is in the uuid format, needs to be lower case
		 * @param uuid
		 * @param ensureEscaped Whether to also check for the escape character at the end
		 * @return true
		 * @return false
		 */
		static bool isValid(const char* uuid, bool ensureEscaped = true) {
			// First make sure only valid characters are contained
			for (int i = 0; i < UUIDLength; i++) {
				const char c = uuid[i];
				if ((('a' <= c) && (c <= 'z')) || (('0' <= c) && (c <= '9'))) {
					// valid character
				} else if (c == '-') {
					bool correctPosition = false;
					for (int j : DashPos) {
						if (j == i) {
							correctPosition= true;
							break;
						}
					}
					if (!correctPosition) {
						return false;
					}
				} else {
					return false; // invalid character
				}
			}

			// Check the excape character if needed
			if (ensureEscaped && uuid[UUIDLength] != '\0') { return false; }
			return true;
		}

		static bool isValid(const char (&uuid)[UUIDLength]) {
			// skips length check if the length is matched
			return isValid(reinterpret_cast<const char*>(uuid), false);
		}

	};
} // tklb

#endif // _TKLB_UUID
