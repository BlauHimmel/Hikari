#include <core\RoughTransmitance.hpp>
#include <core\Timer.hpp>
#include <core\Vector.hpp>

#include <fstream>

NAMESPACE_BEGIN

RoughTransmittance::RoughTransmittance(const std::string & DataFilename)
{
	LOG(INFO) << "Load rough fresnel transmittance data \"" << DataFilename << "\".";
	Timer RFTTimer;

	std::ifstream DataFile(DataFilename, std::ios::in | std::ios::binary);
	if (DataFile.fail())
	{
		throw HikariException("Load rough fresnel transmittance data \"%s\" failed.", DataFilename);
	}

	DataFile.read((char*)(&EtaSamples), sizeof(uint64_t));
	DataFile.read((char*)(&AlphaSamples), sizeof(uint64_t));
	DataFile.read((char*)(&ThetaSamples), sizeof(uint64_t));
	DataFile.read((char*)(&TransmittanceSize), sizeof(uint64_t));
	DataFile.read((char*)(&DiffTransmittanceSize), sizeof(uint64_t));
	DataFile.read((char*)(&EtaMin), sizeof(float));
	DataFile.read((char*)(&EtaMax), sizeof(float));
	DataFile.read((char*)(&AlphaMin), sizeof(float));
	DataFile.read((char*)(&AlphaMax), sizeof(float));
	Transmittances.resize(TransmittanceSize, 0.0f);
	DiffTransmittances.resize(DiffTransmittanceSize, 0.0f);
	for (uint64_t i = 0; i < TransmittanceSize; i++)
	{
		DataFile.read((char*)(&Transmittances[i]), sizeof(float));
	}
	for (uint64_t i = 0; i < DiffTransmittanceSize; i++)
	{
		DataFile.read((char*)(&DiffTransmittances[i]), sizeof(float));
	}

	bEtaFixed = false;
	bAlphaFixed = false;

	LOG(INFO) << "Done. (Took " << RFTTimer.ElapsedString() << " and " <<
		MemString(sizeof(bool) * 2 + sizeof(uint64_t) * 5 + sizeof(float) * (4 + TransmittanceSize + DiffTransmittanceSize)) << ")";
}

float RoughTransmittance::Eval(float CosTheta, float Alpha, float Eta) const
{
	float WarpedCosTheta = std::pow(std::abs(CosTheta), 0.25f);
	float Result;

	if (bAlphaFixed && bEtaFixed)
	{
		if (CosTheta < 0.0f)
		{
			return 0.0f;
		}

		Result = EvalCubicInterpolate1D(
			WarpedCosTheta,
			Transmittances.data(),
			int(ThetaSamples),
			0.0f,
			1.0f
		);
	}
	else if (bEtaFixed)
	{
		if (CosTheta < 0.0f)
		{
			return 0.0f;
		}

		float WarpedAlpha = std::pow((Alpha - AlphaMin) / (AlphaMax - AlphaMin), 0.25f);

		Result = EvalCubicInterpolate2D(
			Point2f(WarpedCosTheta, WarpedAlpha),
			Transmittances.data(),
			Point2i(int(ThetaSamples), int(AlphaSamples)),
			Point2f(0.0f),
			Point2f(1.0f)
		);
	}
	else
	{
		if (CosTheta < 0.0f)
		{
			CosTheta = -CosTheta;
			Eta = 1.0f / Eta;
		}

		const float * pData = Transmittances.data();

		if (Eta < 1.0f)
		{
			pData += EtaSamples * AlphaSamples * ThetaSamples;
			Eta = 1.0f / Eta;
		}

		if (Eta < EtaMin)
		{
			Eta = EtaMin;
		}

		float WarpedAlpha = std::pow((Alpha - AlphaMin) / (AlphaMax - AlphaMin), 0.25f);
		float WarpedEta = std::pow((Eta - EtaMin) / (EtaMax - EtaMin), 0.25f);

		Result = EvalCubicInterpolate3D(
			Point3f(WarpedCosTheta, WarpedAlpha, WarpedEta),
			pData,
			Point3i(int(ThetaSamples), int(AlphaSamples), int(EtaSamples)),
			Point3f(0.0f),
			Point3f(1.0f)
		);
	}

	return Clamp(Result, 0.0f, 1.0f);
}

float RoughTransmittance::EvalDiffuse(float Alpha, float Eta) const
{
	float Result;

	if (bAlphaFixed && bEtaFixed)
	{
		Result = DiffTransmittances[0];
	}
	else if (bEtaFixed)
	{
		float WarpedAlpha = std::pow((Alpha - AlphaMin) / (AlphaMax - AlphaMin), 0.25f);
		Result = EvalCubicInterpolate1D(
			WarpedAlpha,
			DiffTransmittances.data(),
			int(AlphaSamples),
			0.0f,
			1.0f
		);
	}
	else
	{
		const float * pData = DiffTransmittances.data();

		if (Eta < 1.0f)
		{
			pData += EtaSamples * AlphaSamples;
			Eta = 1.0f / Eta;
		}

		if (Eta < EtaMin)
		{
			Eta = EtaMin;
		}

		float WarpedAlpha = std::pow((Alpha - AlphaMin) / (AlphaMax - AlphaMin), 0.25f);
		float WarpedEta = std::pow((Eta - EtaMin) / (EtaMax - EtaMin), 0.25f);

		Result = EvalCubicInterpolate2D(
			Point2f(WarpedAlpha, WarpedEta),
			pData,
			Point2i(int(AlphaSamples), int(EtaSamples)),
			Point2f(0.0f),
			Point2f(1.0f)
		);
	}

	return Clamp(Result, 0.0f, 1.0f);
}

