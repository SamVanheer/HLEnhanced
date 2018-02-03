#ifndef COMMON_XML_CCONSOLEFORMATTARGET_H
#define COMMON_XML_CCONSOLEFORMATTARGET_H

#include <xercesc/framework/XMLFormatter.hpp>

namespace xml
{
/**
*	@brief Format target that enables output to the console
*/
class CConsoleFormatTarget : public xercesc::XMLFormatTarget
{
public:
	/**
	*	@param bDebug Whether to output using developer print
	*/
	CConsoleFormatTarget( const bool bDebug = false );
	~CConsoleFormatTarget() = default;

	void writeChars( const XMLByte* const toWrite,
		const XMLSize_t count,
		xercesc::XMLFormatter* const formatter ) override;

private:
	bool m_bDebug = false;
};
}

#endif //COMMON_XML_CCONSOLEFORMATTARGET_H
