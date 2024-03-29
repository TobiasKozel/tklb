/**
 * @file TProfiler.hpp
 * @author Tobias Kozel
 * @brief Wrapper for the Tracy cpp profiler
 * @version 0.1
 * @date 2023-08-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_PROFILER
#define _TKLB_PROFILER

#ifndef TKLB_USE_PROFILER

	#define TKLB_PROFILER_FRAME_MARK()				///< Marks a frame to calculate FPS, not really needed for audio
	#define TKLB_PROFILER_FRAME_MARK_START(name)	///< Starts a named frame, uses const defined above to maintain the same pointer
	#define TKLB_PROFILER_FRAME_MARK_END(name)		///< Stops a named frame
	#define TKLB_PROFILER_SCOPE()					///< Profiles a scope
	#define TKLB_PROFILER_SCOPE_NAMED(name)			///< Profiles a scope and names it
	#define TKLB_PROFILER_PLOT(name, value)			///< Records a value
	#define TKLB_PROFILER_THREAD_NAME(name)			///< Sets name for current thread
	#define TKLB_PROFILER_MESSAGE_L(msg)			///< Send literal message to profiler
	#define TKLB_PROFILER_MESSAGE(msg, size)		///< Send dynamic string message
	#define TKLB_PROFILER_MALLOC(ptr, size)			///< Track allocation
	#define TKLB_PROFILER_FREE(ptr)					///< Track free
	#define TKLB_PROFILER_MUTEX(type, name, desc) type name;
	#define TKLB_PROFILER_FREE_L(ptr, name)			///< Track named allocaions
	#define TKLB_PROFILER_MALLOC_L(ptr, size, name)	///< Track named allocaions

#else // TKLB_USE_PROFILER

	#define TRACY_ENABLE
	#include "../../external/tracy/public/tracy/Tracy.hpp"

	#ifdef TKLB_IMPL
		#include "../../external/tracy/public/TracyClient.cpp"
	#endif

	#define TKLB_PROFILER_FRAME_MARK()				FrameMark
	#define TKLB_PROFILER_FRAME_MARK_START(name)	FrameMarkStart(name)
	#define TKLB_PROFILER_FRAME_MARK_END(name)		FrameMarkEnd(name)
	#define TKLB_PROFILER_SCOPE()					ZoneScoped
	#define TKLB_PROFILER_SCOPE_NAMED(name)			ZoneScopedN(name)
	#define TKLB_PROFILER_PLOT(name, value)			TracyPlot(name, value)
	#define TKLB_PROFILER_THREAD_NAME(name)			tracy::SetThreadName(name);
	#define TKLB_PROFILER_MESSAGE_L(msg)			TracyMessageL(msg)
	#define TKLB_PROFILER_MESSAGE(msg, size)		TracyMessage(msg, size)
	#define TKLB_PROFILER_MALLOC(ptr, size)			TracyAlloc(ptr, size);
	#define TKLB_PROFILER_FREE(ptr)					TracyFree(ptr);
	#define TKLB_PROFILER_MALLOC_L(ptr, size, name)	TracyAllocN(ptr, size, name);
	#define TKLB_PROFILER_FREE_L(ptr, name)			TracyFreeN(ptr, name);
	#define TKLB_PROFILER_MUTEX(type, name, desc)	TracyLockableN(type, name, desc)

	// TODO this seems shady and it's unused
	#define TKLB_TRACK_ALLOCATE(ptr, size)		TKLB_PROFILER_MALLOC_L(ptr, size, vae::core::profiler::tklbAllocator)
	#define TKLB_TRACK_FREE(ptr, size)			TKLB_PROFILER_FREE_L(ptr, vae::core::profiler::tklbAllocator)

#endif // TKLB_USE_PROFILER

#endif // _TKLB_PROFILER
