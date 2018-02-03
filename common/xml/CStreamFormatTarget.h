#ifndef COMMON_XML_CSTREAMFORMATTARGET_H
#define COMMON_XML_CSTREAMFORMATTARGET_H

#include <ostream>

#include <xercesc/framework/XMLFormatter.hpp>

namespace xml
{
/**
*	@brief Format target that enables output to a stream
*/
class CStreamFormatTarget : public xercesc::XMLFormatTarget
{
public:
	CStreamFormatTarget( std::ostream& stream )
		: m_Stream( stream )
	{
	}
	~CStreamFormatTarget() = default;

	void writeChars( const XMLByte* const toWrite,
		const XMLSize_t count,
		xercesc::XMLFormatter* const formatter ) override;

	void flush() override;

private:
	std::ostream& m_Stream;
};

void CStreamFormatTarget::writeChars( const XMLByte* const toWrite, const XMLSize_t count, xercesc::XMLFormatter* const )
{
	m_Stream.write( reinterpret_cast<const char*>( toWrite ), static_cast<std::streamsize>( count ) );

	if( !m_Stream.good() )
		ThrowXML( xercesc::XMLPlatformUtilsException, xercesc::XMLExcepts::File_CouldNotWriteToFile );

	m_Stream.flush();
}

void CStreamFormatTarget::flush()
{
	m_Stream.flush();
}
}

#endif //COMMON_XML_CSTREAMFORMATTARGET_H
