#include <core\PropertyList.hpp>

NAMESPACE_BEGIN

PropertyList::PropertyList() { }

#define DEFINE_PROPERTY_ACCESSOR(CppType, TypeName, XmlName) \
	void PropertyList::Set##TypeName(const std::string & Name, const CppType & Value) \
	{ \
		if (m_XmlProperties.find(Name) != m_XmlProperties.end()) \
		{ \
			cerr << "Property \"" << Name <<  "\" was specified multiple times!" << endl; \
		} \
        auto &prop = m_XmlProperties[Name]; \
        prop.Value.XmlName##_XmlValue = Value; \
        prop.Type = XmlProperty::XmlName##_XmlType; \
    } \
    \
    CppType PropertyList::Get##TypeName(const std::string & Name) const \
	{ \
		auto it = m_XmlProperties.find(Name); \
		if (it == m_XmlProperties.end()) \
		{ \
			throw HikariException("Property '%s' is missing!", Name); \
		} \
		if (it->second.Type != XmlProperty::XmlName##_XmlType) \
		{ \
			throw HikariException("Property '%s' has the wrong type! (expected <" #XmlName ">)!", Name); \
		} \
		return it->second.Value.XmlName##_XmlValue; \
    } \
    \
    CppType PropertyList::Get##TypeName(const std::string & Name, const CppType & DefaultValue) const \
	{ \
		auto it = m_XmlProperties.find(Name); \
		if (it == m_XmlProperties.end()) \
        { \
			return DefaultValue; \
		} \
        if (it->second.Type != XmlProperty::XmlName##_XmlType) \
        { \
			throw HikariException("Property '%s' has the wrong type! (expected <" #XmlName ">)!", Name); \
		} \
        return it->second.Value.XmlName##_XmlValue; \
    }

DEFINE_PROPERTY_ACCESSOR(bool, Boolean, boolean)
DEFINE_PROPERTY_ACCESSOR(int, Integer, integer)
DEFINE_PROPERTY_ACCESSOR(float, Float, float)
DEFINE_PROPERTY_ACCESSOR(Color3f, Color, color)
DEFINE_PROPERTY_ACCESSOR(Point3f, Point, point)
DEFINE_PROPERTY_ACCESSOR(Vector3f, Vector, vector)
DEFINE_PROPERTY_ACCESSOR(std::string, String, string)
DEFINE_PROPERTY_ACCESSOR(Transform, Transform, transform)

NAMESPACE_END
