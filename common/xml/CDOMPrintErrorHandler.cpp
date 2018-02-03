#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>

#include "extdll.h"
#include "util.h"

#include "CDOMPrintErrorHandler.h"
#include "CStrX.h"
#include "Logging.h"

namespace xml
{
bool CDOMPrintErrorHandler::handleError( const xercesc::DOMError& domError )
{
	const char* pszPrefix = "";

	auto level = spdlog::level::level_enum::info;

	// Display whatever error message passed from the serializer
	switch( domError.getSeverity() )
	{
	case xercesc::DOMError::DOM_SEVERITY_WARNING:
		{
			level = spdlog::level::level_enum::warn;
			break;
		}

	case xercesc::DOMError::DOM_SEVERITY_FATAL_ERROR:
		{
			pszPrefix = "Fatal: ";
		}

		//[[ fallthrough ]]

	case xercesc::DOMError::DOM_SEVERITY_ERROR:
		{
			level = spdlog::level::level_enum::err;
			break;
		}

	default: break;
	}

	auto pLocator = domError.getLocation();

	log->log( level, "{}Line {}, column {}: {}", pszPrefix, pLocator->getLineNumber(), pLocator->getColumnNumber(), CStrX( domError.getMessage() ).LocalForm() );

	// Instructs the serializer to continue serialization if possible.
	return true;
}
}
