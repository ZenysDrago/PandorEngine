#pragma once

// commment if glm shouldn't be included
//#define GLM

// commment if Physx shouldn't be included
//#define PHYSX

#ifdef GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#endif // GLM

#ifdef PHYSX
#include <PhysX/PxPhysicsAPI.h>
#endif


#include <cmath>
#include <iostream>
#include <algorithm>

#define PI 3.14159265359f
#define DEG2RAD PI/180.f
#define RAD2DEG 180.f/PI

namespace Arithmetics
{
	//return the val time itself
	inline float pow(const float& val)
	{
		return val * val;
	}

	// Return the value powered to the value of power
	inline float powI(const float& val, const float power)
	{
		if (power == 0)
			return 1;

		float tmpVal = val;
		for (int i = 1; i < power; i++)
			tmpVal *= val;

		return tmpVal;
	}

	template<typename T>
	inline T Lerp(const T& min, const T& max, float value) {
		value = std::clamp(value, 0.f, 1.f);
		return min * (1 - value) + max * value;
	}

	template<typename T>
	inline T Normalize(T value, T min_value, T max_value) {
		return (value - min_value) / (max_value - min_value);
	}

	template <typename T>
	const T& Clamp(const T& value, const T& min, const T& max) {
		return value < min ? min : (value > max ? max : value);
	}

	template <typename T>
	int Sign(const T& value) {
		return (T(0) < value) - (value < T(0));
	}
}