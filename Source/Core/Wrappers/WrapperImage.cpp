#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION
#include <STB_Image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include <STB_Image/stb_image_write.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <STB_Image/stb_image_resize.h>

using namespace Core::Wrapper;

void WrapperImage::FlipVerticalOnLoad(bool flagTrueIfShouldFlip)
{
	stbi_set_flip_vertically_on_load(flagTrueIfShouldFlip);
}

unsigned char *WrapperImage::Load(char const* filename, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load(filename, x, y, comp, req_comp);
}

void Core::Wrapper::WrapperImage::ImageFree(void* free)
{
	stbi_image_free(free);
}

void Core::Wrapper::WrapperImage::Save(const char* filename, int x, int y, unsigned char* data)
{
	stbi_write_png(filename, x, y, 4, data, x * 4);
}

void Core::Wrapper::WrapperImage::Resize(int in_x, int in_y, unsigned char* in_data, int out_x, int out_y, unsigned char*& out_data)
{
	stbir_resize_uint8(in_data, in_x, in_y, in_x * 4, out_data, out_x, out_y, out_x * 4, 4);
}
