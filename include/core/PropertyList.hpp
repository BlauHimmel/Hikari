#pragma once

#include <core\Common.hpp>
#include <core\Transform.hpp>
#include <core\Color.hpp>

NAMESPACE_BEGIN

/**
* \brief This is an associative container used to supply the constructors
* of \ref Object subclasses with parameter information.
*/
class PropertyList
{
public:
	PropertyList();

	/// Set a boolean property
	void SetBoolean(const std::string & Name, const bool & Value);

	/// Get a boolean property, and throw an exception if it does not exist
	bool GetBoolean(const std::string & Name) const;

	/// Get a boolean property, and use a default value if it does not exist
	bool GetBoolean(const std::string & Name, const bool & DefaultValue) const;

	/// Set an integer property
	void SetInteger(const std::string & Name, const int & Value);

	/// Get an integer property, and throw an exception if it does not exist
	int GetInteger(const std::string & Name) const;

	/// Get am integer property, and use a default value if it does not exist
	int GetInteger(const std::string & Name, const int & DefaultValue) const;

	/// Set a float property
	void SetFloat(const std::string & Name, const float & Value);

	/// Get a float property, and throw an exception if it does not exist
	float GetFloat(const std::string & Name) const;

	/// Get a float property, and use a default value if it does not exist
	float GetFloat(const std::string & Name, const float & DefaultValue) const;

	/// Set a string property
	void SetString(const std::string & Name, const std::string & Value);

	/// Get a string property, and throw an exception if it does not exist
	std::string GetString(const std::string & Name) const;

	/// Get a string property, and use a default value if it does not exist
	std::string GetString(const std::string & Name, const std::string & DefaultValue) const;

	/// Set a color property
	void SetColor(const std::string & Name, const Color3f & Value);

	/// Get a color property, and throw an exception if it does not exist
	Color3f GetColor(const std::string & Name) const;

	/// Get a color property, and use a default value if it does not exist
	Color3f GetColor(const std::string & Name, const Color3f & DefaultValue) const;

	/// Set a point property
	void SetPoint(const std::string & Name, const Point3f & Value);

	/// Get a point property, and throw an exception if it does not exist
	Point3f GetPoint(const std::string & Name) const;

	/// Get a point property, and use a default value if it does not exist
	Point3f GetPoint(const std::string & Name, const Point3f & DefaultValue) const;

	/// Set a vector property
	void SetVector(const std::string & Name, const Vector3f & Value);

	/// Get a vector property, and throw an exception if it does not exist
	Vector3f GetVector(const std::string & Name) const;

	/// Get a vector property, and use a default value if it does not exist
	Vector3f GetVector(const std::string & Name, const Vector3f & DefaultValue) const;

	/// Set a transform property
	void SetTransform(const std::string & Name, const Transform & Value);

	/// Get a transform property, and throw an exception if it does not exist
	Transform GetTransform(const std::string & Name) const;

	/// Get a transform property, and use a default value if it does not exist
	Transform GetTransform(const std::string & Name, const Transform & DefaultValue) const;

private:
	/* Custom variant data type (stores one of boolean/integer/float/...) */
	struct XmlProperty
	{
		enum
		{
			XML_TYPE(boolean)   = 0,
			XML_TYPE(integer)   = 1,
			XML_TYPE(float)     = 2,
			XML_TYPE(string)    = 3,
			XML_TYPE(color)     = 4,
			XML_TYPE(point)     = 5,
			XML_TYPE(vector)    = 6,
			XML_TYPE(transform) = 7
		} Type;

		struct Value
		{
			Value() : XML_VALUE(boolean)(false) { }
			~Value() { }

			bool XML_VALUE(boolean);
			int XML_VALUE(integer);
			float XML_VALUE(float);
			std::string XML_VALUE(string);
			Color3f XML_VALUE(color);
			Point3f XML_VALUE(point);
			Vector3f XML_VALUE(vector);
			Transform XML_VALUE(transform);
		} Value;
		
		XmlProperty() : Type(XML_TYPE(boolean)) { }
	};

	std::map<std::string, XmlProperty> m_XmlProperties;
};

NAMESPACE_END