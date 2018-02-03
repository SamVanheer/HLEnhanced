#ifndef COMMON_XML_CXSTR_H
#define COMMON_XML_CXSTR_H

#include <xercesc/util/XMLString.hpp>

namespace xml
{
/**
*	@brief Class used to easily convert native codepage strings to Xerces strings
*	Based on Xerces' XStr helper class
*/
class CXStr
{
public:
	CXStr( const char* const toTranscode )
	{
		// Call the private transcoding method
		m_pszUnicodeForm = xercesc::XMLString::transcode( toTranscode );
	}

	~CXStr()
	{
		xercesc::XMLString::release( &m_pszUnicodeForm );
	}

	CXStr( CXStr&& other )
	{
		m_pszUnicodeForm = other.m_pszUnicodeForm;

		other.m_pszUnicodeForm = nullptr;
	}

	CXStr& operator=( CXStr&& other )
	{
		if( this != &other )
		{
			m_pszUnicodeForm = other.m_pszUnicodeForm;

			other.m_pszUnicodeForm = nullptr;
		}

		return *this;
	}

	const XMLCh* UnicodeForm() const
	{
		return m_pszUnicodeForm;
	}

private:
	XMLCh* m_pszUnicodeForm;

private:
	CXStr( const CXStr& ) = delete;
	CXStr& operator=( const CXStr& ) = delete;
};
}

#endif //COMMON_XML_CXSTR_H
