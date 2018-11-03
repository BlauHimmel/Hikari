#pragma once

#include <core\Common.hpp>
#include <core\Vector.hpp>
#include <core\Color.hpp>

NAMESPACE_BEGIN

/**
* \brief Stores a RGB high dynamic-range bitmap
*
* The bitmap class provides I/O support using the OpenEXR file format
*/
class Bitmap : public Eigen::Array<Color3f, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
{
public:
	using Base = Eigen::Array<Color3f, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

	/**
	* \brief Allocate a new bitmap of the specified size
	*
	* The contents will initially be undefined, so make sure
	* to clear it if necessary.
	*/
	Bitmap(const Vector2i & Size = Vector2i(0, 0));

	/// Load an OpenEXR file with the specified filename
	Bitmap(const std::string & Filename);

	/// Save the bitmap as an EXR file with the specified filename
	void Save(const std::string & Filename);
};

NAMESPACE_END