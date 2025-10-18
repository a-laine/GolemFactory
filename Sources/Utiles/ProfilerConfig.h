#pragma once

#define USE_OPTICK

#ifdef USE_OPTICK

#include "optick.h"

#define SCOPED_CPU_MARKER(name) OPTICK_EVENT(name)
#define THREAD_MARKER(name) OPTICK_THREAD(name)
#define FRAME_MARKER(name) OPTICK_FRAME(name)
#define PUSH_CPU_MARKER(name) OPTICK_PUSH(name)
#define POP_CPU_MARKER() OPTICK_POP()

#else

#define SCOPED_CPU_MARKER(name)
#define THREAD_MARKER(name)
#define FRAME_MARKER(name)
#define PUSH_CPU_MARKER(name)
#define POP_CPU_MARKER()

#endif
