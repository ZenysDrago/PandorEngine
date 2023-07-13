#pragma once
#include "PandorAPI.h"

namespace Core::Wrapper
{
	namespace WrapperImage
	{
		void FlipVerticalOnLoad(bool flagTrueIfShouldFlip);
		unsigned char* Load(char const* filename, int* x, int* y, int* comp, int req_comp);
		void Save(const char* filename, int x, int y, unsigned char* data);
		void ImageFree(void* free);
		void Resize(int in_x, int in_y, unsigned char* in_data, int out_x, int out_y, unsigned char*& out_data);

	}
}