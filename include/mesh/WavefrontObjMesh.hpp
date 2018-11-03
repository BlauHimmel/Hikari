#pragma once

#include <core\Common.hpp>
#include <core\Mesh.hpp>

NAMESPACE_BEGIN

/**
* \brief Loader for Wavefront OBJ triangle meshes
*/
class WavefrontObjMesh : public Mesh
{
public:
	WavefrontObjMesh(const PropertyList & PropList);

protected:
	struct ObjVertex
	{
		uint32_t P = uint32_t (-1);
		uint32_t N = uint32_t(-1);
		uint32_t UV = uint32_t(-1);

		ObjVertex();

		ObjVertex(const std::string & String);

		bool operator==(const ObjVertex & Rhs) const;
	};

	struct ObjVertexHash 
	{
		size_t operator()(const ObjVertex & Rhs) const;
	};
};

NAMESPACE_END