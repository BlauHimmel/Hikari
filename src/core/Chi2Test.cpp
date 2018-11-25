#include <core\Chi2Test.hpp>
#include <core\Sampling.hpp>
#include <core\BSDF.hpp>
#include <hypothesis.h>
#include <pcg32.h>

NAMESPACE_BEGIN

REGISTER_CLASS(Chi2Test, XML_TEST_CHI2);

Chi2Test::Chi2Test(const PropertyList & PropList)
{
	/* The null hypothesis will be rejected when the associated
	p-value is below the significance level specified here. */
	m_SignificanceLevel = PropList.GetFloat(XML_TEST_CHI2_SIGNIFICANCE_LEVEL, DEFAULT_TEST_CHI2_SIGNIFICANCE_LEVEL);

	/* Number of cells along the latitudinal axis. The azimuthal
	resolution is twice this value. */
	m_CosThetaResolution = PropList.GetInteger(XML_TEST_CHI2_RESOLUTION, DEFAULT_TEST_CHI2_RESOLUTION);

	/* Minimum expected bin frequency. The chi^2 test does not
	work reliably when the expected frequency in a cell is
	low (e.g. less than 5), because normality assumptions
	break down in this case. Therefore, the implementation
	will merge such low-frequency cells when they fall below
	the threshold specified here. */
	m_MinExpFrequency = PropList.GetInteger(XML_TEST_CHI2_MIN_EXP_FREQUENCY, DEFAULT_TEST_CHI2_MIN_EXP_FREQUENCY);

	/* Number of samples that should be taken (-1: automatic) */
	m_SampleCount = PropList.GetInteger(XML_TEST_CHI2_SAMPLE_COUNT, DEFAULT_TEST_CHI2_SAMPLE_COUNT);

	/* Each provided BSDF will be tested for a few different
	incident directions. The value specified here determines
	how many tests will be executed per BSDF */
	m_TestCount = PropList.GetInteger(XML_TEST_CHI2_TEST_COUNT, DEFAULT_TEST_CHI2_TEST_COUNT);

	m_PhiResolution = 2 * m_CosThetaResolution;

	if (m_SampleCount < 0)
	{
		// ~5K samples per bin
		m_SampleCount = m_CosThetaResolution * m_PhiResolution * 5000;
	}
}

Chi2Test::~Chi2Test()
{
	for (BSDF * pBSDF : m_pBSDFs)
	{
		delete pBSDF;
	}
}

void Chi2Test::AddChild(Object * pChildObj)
{
	switch (pChildObj->GetClassType())
	{
	case EClassType::EBSDF:
		m_pBSDFs.push_back((BSDF*)(pChildObj));
		break;
	default:
		throw HikariException(
			"Chi2Test::AddChild(<%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType())
		);
		break;
	}
}

