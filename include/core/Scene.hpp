#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>
#include <core\BoundingBox.hpp>

NAMESPACE_BEGIN

class Scene : public Object
{
public:
	/// Construct a new scene object
	Scene(const PropertyList & PropList);

	/// Release all memory
	virtual ~Scene();

	/// Return the color of the background;
	Color3f GetBackground() const;

	/// Return whether the specified background color is forced to used
	bool GetForceBackground() const;

	/// Return a pointer to the scene's acceleration structure
	const Acceleration * GetAccel() const;

	/// Return a pointer to the scene's acceleration structure
	Acceleration * GetAccel();

	/// Return a pointer to the scene's integrator
	const Integrator * GetIntegrator() const;

	/// Return a pointer to the scene's integrator
	Integrator * GetIntegrator();

	/// Return a pointer to the scene's camera
	const Camera * GetCamera() const;

	/// Return a pointer to the scene's camera
	Camera * GetCamera();

	/// Return a pointer to the scene's sample generator (const version)
	const Sampler * GetSampler() const;

	/// Return a pointer to the scene's sample generator
	Sampler * GetSampler();

	/// Return a reference to an array containing all meshes
	const std::vector<Mesh*> & GetMeshes() const;

	/// Return a reference to an array containing all emitters
	const std::vector<Emitter*> & GetEmitters() const;

	/// Return a pointer to the scene's environment emitter
	const Emitter * GetEnvironmentEmitter() const;

	/// Return a pointer to the scene's environment emitter
	Emitter * GetEnvironmentEmitter();

	/// Return a axis-aligned box that bounds the scene
	BoundingBox3f GetBoundingBox() const;

	/**
	* \brief Intersect a ray against all triangles stored in the scene
	* and return detailed intersection information
	*
	* \param Ray
	*    A 3-dimensional ray data structure with minimum/maximum
	*    extent information
	*
	* \param Isect
	*    A detailed intersection record, which will be filled by the
	*    intersection query
	*
	* \return \c true if an intersection was found
	*/
	bool RayIntersect(const Ray3f & Ray, Intersection & Isect) const;

	/**
	* \brief Intersect a ray against all triangles stored in the scene
	*
	* \param Ray
	*    A 3-dimensional ray data structure with minimum/maximum
	*    extent information
	*
	* \return \c true if an intersection was found
	*/
	bool ShadowRayIntersect(const Ray3f & Ray) const;

	/**
	* \brief Inherited from \ref Object::Activate()
	*
	* Initializes the internal data structures (kd-tree,
	* bvh, emitter sampling data structures, etc.)
	*/
	virtual void Activate() override;

	/// Add a child object to the scene (meshes, integrators etc.)
	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	/// Return a string summary of the scene (for debugging purposes)
	virtual std::string ToString() const override;

	virtual EClassType GetClassType() const override;

protected:
	Color3f m_Background;
	bool m_bForceBackground;

	std::vector<Mesh *> m_pMeshes;
	Integrator * m_pIntegrator = nullptr;
	Sampler * m_pSampler = nullptr;
	Camera * m_pCamera = nullptr;
	Acceleration * m_pAcceleration = nullptr;
	std::vector<Emitter*> m_pEmitters;
	Emitter * m_pEnvironmentEmitter = nullptr;
	BoundingBox3f m_BBox;
};

NAMESPACE_END