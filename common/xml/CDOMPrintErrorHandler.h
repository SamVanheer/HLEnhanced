#ifndef COMMON_XML_CDOMPRINTERRORHANDLER_H
#define COMMON_XML_CDOMPRINTERRORHANDLER_H

#include <xercesc/dom/DOMErrorHandler.hpp>

namespace xml
{
class CDOMPrintErrorHandler : public xercesc::DOMErrorHandler
{
public:
	CDOMPrintErrorHandler() = default;
	~CDOMPrintErrorHandler() = default;

	bool handleError( const xercesc::DOMError& domError ) override;
};
}

#endif //COMMON_XML_CDOMPRINTERRORHANDLER_H
