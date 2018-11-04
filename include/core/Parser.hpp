#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

/**
* \brief Load a scene from the specified filename and
* return its root object
*/
Object * LoadFromXML(const std::string & Filename);

NAMESPACE_END