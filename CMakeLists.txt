cmake_minimum_required(VERSION 3.10)
project(tklb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(tklb_SOURCES
	src/types/audio/resampler/TResamplerSpeex.cpp
	src/types/audio/TWaveFile.cpp
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # needed for clang

if (TKLB_BUILD_STATIC_LIBS)
	add_library(tklb STATIC ${tklb_SOURCES})
else()
	add_library(tklb SHARED ${tklb_SOURCES})
endif()

TARGET_INCLUDE_DIRECTORIES(tklb
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:src>
)
