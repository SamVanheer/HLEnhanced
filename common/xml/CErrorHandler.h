#ifndef COMMON_XML_CERRORHANDLER_H
#define COMMON_XML_CERRORHANDLER_H

#include <spdlog/common.h>

#include <xercesc/sax/ErrorHandler.hpp>

namespace xml
{
class CErrorHandler : public xercesc::ErrorHandler
{
public:
	CErrorHandler() = default;
	~CErrorHandler() = default;

	void warning( const xercesc::SAXParseException& toCatch ) override;
	void error( const xercesc::SAXParseException& toCatch ) override;
	void fatalError( const xercesc::SAXParseException& toCatch ) override;
	void resetErrors() override;

	bool GetSawErrors() const;

private:
	void LogError( const xercesc::SAXParseException& toCatch, const spdlog::level::level_enum level, const char* pszPrefix = "" );

private:
	bool m_bSawErrors = false;
};

inline bool CErrorHandler::GetSawErrors() const
{
	return m_bSawErrors;
}
}

#endif //COMMON_XML_CERRORHANDLER_H
