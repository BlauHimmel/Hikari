#include <camera\PerspectiveCamera.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(PerspectiveCamera, XML_CAMERA_PERSPECTIVE);

PerspectiveCamera::~PerspectiveCamera()
{
	delete m_pFilter;
}

PerspectiveCamera::PerspectiveCamera(const PropertyList & PropList)
{
	/* Width and height in pixels. Default: 720p */
	m_OutputSize.x() = PropList.GetInteger(XML_CAMERA_PERSPECTIVE_WIDTH, 1280);
	m_OutputSize.y() = PropList.GetInteger(XML_CAMERA_PERSPECTIVE_HEIGHT, 720);
	m_InvOutputSize = m_OutputSize.cast<float>().cwiseInverse();

	/* Specifies an optional camera-to-world transformation. Default: none */
	m_CameraToWorld = PropList.GetTransform(XML_CAMERA_PERSPECTIVE_TO_WORLD, Transform());

	/* Horizontal field of view in degrees */
	m_Fov = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_FOV, 30.0f);

	/* Near and far clipping planes in world-space units */
	m_NearClip = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_NEAR_CLIP, 1e-4f);
	m_FarClip = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_FAR_CLIP, 1e4f);

	m_pFilter = nullptr;
}

void PerspectiveCamera::Activate()
{
	float Aspect = float(m_OutputSize.x()) / float(m_OutputSize.y());

	/* Project vectors in camera space onto a plane at z=1:
	*
	*  xProj = cot * x / z
	*  yProj = cot * y / z
	*  zProj = (far * (z - near)) / (z * (far-near))
	*  The cotangent factor ensures that the field of view is
	*  mapped to the interval [-1, 1].
	*/
	float Recip = 1.0f / (m_FarClip - m_NearClip);
	float Cot = 1.0f / std::tan(DegToRad(m_Fov / 2.0f));

	Eigen::Matrix4f Perspective;
	Perspective <<
		Cot, 0,   0,                 0,
		0,   Cot, 0,                 0,
		0,   0,   m_FarClip * Recip, -m_NearClip * m_FarClip * Recip,
		0,   0,   1,                 0;

	/**
	* Translation and scaling to shift the clip coordinates into the
	* range from zero to one. Also takes the aspect ratio into account.
	*/
	m_SampleToCamera = Transform(
		Eigen::DiagonalMatrix<float, 3>(Vector3f(-0.5f, -0.5f * Aspect, 1.0f)) *
		Eigen::Translation<float, 3>(-1.0f, -1.0f / Aspect, 0.0f) * 
		Perspective
	).Inverse();

	/* If no reconstruction filter was assigned, instantiate a Gaussian filter */
	if (m_pFilter == nullptr)
	{
		m_pFilter = (ReconstructionFilter*)(ObjectFactory::CreateInstance(XML_FILTER_GAUSSION, PropertyList()));
	}
}

Color3f PerspectiveCamera::SampleRay(Ray3f & Ray, const Point2f & SamplePosition, const Point2f & ApertureSample) const
{
	/* Compute the corresponding position on the near plane (in local camera space) */
	Point3f NearP = m_SampleToCamera * Point3f(
		SamplePosition.x() * m_InvOutputSize.x(),
		SamplePosition.y() * m_InvOutputSize.y(), 
		0.0f
	);

	/* Turn into a normalized ray direction, and adjust the ray interval accordingly */
	Vector3f Dir = NearP.normalized();
	float InvZ = 1.0f / Dir.z();

	Ray.Origin = m_CameraToWorld * Point3f(0.0f);
	Ray.Direction = m_CameraToWorld * Dir;
	Ray.MinT = m_NearClip * InvZ;
	Ray.MaxT = m_FarClip * InvZ;
	Ray.Update();

	return Color3f(1.0f);
}

void PerspectiveCamera::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EReconstructionFilter:
		if (m_pFilter != nullptr)
		{
			throw HikariException("Camera: tried to register multiple reconstruction filters!");
		}
		m_pFilter = (ReconstructionFilter*)(pChildObj);
		break;
	default:
		throw HikariException("Camera::AddChild(<%s>) is not supported!", ClassTypeName(pChildObj->GetClassType()));
	}
}

std::string PerspectiveCamera::ToString() const
{
	return tfm::format(
		"PerspectiveCamera[\n"
		"  cameraToWorld = %s,\n"
		"  outputSize = %s,\n"
		"  fov = %f,\n"
		"  clip = [%f, %f],\n"
		"  filter = %s\n"
		"]",
		Indent(m_CameraToWorld.ToString(), 18),
		m_OutputSize.ToString(),
		m_Fov,
		m_NearClip,
		m_FarClip,
		Indent(m_pFilter->ToString())
	);
}

NAMESPACE_END