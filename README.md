# TKLB Readme

## TODOs
- FFT convolution

## General


## TKLB_IMPL
Define for header only experience

TODO or use compile units

### TKLB_NO_ASSERT
Will disable all assertions

### TKLB_CUSTOM_MALLOC
Allows defining custom tklb_free and tklb_malloc

### TKLB_RELEASE
Will disable asserts reduce logging and some other optimizations

### TKLB_NO_STDLIB
Will get rid of all(most) standard lib references.
Own memory function and printing function will have to be defined.

### TKLB_NO_SIMD
Disables SSE or other intrinsics

### TKLB_MEMORY_CHECK
Wrap all allocations with magic numbers and validate them (uses std malloc and will define tklb_malloc)

### TKLB_USE_PROFILER
Includes tracy to do remote profiling

### TKLB_MAXCHANNELS
Maximum channels for audio related classes. Defaults to 2
Only relevant for the oversampler at the moment.

### TKLB_SAMPLE_FLOAT
Will do all audio related stuff with floats

### TKLB_CONVOLUTION_FLOAT
Will do float convolution which allows sse optimizations regardless of TKLB_SAMPLE_FLOAT
