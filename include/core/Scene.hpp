#pragma once

#include <core\Common.hpp>
#include <core\Object.hpp>

NAMESPACE_BEGIN

class Scene : public Object
{
public:
	/// Construct a new scene object
	Scene(const PropertyList & PropList);

	/// Release all memory
	virtual ~Scene();

	/// Return a pointer to the scene's acceleration structure
	const Acceleration * GetAccel() const;

	/// Return a pointer to the scene's integrator
	const Integrator * GetIntegrator() const;

	/// Return a pointer to the scene's integrator
	Integrator * GetIntegrator();

	/// Return a pointer to the scene's camera
	const Camera * GetCamera() const;

	/// Return a pointer to the scene's sample generator (const version)
	const Sampler * GetSampler() const;

	/// Return a pointer to the scene's sample generator
	Sampler * GetSampler();

	/// Return a reference to an array containing all meshes
	const std::vector<Mesh*> & GetMeshes() const;

	/// Return a reference to an array containing all emitters
	const std::vector<Emitter*> & GetEmitters() const;

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
	virtual void AddChild(Object * pChildObj) override;

	/// Return a string summary of the scene (for debugging purposes)
	virtual std::string ToString() const override;

	virtual EClassType GetClassType() const override;

protected:
	std::vector<Mesh*> m_pMeshes;
	Integrator * m_pIntegrator = nullptr;
	Sampler * m_pSampler = nullptr;
	Camera * m_pCamera = nullptr;
	Acceleration * m_pAcceleration = nullptr;
	std::vector<Emitter*> m_pEmitters;
};

NAMESPACE_END