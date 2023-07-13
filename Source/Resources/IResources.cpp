#include "pch.h"

#include "../Resources/IResources.h"
#include <Utils\Utils.h>
#include <regex>

using namespace Resources;

IResources::IResources(std::string _path, ResourcesType _type)
{
	std::replace(_path.begin(), _path.end(), '\\', '/');
	_path = std::regex_replace(_path, std::regex("//"), "/");
	std::transform(_path.begin(), _path.end(), _path.begin(), [](unsigned char c) { return tolower(c); });
	p_fullPath = _path;

	Utils::ToPath(_path);
	p_path = _path;
	auto pos = p_path.find_last_of('/') + 1;
	if (pos < p_path.size())
		p_name = p_path.substr(pos);
	else
		p_name = p_path;

	pos = p_name.find_last_of(':') + 1;
	if (pos < p_name.size())
		p_name = p_name.substr(pos);
	type = _type;
}

Resources::IResources::~IResources()
{
	p_shouldBeLoaded = false;
	isLoaded = false;
	hasBeenSent = false;
}
