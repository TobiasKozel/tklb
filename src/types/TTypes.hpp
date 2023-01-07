#ifndef _TKLB_TYPES
#define _TKLB_TYPES

#ifndef TKLB_NO_STDLIB
#include <stddef.h>
namespace tklb {
	using SizeT = size_t;
	using Pointer = size_t;
}

#else
namespace tklb {
	using SizeT = unsigned long;
	using Pointer = SizeT;
} // tklb
#endif

static_assert(
	sizeof(tklb::Pointer) == sizeof(void*),
	"tklb::Pointer width is not equal to sizeof(void*)"
);

#endif // _TKLB_TYPES
