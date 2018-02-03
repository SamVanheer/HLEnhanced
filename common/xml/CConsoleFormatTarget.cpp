#include "extdll.h"
#include "util.h"

#include "CConsoleFormatTarget.h"

namespace xml
{
CConsoleFormatTarget::CConsoleFormatTarget( const bool bDebug )
	: m_bDebug( bDebug )
{
}

void CConsoleFormatTarget::writeChars( const XMLByte* const toWrite, const XMLSize_t count, xercesc::XMLFormatter* const )
{
	XMLByte buffer[ 8192 ];

	//Write input piece by piece until done

	XMLSize_t nextStart = 0;

	while( nextStart < count )
	{
		const auto writeCount = min( ( count - nextStart ), sizeof( buffer ) - 1 );

		memcpy( buffer, toWrite + nextStart, writeCount );

		buffer[ writeCount ] = '\0';

		nextStart += writeCount;

		if( m_bDebug )
			Con_DPrintf( "%s", buffer );
		else
			Con_Printf( "%s", buffer );
	}
}
}
