#include "../src/types/TStackBuffer.hpp"

template <class T>
using HeapBufferType = tklb::StackBuffer<T, 128>;

#include "./TestBufferCommon.hpp"
