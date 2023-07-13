// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H
// add headers that you want to pre-compile here
#include "Debug/Log.h"
#include "Math/Maths.h"

#include <Core/Wrappers/WrapperWindow.h>
#include <Core/Wrappers/WrapperUI.h>
#include <Core/Wrappers/WrapperImage.h>
using namespace Core::Wrapper;

#define NOMINMAX
#include <Windows.h>
#include "framework.h"
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>
#include <fstream>
#include <functional>
#include <map>
#include <unordered_map>
#include <typeinfo>

#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>

#endif //PCH_H

