#include <cstdio>

#include "CStdOutFormatTarget.h"

namespace xml
{
void CStdOutFormatTarget::writeChars( const XMLByte* const toWrite, const XMLSize_t count, xercesc::XMLFormatter* const )
{
	XMLSize_t written = fwrite( toWrite, sizeof( XMLByte ), count, stdout );
	if( written != count )
		ThrowXML( xercesc::XMLPlatformUtilsException, xercesc::XMLExcepts::File_CouldNotWriteToFile );
	fflush( stdout );
}

void CStdOutFormatTarget::flush()
{
	fflush( stdout );
}
}
