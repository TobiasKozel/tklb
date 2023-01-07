# TKLB Readme
Cross Platform C++14 library.

## TODOs
- FFT convolution
- Resamplers
- Vector math

## General

### TKLB_IMPL
Define in compilation unit

### TKLB_RELEASE
Will disable asserts reduce logging to warnings and above.

### TKLB_USE_PROFILER
Enables the wrapped tracy profiler

## Assertions
No Exceptions are thrown anywhere in tklb and wrapped assertions are used instead.
### TKLB_NO_ASSERT
Will disable all assertions

### TKLB_ASSERT_BREAK
Will try to signal the attached debugger to break instead of an assert which
can be convenient at times.

### TKLB_NO_STDLIB
Will get rid of all standard library includes if
- `TKLB_NO_SIMD` is also defined to get rid of `xsimd`.
- `TKLB_NO_LOG` is defined, since the logger pulls in `stdarg.h` and `stddef.h`.
- `TKLB_USE_PROFILER` is not defined.
- `TKLB_ASSERT()` is defined to a custom assert function or assertions are disabled with `TKLB_NO_ASSERT` to avoid `<cassert>`.

Also see `TKLB_CUSTOM_MALLOC` and `TKLB_CUSTOM_PRINT`

### TKLB_CUSTOM_MALLOC
Allows defining custom `tklb_free` and `tklb_malloc` without fully dropping the std lib.

## Logger
Logging needs around 2k of static memory and messages will be truncated to 1024 characters.
Offers severity and file + line number for convenience.
```
DEBUG | vae_engine.hpp:236      | Initializing engine...
DEBUG | vae_engine.hpp:245      | Engine initialized
 INFO | vae_bank_manager.hpp:77 | Loading bank from file ./bank1
DEBUG | vae_bank_loader.hpp:56  | Started loading bank bank1
ERROR | vae_bank_loader.hpp:66  | Failed to read file
ERROR | vae_bank_manager.hpp:81 | Failed to load bank from file ./bank1
 INFO | vae_engine.hpp:879      | Start unloading all banks
 INFO | vae_engine.hpp:217      | Engine destructed
```

### TKLB_CUSTOM_PRINT
Allows to redirect the formatted logging messages to a custom implementation of `tklb_print`
### TKLB_CUSTOM_PRINT_FORMAT
TODO allow custom print formatting to disable `stb_sprintf.h`
### TKLB_NO_LOG
Will disable the tklb logger completly.

### TKLB_FORCE_LOG
Will force all log message even when `TKLB_RELEASE` is defined.

### TKLB_NO_SIMD
Disables SSE or other intrinsics via `xsimd` and attempts to do the same for dependecies.

### TKLB_MEMORY_CHECK
Wrap all allocations with magic numbers and validate them (uses std malloc and will define tklb_malloc)
Heapbuffer will check bounds for every access

### TKLB_SAMPLE_FLOAT
Will set the prefered sample type to float.

This means the AudioBuffer implementations will default to it, and external library code will
use floats for processing if possible.
