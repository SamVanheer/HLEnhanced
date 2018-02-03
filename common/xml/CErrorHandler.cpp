#include <xercesc/sax/SAXParseException.hpp>

#include "extdll.h"
#include "util.h"

#include "CErrorHandler.h"
#include "CStrX.h"
#include "Logging.h"

namespace xml
{
void CErrorHandler::warning( const xercesc::SAXParseException& toCatch )
{
	LogError( toCatch, spdlog::level::level_enum::warn, "Fatal: " );
}

void CErrorHandler::error( const xercesc::SAXParseException& toCatch )
{
	LogError( toCatch, spdlog::level::level_enum::err );
}

void CErrorHandler::fatalError( const xercesc::SAXParseException& toCatch )
{
	LogError( toCatch, spdlog::level::level_enum::err, "Fatal: " );
}

void CErrorHandler::resetErrors()
{
	m_bSawErrors = false;
}

void CErrorHandler::LogError( const xercesc::SAXParseException& toCatch, const spdlog::level::level_enum level, const char* pszPrefix )
{
	m_bSawErrors = true;

	log->log( level, "{}Line {}, column {}: {}", pszPrefix, toCatch.getLineNumber(), toCatch.getColumnNumber(), CStrX( toCatch.getMessage() ).LocalForm() );
}
}
