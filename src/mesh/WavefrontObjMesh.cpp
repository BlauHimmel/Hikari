#include <mesh\WavefrontObjMesh.hpp>
#include <core\Timer.hpp>
#include <unordered_map>
#include <fstream>

NAMESPACE_BEGIN

REGISTER_CLASS(WavefrontObjMesh, XML_MESH_WAVEFRONG_OBJ);

WavefrontObjMesh::WavefrontObjMesh(const PropertyList & PropList)
{
	using VertexMap = std::unordered_map<ObjVertex, uint32_t, ObjVertexHash>;
	filesystem::path Filename = GetFileResolver()->resolve(PropList.GetString(XML_MESH_WAVEFRONG_OBJ_FILENAME));

	size_t FileSizeKB = Filename.file_size();
	size_t FileSizeMB = size_t(std::ceil(float(FileSizeKB) / (1024.0f * 1024.0f)));

	size_t ApproxVertexNumber = FileSizeMB * 5000;
	size_t ApproxFacetNumber = ApproxVertexNumber * 2;

	std::ifstream FileIn(Filename.str());
	if (FileIn.fail())
	{
		throw HikariException("Unable to open OBJ file \"%s\"!", Filename);
	}

	Transform Trans = PropList.GetTransform(XML_MESH_WAVEFRONG_OBJ_TO_WORLD, DEFAULT_MESH_TO_WORLD);

	LOG(INFO) << "Loading \"" << Filename << "\" ... ";
	cout.flush();
	Timer ObjTimer;

	std::vector<Vector3f> Positions;  Positions.reserve(ApproxVertexNumber);
	std::vector<Vector2f> Texcoords;  Texcoords.reserve(ApproxVertexNumber);
	std::vector<Vector3f> Normals;    Normals.reserve(ApproxVertexNumber);
	std::vector<uint32_t> Indices;    Indices.reserve(ApproxFacetNumber * 3);
	std::vector<ObjVertex> Vertices;  Vertices.reserve(ApproxVertexNumber);
	VertexMap Map;

	std::stringstream Line;
	std::string LineString;

	while (std::getline(FileIn, LineString))
	{
		Line.str("");
		Line.clear();
		Line << LineString;

		std::string Prefix;
		Line >> Prefix;

		if (Prefix == "v")
		{
			Point3f Pos;
			Line >> Pos.x() >> Pos.y() >> Pos.z();
			Pos = Trans * Pos;
			m_BBox.ExpandBy(Pos);
			Positions.push_back(Pos);
		}
		else if (Prefix == "vt")
		{
			Point2f Tex;
			Line >> Tex.x() >> Tex.y();
			Texcoords.push_back(Tex);
		}
		else if (Prefix == "vn")
		{
			Normal3f Norm;
			Line >> Norm.x() >> Norm.y() >> Norm.z();
			Normals.push_back((Trans * Norm).normalized());
		}
		else if (Prefix == "f")
		{
			std::string V1, V2, V3, V4;
			Line >> V1 >> V2 >> V3 >> V4;
			ObjVertex Verts[6];
			int nVertices = 3;

			Verts[0] = ObjVertex(V1);
			Verts[1] = ObjVertex(V2);
			Verts[2] = ObjVertex(V3);

			if (!V4.empty())
			{
				/* This is a quad, split into two triangles */
				Verts[3] = ObjVertex(V4);
				Verts[4] = Verts[0];
				Verts[5] = Verts[2];
				nVertices = 6;
			}

			/* Convert to an indexed vertex list */
			for (int i = 0; i < nVertices; ++i)
			{
				const ObjVertex & V = Verts[i];
				auto Iter = Map.find(V);
				if (Iter == Map.end())
				{
					Map[V] = uint32_t(Vertices.size());
					Indices.push_back(uint32_t(Vertices.size()));
					Vertices.push_back(V);
				}
				else
				{
					Indices.push_back(Iter->second);
				}
			}
		}
	}

	m_F.resize(3, Indices.size() / 3);
	memcpy(m_F.data(), Indices.data(), sizeof(uint32_t) * Indices.size());

	m_V.resize(3, Vertices.size());
	for (uint32_t i = 0; i < Vertices.size(); ++i)
	{
		m_V.col(i) = Positions.at(Vertices[i].P - 1);
	}

	if (!Normals.empty())
	{
		m_N.resize(3, Vertices.size());
		for (uint32_t i = 0; i < Vertices.size(); ++i)
		{
			m_N.col(i) = Normals.at(Vertices[i].N - 1);
		}
	}

	if (!Texcoords.empty())
	{
		m_UV.resize(2, Vertices.size());
		for (uint32_t i = 0; i < Vertices.size(); ++i)
		{
			m_UV.col(i) = Texcoords.at(Vertices[i].UV - 1);
		}
	}

	m_Name = Filename.str();
	LOG(INFO) << "Done. (V = " << m_V.cols() << ", F = " << m_F.cols() << ", took "
		<< ObjTimer.ElapsedString() << " and "
		<< MemString(m_F.size() * sizeof(uint32_t) + sizeof(float) * (m_V.size() + m_N.size() + m_UV.size()))
		<< ")";
}

WavefrontObjMesh::ObjVertex::ObjVertex() { }

WavefrontObjMesh::ObjVertex::ObjVertex(const std::string & String)
{
	std::vector<std::string> Tokens = Tokenize(String, "/", true);

	if (Tokens.size() < 1 || Tokens.size() > 3)
	{
		throw HikariException("Invalid vertex data: \"%s\"", String);
	}

	P = ToUInt(Tokens[0]);

	if (Tokens.size() >= 2 && !Tokens[1].empty())
	{
		UV = ToUInt(Tokens[1]);
	}

	if (Tokens.size() >= 3 && !Tokens[2].empty())
	{
		N = ToUInt(Tokens[2]);
	}
}

bool WavefrontObjMesh::ObjVertex::operator==(const ObjVertex & Rhs) const
{
	return Rhs.P == P && Rhs.N == N && Rhs.UV == UV;
}

size_t WavefrontObjMesh::ObjVertexHash::operator()(const ObjVertex & Rhs) const
{
	size_t Hash = std::hash<uint32_t>()(Rhs.P);
	Hash = Hash * 37 + std::hash<uint32_t>()(Rhs.UV);
	Hash = Hash * 37 + std::hash<uint32_t>()(Rhs.N);
	return Hash;
}

NAMESPACE_END
