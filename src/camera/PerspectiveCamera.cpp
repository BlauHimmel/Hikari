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
	m_OutputSize.x() = PropList.GetInteger(XML_CAMERA_PERSPECTIVE_WIDTH, DEFAULT_CAMERA_OUTPUTSIZE_X);
	m_OutputSize.y() = PropList.GetInteger(XML_CAMERA_PERSPECTIVE_HEIGHT, DEFAULT_CAMERA_OUTPUTSIZE_Y);
	m_InvOutputSize = m_OutputSize.cast<float>().cwiseInverse();

	/* Specifies an optional camera-to-world transformation. Default: none */
	m_CameraToWorld = PropList.GetTransform(XML_CAMERA_PERSPECTIVE_TO_WORLD, DEFAULT_CAMERA_CAMERA_TO_WORLD);

	/* Horizontal field of view in degrees */
	m_Fov = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_FOV, DEFAULT_CAMERA_FOV);

	/* Near and far clipping planes in world-space units */
	m_NearClip = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_NEAR_CLIP, DEFAULT_CAMERA_NEAR_CLIP);
	m_FarClip = PropList.GetFloat(XML_CAMERA_PERSPECTIVE_FAR_CLIP, DEFAULT_CAMERA_FAR_CLIP);

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

	m_dX = m_SampleToCamera * Point3f(m_InvOutputSize.x(), 0.0f, 0.0f) -
		m_SampleToCamera * Point3f(0.0f);
	m_dY = m_SampleToCamera * Point3f(0.0f, m_InvOutputSize.y(), 0.0f) -
		m_SampleToCamera * Point3f(0.0f);

	/* If no reconstruction filter was assigned, instantiate a default filter */
	if (m_pFilter == nullptr)
	{
		LOG(WARNING) << "No reconstruction filter was specified, create a default reconstruction filter.";
		m_pFilter = (ReconstructionFilter*)(ObjectFactory::CreateInstance(DEFAULT_CAMERA_RFILTER, PropertyList()));
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
	Ray.RxOrigin = Ray.Origin;
	Ray.RyOrigin = Ray.Origin;
	Ray.RxDirection = m_CameraToWorld * Vector3f((Vector3f(NearP) + m_dX).normalized());
	Ray.RyDirection = m_CameraToWorld * Vector3f((Vector3f(NearP) + m_dY).normalized());
	Ray.bHasDifferentials = true;
	Ray.Update();

	return Color3f(1.0f);
}

void PerspectiveCamera::AddChild(Object * pChildObj, const std::string & Name)
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
		throw HikariException("Camera::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
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