#ifndef COMMON_XML_CSTRX_H
#define COMMON_XML_CSTRX_H

#include <xercesc/util/XMLString.hpp>

namespace xml
{
/**
*	@brief Class used to easily convert Xerces strings to native codepage strings
*	Based on Xerces' StrX helper class
*/
class CStrX
{
public:
	CStrX( const XMLCh* const toTranscode )
	{
		// Call the private transcoding method
		m_pszLocalForm = xercesc::XMLString::transcode( toTranscode );
	}

	~CStrX()
	{
		xercesc::XMLString::release( &m_pszLocalForm );
	}

	CStrX( CStrX&& other )
	{
		m_pszLocalForm = other.m_pszLocalForm;

		other.m_pszLocalForm = nullptr;
	}

	CStrX& operator=( CStrX&& other )
	{
		if( this != &other )
		{
			m_pszLocalForm = other.m_pszLocalForm;

			other.m_pszLocalForm = nullptr;
		}

		return *this;
	}

	const char* LocalForm() const
	{
		return m_pszLocalForm;
	}

private:
	char* m_pszLocalForm;

private:
	CStrX( const CStrX& ) = delete;
	CStrX& operator=( const CStrX& ) = delete;
};
}

#endif //COMMON_XML_CSTRX_H
