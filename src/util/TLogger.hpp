#ifndef _TKLB_LOGGER
#define _TKLB_LOGGER

/**
 * @file TLogger.hpp
 * @author Tobias Kozel
 * @brief Custom logger around stb_sprintf
 * @details
 * Defining TKLB_NO_STDLIB or TKLB_CUSTOM_PRINT allows using custom print outlet
 * Defining TKLB_RELEASE will only log warnings and above
 * Defining TKLB_FORCE_LOG will override this
 * TODO mention profiler
 *
 * @version 0.1
 * @date 2022-08-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifdef TKLB_NO_LOG
	#define TKLB_DEBUG(...)
	#define TKLB_INFO(...)
	#define TKLB_WARN(...)
	#define TKLB_ERROR(...)
	#define TKLB_CRITICAL(...)
#else // TKLB_NO_LOG
	#include <stdarg.h>	// Variable argument wrangling

	#ifdef TKLB_IMPL
		#define STB_SPRINTF_IMPLEMENTATION
	#endif
	#include "../../external/stb_sprintf.h"

	/**
	 * @brief main logging function. This can be overridden by defining TKLB_CUSTOM_PRINT
	 *        or TKLB_NO_STDLIB and implementing it somewhere else
	 * @param level 0 debug, info, warn, error, critcal
	 * @param message already formatted message
	 */
	void tklb_print(int level, const char* message);
	#ifdef TKLB_IMPL
		#if !defined(TKLB_NO_STDLIB) && !defined(TKLB_CUSTOM_PRINT)
			#include <stdio.h>
			void tklb_print(int level, const char* message) {
				printf("%s\n", message);
			}
		#endif
	#endif // TKLB_IMPL

	/**
	 * @brief Pretty formats the message with level, file etc.
	 *
	 * @param level Log level
	 * @param path Full file path
	 * @param line Line number
	 * @param format Format string
	 * @param ... format arguments
	 */
	void tklb_print_path(int level, const char* path, int line, const char* format, ...);
	#ifdef TKLB_IMPL
		void tklb_print_path(int level, const char* path, int line, const char* format, ...) {
			if (path == nullptr) { return; }
			if (format == nullptr) { return; }

			constexpr int bufferSize = 1024;	// Should be plenty for most purposes, the formating respects this limit.
			static bool locked = false;			// TODO TKLB properly protect multithread access
			static char buffer[bufferSize];
			static char buffer2[bufferSize];	// Need to buffers, could use a little optimization

			{
				// Strip away the path and only keep the file name
				int lastDelimiter = 0;

				for (int i = 0; i < bufferSize; i++) {
					if (path[i] == '\0') {
						break;
					}
					if (path[i] == '\\' || path[i] == '/') {
						lastDelimiter = i;
					}
				}
				// move the beginning of the string forward so shorten it
				path += lastDelimiter + 1;
			}

			while(locked) { }
			locked = true;

			switch (level) {
				case 0:	stbsp_snprintf(buffer, bufferSize, "DEBUG | %s:%i \t| %s", path, line, format); break;
				case 1:	stbsp_snprintf(buffer, bufferSize, " INFO | %s:%i \t| %s", path, line, format); break;
				case 2:	stbsp_snprintf(buffer, bufferSize, " WARN | %s:%i \t| %s", path, line, format); break;
				case 3:	stbsp_snprintf(buffer, bufferSize, "ERROR | %s:%i \t| %s", path, line, format); break;
				case 4:	stbsp_snprintf(buffer, bufferSize, " CRIT | %s:%i \t| %s", path, line, format); break;
				default: break;
			}

			va_list va;
			va_start(va, format);
			const int length = stbsp_vsnprintf(buffer2, bufferSize, buffer, va);
			va_end(va);
			buffer2[bufferSize - 1] = '\0'; // Force buffer termination eventhough stb alredy promised that
			tklb_print(level, buffer2);

			#ifdef TKLB_PROFILER_MESSAGE
				TKLB_PROFILER_MESSAGE(buffer2, length)
			#else
				(void)(length);
			#endif
			locked = false;
		}
	#endif // TKLB_IMPL

	// Always print errors and warnings
	#define TKLB_CRITICAL(msg, ...)	tklb_print_path(4, __FILE__, __LINE__, msg, ## __VA_ARGS__);
	#define TKLB_ERROR(msg, ...)	tklb_print_path(3, __FILE__, __LINE__, msg, ## __VA_ARGS__);
	#define TKLB_WARN(msg, ...)		tklb_print_path(2, __FILE__, __LINE__, msg, ## __VA_ARGS__);

	// Only log these if explicitly requested or in debug mode.
	#if !defined(TKLB_RELEASE) || defined(TKLB_FORCE_LOG)
		#define TKLB_INFO(msg, ...)  tklb_print_path(1, __FILE__, __LINE__, msg, ## __VA_ARGS__);
		#define TKLB_DEBUG(msg, ...) tklb_print_path(0, __FILE__, __LINE__, msg, ## __VA_ARGS__);
	#else
		#define TKLB_INFO(...)
		#define TKLB_DEBUG(...)
	#endif
#endif // TKLB_NO_LOG

#endif // _TKLB_LOGGER
