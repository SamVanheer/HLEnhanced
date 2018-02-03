#ifndef COMMON_XML_CSTDOUTFORMATTARGET_H
#define COMMON_XML_CSTDOUTFORMATTARGET_H

#include <xercesc/framework/XMLFormatter.hpp>

namespace xml
{
/**
*	@brief Format target that enables output to standard output
*/
class CStdOutFormatTarget : public xercesc::XMLFormatTarget
{
public:
	CStdOutFormatTarget() = default;
	~CStdOutFormatTarget() = default;

	void writeChars( const XMLByte* const toWrite,
		const XMLSize_t count,
		xercesc::XMLFormatter* const formatter ) override;

	void flush() override;
};
}

#endif //COMMON_XML_CSTDOUTFORMATTARGET_H
