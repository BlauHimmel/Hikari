#include <core\StudentTTest.hpp>
#include <core\Scene.hpp>
#include <core\Integrator.hpp>
#include <core\Camera.hpp>
#include <core\Sampler.hpp>
#include <core\BSDF.hpp>
#include <hypothesis.h>
#include <pcg32.h>

NAMESPACE_BEGIN

REGISTER_CLASS(StudentTTest, XML_TEST_STUDENT_T);

StudentTTest::StudentTTest(const PropertyList & PropList)
{
	/* The null hypothesis will be rejected when the associated
	p-value is below the significance level specified here. */
	m_SignificanceLevel = PropList.GetFloat(XML_TEST_STUDENT_T_SIGNIFICANCE_LEVEL, DEFAULT_TEST_STUDENT_T_SIGNIFICANCE_LEVEL);

	/* This parameter specifies a list of incidence angles that will be tested */
	std::vector<std::string> Angles = Tokenize(PropList.GetString(XML_TEST_STUDENT_T_ANGLES, DEFAULT_TEST_STUDENT_T_ANGLES));
	for (std::string Angle : Angles)
	{
		m_Angles.push_back(ToFloat(Angle));
	}

	/* This parameter specifies a list of reference values, one for each angle */
	std::vector<std::string> References = Tokenize(PropList.GetString(XML_TEST_STUDENT_T_REFERENCES, DEFAULT_TEST_STUDENT_T_REFERENCES));
	for (std::string Angle : References)
	{
		m_References.push_back(ToFloat(Angle));
	}

	/* Number of BSDF samples that should be generated (default: 100K) */
	m_SampleCount = PropList.GetInteger(XML_TEST_STUDENT_T_SAMPLE_COUNT, DEFAULT_TEST_STUDENT_T_SAMPLE_COUNT);
}

StudentTTest::~StudentTTest()
{
	for (BSDF * pBSDF : m_pBSDFs)
	{
		delete pBSDF;
	}
	for (Scene * pScene : m_pScenes)
	{
		delete pScene;
	}
}

void StudentTTest::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EBSDF:
		m_pBSDFs.push_back((BSDF*)(pChildObj));
		break;
	case EClassType::EScene:
		m_pScenes.push_back((Scene*)(pChildObj));
		break;
	default:
		throw HikariException(
			"StudentTTest::AddChild(<%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType())
		);
		break;
	}
}

void StudentTTest::Activate()
{
	int Total = 0, Passed = 0;
	pcg32 Random;

	if (!m_pBSDFs.empty())
	{
		if (m_References.size() * m_pBSDFs.size() != m_Angles.size())
		{
			throw HikariException("Specified a different number of angles and reference values!");
		}
		if (!m_pScenes.empty())
		{
			throw HikariException("Cannot test BSDFs and scenes at the same time!");
		}

		/* Test each registered BSDF */
		int Idx = 0;
		for (BSDF * pBSDF : m_pBSDFs)
		{
			for (size_t i = 0; i<m_References.size(); ++i)
			{
				float Angle = m_Angles[i], Reference = m_References[Idx++];

				LOG(INFO) << "\n------------------------------------------------------";
				LOG(INFO) << "\nTesting (angle=" << Angle << "): " << pBSDF->ToString();
				++Total;

				BSDFQueryRecord BSDFRecord(SphericalDirection(DegToRad(Angle), 0.0f));

				LOG(INFO) << "\nDrawing " << m_SampleCount << " samples .. ";

				double Mean = 0, Variance = 0;
				for (int k = 0; k < m_SampleCount; ++k)
				{
					Point2f Sample(Random.nextFloat(), Random.nextFloat());
					double Result = double(pBSDF->Sample(BSDFRecord, Sample).GetLuminance());

					/* Numerically robust online variance estimation using an
					algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
					double Delta = Result - Mean;
					Mean += Delta / double(k + 1);
					Variance += Delta * (Result - Mean);
				}

				Variance /= m_SampleCount - 1;

				std::pair<bool, std::string> Result = hypothesis::students_t_test(
					Mean,
					Variance,
					Reference,
					m_SampleCount,
					m_SignificanceLevel,
					int(m_References.size())
				);

				if (Result.first)
				{
					++Passed;
				}

				LOG(INFO) << Result.second;
			}
		}
	}
	else
	{
		if (m_References.size() != m_pScenes.size())
		{
			throw HikariException("Specified a different number of scenes and reference values!");
		}

		Sampler * pSampler = (Sampler*)(ObjectFactory::CreateInstance(XML_SAMPLER_INDEPENDENT, PropertyList()));

		int Idx = 0;
		for (Scene * pScene : m_pScenes)
		{
			const Integrator * pIntegrator = pScene->GetIntegrator();
			const Camera * pCamera = pScene->GetCamera();
			float Reference = m_References[Idx++];

			LOG(INFO) << "\n------------------------------------------------------";
			LOG(INFO) << "\nTesting scene: " << pScene->ToString();
			++Total;

			LOG(INFO) << "\nGenerating " << m_SampleCount << " paths.. ";

			double Mean = 0, Variance = 0;
			for (int k = 0; k < m_SampleCount; ++k)
			{
				/* Sample a ray from the camera */
				Ray3f Ray;
				Point2f PixelSample = (pSampler->Next2D().array() * pCamera->GetOutputSize().cast<float>().array()).matrix();
				Color3f Value = pCamera->SampleRay(Ray, PixelSample, pSampler->Next2D());

				/* Compute the incident radiance */
				Value *= pIntegrator->Li(pScene, pSampler, Ray);

				/* Numerically robust online variance estimation using an
				algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
				double Result = double(Value.GetLuminance());
				double Delta = Result - Mean;
				Mean += Delta / double(k + 1);
				Variance += Delta * (Result - Mean);
			}

			Variance /= m_SampleCount - 1;

			std::pair<bool, std::string> Result = hypothesis::students_t_test(
				Mean,
				Variance,
				Reference,
				m_SampleCount,
				m_SignificanceLevel,
				int(m_References.size())
			);

			if (Result.first)
			{
				++Passed;
			}

			LOG(INFO) << "\n" << Result.second;
		}
	}
	LOG(INFO) << "\nPassed " << Passed << "/" << Total << " tests.";
}

std::string StudentTTest::ToString() const
{
	return tfm::format(
		"StudentsTTest[\n"
		"  significanceLevel = %f,\n"
		"  sampleCount= %i\n"
		"]",
		m_SignificanceLevel,
		m_SampleCount
	);
}

Object::EClassType StudentTTest::GetClassType() const
{
	return EClassType::ETest;
}

NAMESPACE_END