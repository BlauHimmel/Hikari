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
			boolean_XmlType   = 0,
			integer_XmlType   = 1,
			float_XmlType     = 2,
			string_XmlType    = 3,
			color_XmlType     = 4,
			point_XmlType     = 5,
			vector_XmlType    = 6,
			transform_XmlType = 7
		} Type;

		struct Value
		{
			Value() : boolean_XmlValue(false) { }
			~Value() { }

			bool boolean_XmlValue;
			int integer_XmlValue;
			float float_XmlValue;
			std::string string_XmlValue;
			Color3f color_XmlValue;
			Point3f point_XmlValue;
			Vector3f vector_XmlValue;
			Transform transform_XmlValue;

		} Value;

		XmlProperty() : Type(boolean_XmlType) { }
	};

	std::map<std::string, XmlProperty> m_XmlProperties;
};

NAMESPACE_END