void Chi2Test::Activate()
{
	int Passed = 0, Total = 0, Resolution = m_CosThetaResolution * m_PhiResolution;
	/* Pseudorandom number generator */
	pcg32 Random;

	std::unique_ptr<double[]> pObservedFrequencies(new double[Resolution]);
	std::unique_ptr<double[]> pExpectedFrequencies(new double[Resolution]);

	/* Test each registered BSDF */
	for (BSDF * pBSDF : m_pBSDFs)
	{
		/* Run several tests per BSDF to be on the safe side */
		for (int l = 0; l < m_TestCount; ++l)
		{
			memset(pObservedFrequencies.get(), 0, Resolution * sizeof(double));
			memset(pExpectedFrequencies.get(), 0, Resolution * sizeof(double));

			LOG(INFO) << "\n------------------------------------------------------";
			LOG(INFO) << "\nTesting: " << pBSDF->ToString();
			++Total;

			float CosTheta = Random.nextFloat();
			float SinTheta = std::sqrt(std::max(0.0f, 1.0f - CosTheta * CosTheta));
			float SinPhi, CosPhi;
			SinCos(2.0f * float(M_PI) * Random.nextFloat(), &SinPhi, &CosPhi);
			Vector3f Wi(CosPhi * SinTheta, SinPhi * SinTheta, CosTheta);

			LOG(INFO) << "\nAccumulating " << m_SampleCount << " samples into a " << m_CosThetaResolution << "x" << m_PhiResolution << " contingency table .. ";

			/* Generate many samples from the BSDF and create a histogram / contingency table */
			BSDFQueryRecord BSDFRecord(Wi);
			for (int i = 0; i < m_SampleCount; ++i)
			{
				Point2f Sample(Random.nextFloat(), Random.nextFloat());
				Color3f Result = pBSDF->Sample(BSDFRecord, Sample);

				if ((Result.array() == 0).all())
				{
					continue;
				}

				int CosThetaBin = std::min(
					std::max(
						0, 
						int(std::floor((BSDFRecord.Wo.z() * 0.5f + 0.5f) * m_CosThetaResolution))
					), 
					m_CosThetaResolution - 1
				);

				float ScaledPhi = std::atan2(BSDFRecord.Wo.y(), BSDFRecord.Wo.x()) * float(INV_TWOPI);
				if (ScaledPhi < 0)
				{
					ScaledPhi += 1;
				}

				int PhiBin = std::min(
					std::max(
						0,
						int(std::floor(ScaledPhi * m_PhiResolution))), 
					m_PhiResolution - 1
				);
				pObservedFrequencies[CosThetaBin * m_PhiResolution + PhiBin] += 1;
			}
			LOG(INFO) << "\ndone." << endl;

			/* Numerically integrate the probability density function over rectangles in spherical coordinates. */
			double * pPtr = pExpectedFrequencies.get();
			LOG(INFO) << "\nIntegrating expected frequencies .. ";

			for (int i = 0; i < m_CosThetaResolution; ++i)
			{
				double CosThetaStart = -1.0 + i * 2.0 / m_CosThetaResolution;
				double CosThetaEnd = -1.0 + (i + 1) * 2.0 / m_CosThetaResolution;
				
				for (int j = 0; j < m_PhiResolution; ++j)
				{
					double PhiStart = j * 2 * float(M_PI) / m_PhiResolution;
					double PhiEnd = (j + 1) * 2 * float(M_PI) / m_PhiResolution;

					auto IntegrandFunc = [&](double CosTheta, double Phi) -> double
					{
						double SinTheta = std::sqrt(1.0f - CosTheta * CosTheta);
						double SinPhi = std::sin(Phi), CosPhi = std::cos(Phi);

						Vector3f Wo(
							float(SinTheta * CosPhi),
							float(SinTheta * SinPhi),
							float(CosTheta)
						);

						BSDFQueryRecord BSDFRecord(Wi, Wo, EMeasure::ESolidAngle);
						return pBSDF->Pdf(BSDFRecord);
					};

					double Integral = hypothesis::adaptiveSimpson2D(IntegrandFunc, CosThetaStart, PhiStart, CosThetaEnd, PhiEnd);

					*pPtr++ = Integral * m_SampleCount;
				}
			}

			LOG(INFO) << "\ndone." << endl;

			/* Write the test input data to disk for debugging */
			hypothesis::chi2_dump(
				m_CosThetaResolution,
				m_PhiResolution,
				pObservedFrequencies.get(),
				pExpectedFrequencies.get(),
				tfm::format("chi2test_%i.m", Total)
			);

			/* Perform the Chi^2 test */
			std::pair<bool, std::string> Result = hypothesis::chi2_test(
				m_CosThetaResolution * m_PhiResolution,
				pObservedFrequencies.get(),
				pExpectedFrequencies.get(),
				m_SampleCount,
				m_MinExpFrequency,
				m_SignificanceLevel,
				m_TestCount * int(m_pBSDFs.size())
			);

			if (Result.first)
			{
				++Passed;
			}

			LOG(INFO) << "\n" << Result.second ;
		}
	}

	LOG(INFO) << "\nPassed " << Passed << "/" << Total << " tests.";
}

std::string Chi2Test::ToString() const
{
	return tfm::format(
		"ChiSquareTest[\n"
		"  thetaResolution = %i,\n"
		"  phiResolution = %i,\n"
		"  minExpFrequency = %i,\n"
		"  sampleCount = %i,\n"
		"  testCount = %i,\n"
		"  significanceLevel = %f\n"
		"]",
		m_CosThetaResolution,
		m_PhiResolution,
		m_MinExpFrequency,
		m_SampleCount,
		m_TestCount,
		m_SignificanceLevel
	);
}

Object::EClassType Chi2Test::GetClassType() const
{
	return EClassType::ETest;
}

NAMESPACE_END