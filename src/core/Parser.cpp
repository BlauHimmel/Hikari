#include <core\Parser.hpp>
#include <core\PropertyList.hpp>
#include <pugixml.hpp>
#include <fstream>
#include <set>

NAMESPACE_BEGIN

Object * LoadFromXML(const std::string & Filename)
{
	/* Load the XML file */
	pugi::xml_document Doc;
	pugi::xml_parse_result Result = Doc.load_file(Filename.c_str());

	/* Helper function: map a position offset in bytes to a more readable line/column value */
	auto OffsetFunc = [&](ptrdiff_t Pos) -> std::string
	{
		std::fstream IS(Filename);
		char Buffer[1024];
		int Line = 0, Linestart = 0, Offset = 0;
		while (IS.good())
		{
			IS.read(Buffer, sizeof(Buffer));
			for (int i = 0; i < IS.gcount(); ++i)
			{
				if (Buffer[i] == '\n')
				{
					if (Offset + i >= Pos)
					{
						return tfm::format("line %i, col %i", Line + 1, Pos - Linestart);
					}
					++Line;
					Linestart = Offset + i;
				}
			}
			Offset += (int)IS.gcount();
		}
		return "byte offset " + std::to_string(Pos);
	};

	if (!Result) /* There was a parser / file IO error */
	{
		throw HikariException("Error while parsing \"%s\": %s (at %s)", Filename, Result.description(), OffsetFunc(Result.offset));
	}

	/* Set of supported XML tags */
	enum ETag
	{
		/* Object classes */
		EScene                = Object::EClassType::EScene,
		EMesh                 = Object::EClassType::EMesh,
		EBSDF                 = Object::EClassType::EBSDF,
		EPhaseFunction        = Object::EClassType::EPhaseFunction,
		EEmitter              = Object::EClassType::EEmitter,
		EMedium               = Object::EClassType::EMedium,
		ECamera               = Object::EClassType::ECamera,
		EIntegrator           = Object::EClassType::EIntegrator,
		ESampler              = Object::EClassType::ESampler,
		ETest                 = Object::EClassType::ETest,
		EReconstructionFilter = Object::EClassType::EReconstructionFilter,
		EAcceleration         = Object::EClassType::EAcceleration,

		/* Properties */
		EBoolean              = Object::EClassType::EClassTypeCount,
		EInteger,
		EFloat,
		EString,
		EPoint,
		EVector,
		EColor,
		ETransform,
		ETranslate,
		EMatrix,
		ERotate,
		EScale,
		ELookAt,

		EInvalid
	};

	/* Create a mapping from tag names to tag IDs */
	std::map<std::string, ETag> Tags;
	Tags[XML_SCENE]                = EScene;
	Tags[XML_MESH]                 = EMesh;
	Tags[XML_BSDF]                 = EBSDF;
	Tags[XML_PHASE]                = EPhaseFunction;
	Tags[XML_EMITTER]              = EEmitter;
	Tags[XML_MEDIUM]               = EMedium;
	Tags[XML_CAMERA]               = ECamera;
	Tags[XML_INTEGRATOR]           = EIntegrator;
	Tags[XML_SAMPLER]              = ESampler;
	Tags[XML_TEST]                 = ETest;
	Tags[XML_FILTER]               = EReconstructionFilter;
	Tags[XML_ACCELERATION]         = EAcceleration;
	Tags["boolean"]                = EBoolean;
	Tags["integer"]                = EInteger;
	Tags["float"]                  = EFloat;
	Tags["string"]                 = EString;
	Tags["point"]                  = EPoint;
	Tags["vector"]                 = EVector;
	Tags["color"]                  = EColor;
	Tags["transform"]              = ETransform;
	Tags[XML_TRANSFORM_TRANSLATE]  = ETranslate;
	Tags[XML_TRANSFORM_MATRIX]     = EMatrix;
	Tags[XML_TRANSFORM_ROTATE]     = ERotate;
	Tags[XML_TRANSFORM_SCALE]      = EScale;
	Tags[XML_TRANSFORM_LOOKAT]     = ELookAt;

	/* Helper function to check if attributes are fully specified */
	auto CheckAttributesFunc = [&](const pugi::xml_node & Node, std::set<std::string> Attrs)
	{
		for (auto Attr : Node.attributes())
		{
			auto Iter = Attrs.find(Attr.name());
			if (Iter == Attrs.end())
			{
				throw HikariException(
					"Unexpected attribute \"%s\" in \"%s\" at %s",
					Attr.name(), Node.name(), OffsetFunc(Node.offset_debug())
				);
			}
			Attrs.erase(Iter);
		}
		if (!Attrs.empty())
		{
			throw HikariException(
				"Missing attribute \"%s\" in \"%s\" at %s",
				*Attrs.begin(), Node.name(), OffsetFunc(Node.offset_debug())
			);
		}
	};

	Eigen::Affine3f Trans;

	/* Helper function to parse a Nori XML node (recursive) */
	std::function<Object*(pugi::xml_node &, PropertyList &, ETag)> ParseTagFunc = [&]
	(pugi::xml_node & Node, PropertyList & PropList, ETag ParentTag) -> Object *
	{
		/* Skip over comments */
		if (Node.type() == pugi::node_comment || Node.type() == pugi::node_declaration)
		{
			return nullptr;
		}

		if (Node.type() != pugi::node_element)
		{
			throw HikariException(
				"Error while parsing \"%s\": unexpected content at %s",
				Filename, OffsetFunc(Node.offset_debug())
			);
		}

		/* Look up the name of the current element */
		auto Iter = Tags.find(Node.name());
		if (Iter == Tags.end())
		{
			throw HikariException(
				"Error while parsing \"%s\": unexpected tag \"%s\" at %s",
				Filename, Node.name(), OffsetFunc(Node.offset_debug())
			);
		}
		ETag Tag = Iter->second;

		/* Perform some safety checks to make sure that the XML tree really makes sense */
		bool bHasParent = int(ParentTag) != int(ETag::EInvalid);
		bool bParentIsObject = bHasParent && int(ParentTag) < int(Object::EClassType::EClassTypeCount);
		bool bCurrentIsObject = int(Tag) < int(Object::EClassType::EClassTypeCount);
		bool bParentIsTransform = int(ParentTag) == int(ETag::ETransform);
		bool bCurrentIsTransformOp = (
			Tag == ETag::ETranslate ||
			Tag == ETag::ERotate ||
			Tag == ETag::EScale ||
			Tag == ETag::ELookAt ||
			Tag == ETag::EMatrix
			);

		if (!bHasParent && !bCurrentIsObject)
		{
			throw HikariException(
				"Error while parsing \"%s\": root element \"%s\" must be a Object (at %s)",
				Filename, Node.name(), OffsetFunc(Node.offset_debug())
			);
		}

		if (bParentIsTransform != bCurrentIsTransformOp)
		{
			throw HikariException(
				"Error while parsing \"%s\": transform nodes can only contain transform operations (at %s)",
				Filename, OffsetFunc(Node.offset_debug())
			);
		}

		if (bHasParent && !bParentIsObject && !(bParentIsTransform && bCurrentIsTransformOp))
		{
			throw HikariException(
				"Error while parsing \"%s\": node \"%s\" requires a Object as parent (at %s)",
				Filename, Node.name(), OffsetFunc(Node.offset_debug())
			);
		}

		if (Tag == ETag::EScene)
		{
			Node.append_attribute("type") = "scene";
		}
		else if (Tag == ETag::ETransform)
		{
			Trans.setIdentity();
		}

		PropertyList ChildPropList;
		std::vector<Object*> pChildren;
		for (pugi::xml_node & Child : Node.children())
		{
			Object * pChild = ParseTagFunc(Child, ChildPropList, Tag);
			if (pChild)
			{
				pChildren.push_back(pChild);
			}
		}

		Object * pResult = nullptr;

		try
		{
			if (bCurrentIsObject)
			{
				CheckAttributesFunc(Node, { "type" });

				/* This is an object, first instantiate it */
				pResult = ObjectFactory::CreateInstance(Node.attribute("type").value(), ChildPropList);

				if (pResult->GetClassType() != Object::EClassType(Tag))
				{
					throw HikariException(
						"Unexpectedly constructed an object of type <%s> (expected type <%s>): %s",
						Object::ClassTypeName(pResult->GetClassType()),
						Object::ClassTypeName(Object::EClassType(Tag)),
						pResult->ToString()
					);
				}

				/* Add all children */
				for (auto pChild : pChildren)
				{
					pResult->AddChild(pChild);
					pChild->SetParent(pResult);
				}

				/* Activate / configure the object */
				pResult->Activate();
			}
			else
			{
				/* This is a property */
				switch (Tag)
				{
				case ETag::EString:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetString(Node.attribute("name").value(), Node.attribute("value").value());
					break;
				case ETag::EFloat:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetFloat(Node.attribute("name").value(), ToFloat(Node.attribute("value").value()));
					break;
				case ETag::EInteger:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetInteger(Node.attribute("name").value(), ToInt(Node.attribute("value").value()));
					break;
				case ETag::EBoolean:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetBoolean(Node.attribute("name").value(), ToBool(Node.attribute("value").value()));
					break;
				case ETag::EPoint:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetPoint(Node.attribute("name").value(), Point3f(ToVector3f(Node.attribute("value").value())));
					break;
				case ETag::EVector:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetVector(Node.attribute("name").value(), Vector3f(ToVector3f(Node.attribute("value").value())));
					break;
				case ETag::EColor:
					CheckAttributesFunc(Node, { "name", "value" });
					PropList.SetColor(Node.attribute("name").value(), Color3f(ToVector3f(Node.attribute("value").value()).array()));
					break;
				case ETag::ETransform:
					CheckAttributesFunc(Node, { "name" });
					PropList.SetTransform(Node.attribute("name").value(), Trans.matrix());
					break;
				case ETag::ETranslate:
				{
					CheckAttributesFunc(Node, { "value" });
					Eigen::Vector3f V = ToVector3f(Node.attribute("value").value());
					Trans = Eigen::Translation<float, 3>(V.x(), V.y(), V.z()) * Trans;
					break;
				}
				case ETag::EMatrix:
				{
					CheckAttributesFunc(Node, { "value" });
					std::vector<std::string> Tokens = Tokenize(Node.attribute("value").value());
					if (Tokens.size() != 16)
					{
						throw HikariException("Expected 16 values");
					}
					Eigen::Matrix4f Mat;
					for (int i = 0; i < 4; ++i)
					{
						for (int j = 0; j < 4; ++j)
						{
							Mat(i, j) = ToFloat(Tokens[i * 4 + j]);
						}
					}
					Trans = Eigen::Affine3f(Mat) * Trans;
					break;
				}
				case ETag::EScale:
				{
					CheckAttributesFunc(Node, { "value" });
					Eigen::Vector3f V = ToVector3f(Node.attribute("value").value());
					Trans = Eigen::DiagonalMatrix<float, 3>(V) * Trans;
					break;
				}
				case ETag::ERotate:
				{	
					CheckAttributesFunc(Node, { XML_TRANSFORM_ANGLE, XML_TRANSFORM_AXIS });
					float Angle = DegToRad(ToFloat(Node.attribute(XML_TRANSFORM_ANGLE).value()));
					Eigen::Vector3f Axis = ToVector3f(Node.attribute(XML_TRANSFORM_AXIS).value());
					Trans = Eigen::AngleAxis<float>(Angle, Axis) * Trans;
					break;
				}
				case ETag::ELookAt:
				{
					CheckAttributesFunc(Node, { XML_TRANSFORM_ORIGIN, XML_TRANSFORM_TARGET, XML_TRANSFORM_UP });
					Eigen::Vector3f Origin = ToVector3f(Node.attribute(XML_TRANSFORM_ORIGIN).value());
					Eigen::Vector3f Target = ToVector3f(Node.attribute(XML_TRANSFORM_TARGET).value());
					Eigen::Vector3f Up = ToVector3f(Node.attribute(XML_TRANSFORM_UP).value());

					Vector3f Dir = (Target - Origin).normalized();
					Vector3f Left = Up.normalized().cross(Dir).normalized();
					Vector3f NewUp = Dir.cross(Left).normalized();

					Eigen::Matrix4f LookAtMat;
					LookAtMat << Left, NewUp, Dir, Origin, 0, 0, 0, 1;

					Trans = Eigen::Affine3f(LookAtMat) * Trans;
					break;
				}
				default: 
					throw HikariException("Unhandled element \"%s\"", Node.name());
				};
			}
		}
		catch (const HikariException & Ex) 
		{
			throw HikariException(
				"Error while parsing \"%s\": %s (at %s)", 
				Filename, Ex.what(), OffsetFunc(Node.offset_debug())
			);
		}

		return pResult;
	};

	PropertyList PropList;
	return ParseTagFunc(*Doc.begin(), PropList, ETag::EInvalid);
}

NAMESPACE_END