#pragma once

#pragma warning(disable:4251)
#ifdef PANDOR_EXPORTS
#define PANDOR_API __declspec(dllexport)
#else
#define PANDOR_API __declspec(dllimport)
#endif

// Enable Multi-thread
#define MULTITHREAD