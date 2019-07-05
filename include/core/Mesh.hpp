#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\Shape.hpp>
#include <core\Frame.hpp>
#include <core\DiscretePDF.hpp>
#include <core\BoundingBox.hpp>
#include <core\Emitter.hpp>
#include <core\BSDF.hpp>

NAMESPACE_BEGIN

/**
* \brief Triangle shape used ONLY in mesh
*/
class Triangle : public Shape
{
public:
	Triangle();

	Triangle(Mesh * pMesh, uint32_t * pFacet, uint32_t iFacet);

	/**
	* \brief Uniformly sample a position on the mesh with
	* respect to surface area. Returns both position and normal
	* Note : Normally, we use Mesh::SamplePosition() instead of
	* this function.
	*/
	virtual void SamplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const override;

	/// Return the surface area of the given triangle
	virtual float SurfaceArea() const override;

	/// Return an axis-aligned bounding box of the entire mesh
	virtual BoundingBox3f GetBoundingBox() const override;

	/// Return the centroid of the given triangle
	virtual Point3f GetCentroid() const override;

	/** \brief intersection test
	*
	* \param Ray
	*    The ray segment to be used for the intersection query
	* \param U
	*   Upon success, \c U will contain the 'U' component of the intersection
	*   in barycentric coordinates
	* \param V
	*   Upon success, \c V will contain the 'V' component of the intersection
	*   in barycentric coordinates
	* \param T
	*    Upon success, \a T contains the distance from the ray origin to the
	*    intersection point,
	* \return
	*   \c true if an intersection has been detected
	*/
	virtual bool RayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const override;

	/// After intersection test passed, compute the detail information of the intersection point.
	virtual void PostIntersect(Intersection & Isect) const override;

	/**
	* \brief Return the pointer of the mesh that this shape attach
	* \return
	*   \c nullptr if the shape does not attach to any mesh
	*/
	virtual Mesh * GetMesh() const override;

	/**
	* \brief Return the index of facet in the mesh that this shape attach
	* \return
	*   \c uint32_t(-1) if the shape does not attach to any mesh
	*/
	virtual uint32_t GetFacetIndex() const override;

	/// Is this mesh an area emitter?
	virtual bool IsEmitter() const override;

	/// Return a pointer to an attached area emitter instance
	virtual Emitter * GetEmitter() override;

	/// Return a pointer to an attached area emitter instance (const version)
	virtual const Emitter * GetEmitter() const override;

	/// Return a pointer to the BSDF associated with this mesh
	virtual const BSDF * GetBSDF() const override;

	virtual std::string ToString() const override;

	Mesh * m_pMesh = nullptr;
	uint32_t * m_pFacet = nullptr;
	uint32_t m_iFacet = 0;
};

/**
* \brief Triangle mesh
*
* This class stores a triangle mesh object and provides numerous functions
* for querying the individual triangles. Subclasses of \c Mesh implement
* the specifics of how to create its contents (e.g. by loading from an
* external file)
*/

class Mesh : public Object
{
public:
	/// Release all memory
	virtual ~Mesh();

	/// Initialize internal data structures (called once by the XML parser)
	virtual void Activate() override;

	/// Return the total number of triangles in this hsape
	uint32_t GetTriangleCount() const;

	/// Return the total number of vertices in this hsape
	uint32_t GetVertexCount() const;

	/**
	* \brief Uniformly sample a position on the mesh with
	* respect to surface area. Returns both position and normal
	*/
	void SamplePosition(float Sample1D, const Point2f & Sample2D, Point3f & P, Normal3f & N) const;

	///  Compute the probability of sampling point on the mesh
	float Pdf() const;

	/// Return the surface area of the mesh
	float SurfaceArea() const;

	/// Return the surface area of the given triangle
	float SurfaceArea(uint32_t Index) const;

	/// Return an axis-aligned bounding box of the entire mesh
	const BoundingBox3f & GetBoundingBox() const;

	/// Return an axis-aligned bounding box containing the given triangle
	BoundingBox3f GetBoundingBox(uint32_t Index) const;

	/// Return the centroid of the given triangle
	Point3f GetCentroid(uint32_t Index) const;

	/** \brief Ray-triangle intersection test
	*
	* Uses the algorithm by Moeller and Trumbore discussed at
	* <tt>http://www.acm.org/jgt/papers/MollerTrumbore97/code.html</tt>.
	*
	* Note that the test only applies to a single triangle in the mesh.
	* An acceleration data structure like BVH or KDTree is needed to search
	* for intersections against many triangles.
	*
	* \param Index
	*    Index of the triangle that should be intersected
	* \param Ray
	*    The ray segment to be used for the intersection query
	* \param U
	*   Upon success, \c U will contain the 'U' component of the intersection
	*   in barycentric coordinates
	* \param V
	*   Upon success, \c V will contain the 'V' component of the intersection
	*   in barycentric coordinates
	* \param T
	*    Upon success, \a T contains the distance from the ray origin to the
	*    intersection point,
	* \return
	*   \c true if an intersection has been detected
	*/
	bool RayIntersect(uint32_t Index, const Ray3f & Ray, float & U, float & V, float & T) const;

	/// Return a pointer to the vertex positions
	const MatrixXf & GetVertexPositions() const;

	/// Return a pointer to the vertex normals (or \c nullptr if there are none)
	const MatrixXf & GetVertexNormals() const;

	/// Return a pointer to the texture coordinates (or \c nullptr if there are none)
	const MatrixXf & GetVertexTexCoords() const;

	/// Return a pointer to the triangle vertex index list
	const MatrixXu & GetIndices() const;

	/// Is this mesh an area emitter?
	bool IsEmitter() const;

	/// Return a pointer to an attached area emitter instance
	Emitter * GetEmitter();

	/// Return a pointer to an attached area emitter instance (const version)
	const Emitter * GetEmitter() const;

	/// Return a pointer to the BSDF associated with this mesh
	const BSDF * GetBSDF() const;

	/// Return the name of this mesh
	const std::string & GetName() const;

	/// Register a child object (e.g. a BSDF) with the mesh
	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	/// Return a human-readable summary of this instance
	virtual std::string ToString() const override;

	/**
	* \brief Return the type of object (i.e. Mesh/BSDF/etc.)
	* provided by this instance
	* */
	virtual EClassType GetClassType() const;

protected:
	/// Create an empty mesh
	Mesh();

protected:
	std::string m_Name;                            ///< Identifying name
	MatrixXf m_V;                                  ///< Vertex positions
	MatrixXf m_N;                                  ///< Vertex normals
	MatrixXf m_UV;                                 ///< Vertex texture coordinates
	MatrixXu m_F;                                  ///< Faces
	BSDF * m_pBSDF = nullptr;                      ///< BSDF of the surface
	Emitter * m_pEmitter = nullptr;                ///< Associated emitter, if any
	BoundingBox3f m_BBox;                          ///< Bounding box of the mesh
	std::unique_ptr<DiscretePDF1D> m_pPDF;         ///< Used for sampling triangle of the mesh weighted by its area
	float m_MeshArea = 0.0f;                       ///< Total surface area of the mesh
	float m_InvMeshArea = 0.0f;                    ///< Probability of a sampling point on the mesh
};

NAMESPACE_END