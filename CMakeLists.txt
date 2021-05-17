cmake_minimum_required(VERSION 3.10)
project(tklb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_library(tklb
	src/types/audio/resampler/TResamplerSpeex.cpp
)

TARGET_INCLUDE_DIRECTORIES(tklb
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:src>
)
