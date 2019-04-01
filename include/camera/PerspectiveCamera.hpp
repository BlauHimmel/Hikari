#pragma once

#include <core\Common.hpp>
#include <core\Camera.hpp>

NAMESPACE_BEGIN

/**
* \brief Perspective camera with depth of field
*
* This class implements a simple perspective camera model. It uses an
* infinitesimally small aperture, creating an infinite depth of field.
*/
class PerspectiveCamera : public Camera
{
public:
	virtual ~PerspectiveCamera();

	PerspectiveCamera(const PropertyList & PropList);

	virtual void Activate() override;

	virtual Color3f SampleRay(Ray3f & Ray, const Point2f & SamplePosition, const Point2f & ApertureSample) const override;

	virtual void AddChild(Object * pChildObj, const std::string & Name) override;

	virtual std::string ToString() const override;

protected:
	Vector2f m_InvOutputSize;
	Transform m_SampleToCamera;
	Transform m_CameraToWorld;
	float m_Fov;
	float m_NearClip;
	float m_FarClip;
	Vector3f m_dX;
	Vector3f m_dY;
};

NAMESPACE_END