void RoughTransmittance::SetEta(float Eta)
{
	if (bEtaFixed)
	{
		return;
	}

	TransmittanceSize = AlphaSamples * ThetaSamples;
	DiffTransmittanceSize = AlphaSamples;

	LOG(INFO) << tfm::format(
		"Reducing dimension from 3D to 2D (%s), Eta = %f",
		MemString((TransmittanceSize + DiffTransmittanceSize) * sizeof(float)),
		Eta
	);

	float * pTransmittances = Transmittances.data();
	float * pDiffTransmittances = DiffTransmittances.data();

	if (Eta < 1.0f)
	{
		pTransmittances += EtaSamples * AlphaSamples * ThetaSamples;
		pDiffTransmittances += EtaSamples * AlphaSamples;
		Eta = 1.0f / Eta;
	}

	if (Eta < EtaMin)
	{
		Eta = EtaMin;
	}

	float WarpedEta = std::pow((Eta - EtaMin) / (EtaMax - EtaMin), 0.25f);

	std::vector<float> NewTransmittances, NewDiffTransmittances;
	NewTransmittances.resize(TransmittanceSize);
	NewDiffTransmittances.resize(DiffTransmittanceSize);

	float dAlpha = 1.0f / (AlphaSamples - 1.0f);
	float dTheta = 1.0f / (ThetaSamples - 1.0f);

	for (size_t i = 0; i < AlphaSamples; i++)
	{
		for (size_t j = 0; j < ThetaSamples; j++)
		{
			NewTransmittances[i * ThetaSamples + j] = EvalCubicInterpolate3D(
				Point3f(j * dTheta, i * dAlpha, WarpedEta),
				pTransmittances,
				Point3i(int(ThetaSamples), int(AlphaSamples), int(EtaSamples)),
				Point3f(0.0f),
				Point3f(1.0f)
			);
		}

		NewDiffTransmittances[i] = EvalCubicInterpolate2D(
			Point2f(i * dAlpha, WarpedEta),
			pDiffTransmittances,
			Point2i(int(AlphaSamples), int(EtaSamples)),
			Point2f(0.0f),
			Point2f(1.0f)
		);
	}

	Transmittances = NewTransmittances;
	DiffTransmittances = NewDiffTransmittances;
	bEtaFixed = true;
}

void RoughTransmittance::SetAlpha(float Alpha)
{
	if (!bEtaFixed)
	{
		throw HikariException("SetAlpha(): needs a preceding call to SetEta()!");
	}

	if (bAlphaFixed)
	{
		return;
	}

	TransmittanceSize = ThetaSamples;
	DiffTransmittanceSize = 1;

	LOG(INFO) << tfm::format(
		"Reducing dimension from 2D to 1D (%s), Alpha = %f",
		MemString((TransmittanceSize + DiffTransmittanceSize) * sizeof(float)),
		Alpha
	);

	float WarpedAlpha = std::pow((Alpha - AlphaMin) / (AlphaMax - AlphaMin), 0.25f);;

	std::vector<float> NewTransmittances, NewDiffTransmittances;
	NewTransmittances.resize(TransmittanceSize);
	NewDiffTransmittances.resize(DiffTransmittanceSize);

	float dTheta = 1.0f / (ThetaSamples - 1.0f);

	for (size_t i = 0; i < ThetaSamples; i++)
	{
		NewTransmittances[i] = EvalCubicInterpolate2D(
			Point2f(i * dTheta, WarpedAlpha),
			Transmittances.data(),
			Point2i(int(ThetaSamples), int(AlphaSamples)),
			Point2f(0.0f),
			Point2f(1.0f)
		);
	}

	NewDiffTransmittances[0] = EvalCubicInterpolate1D(
		WarpedAlpha,
		DiffTransmittances.data(),
		int(AlphaSamples),
		0.0f,
		1.0f
	);

	Transmittances = NewTransmittances;
	DiffTransmittances = NewDiffTransmittances;
	bAlphaFixed = true;
}

void RoughTransmittance::CheckEta(float Eta) const
{
	if (Eta < 1.0f)
	{
		Eta = 1.0f / Eta;
	}

	if (Eta < EtaMin || Eta > EtaMax)
	{
		throw HikariException(
			"Requested relative index of refraction Eta = %f "
			"is outside of the supported range [%f, %f]! Please update "
			"your scene so that it uses realistic IOR values.",
			Eta, EtaMin, EtaMax
		);
	}
}

void RoughTransmittance::CheckAlpha(float Alpha) const
{
	if (Alpha < AlphaMin || Alpha > AlphaMax)
	{
		throw HikariException(
			"Requested roughness value Alpha = %f "
			"is outside of the supported range [%f, %f]! Please update "
			"your scene to make it within this range.",
			Alpha, AlphaMin, AlphaMax
		);
	}
}

std::unique_ptr<RoughTransmittance> RoughTransmittance::Clone() const
{
	std::unique_ptr<RoughTransmittance> Ret(new RoughTransmittance());

	Ret->EtaSamples = EtaSamples;
	Ret->AlphaSamples = AlphaSamples;
	Ret->ThetaSamples = ThetaSamples;
	Ret->bEtaFixed = bEtaFixed;
	Ret->bAlphaFixed = bAlphaFixed;
	Ret->EtaMin = EtaMin;
	Ret->EtaMax = EtaMax;
	Ret->AlphaMin = AlphaMin;
	Ret->AlphaMax = AlphaMax;
	Ret->TransmittanceSize = TransmittanceSize;
	Ret->DiffTransmittanceSize = DiffTransmittanceSize;
	Ret->Transmittances = Transmittances;
	Ret->DiffTransmittances = DiffTransmittances;

	return Ret;
}


NAMESPACE_END