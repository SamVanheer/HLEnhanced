#include <xercesc/util/IOException.hpp>

#include "extdll.h"
#include "util.h"

#include "CGameFileFormatTarget.h"

namespace xml
{
CGameFileFormatTarget::CGameFileFormatTarget( IFileSystem* pFileSystem, const char* pszFilename, const char* pszPathID )
	: m_pFileSystem( pFileSystem )
{
	ASSERT( pFileSystem );
	ASSERT( pszFilename );

	m_hFile = pFileSystem->Open( pszFilename, "wb", pszPathID );

	if( FILESYSTEM_INVALID_HANDLE == m_hFile )
	{
		ThrowXML1( xercesc::IOException, xercesc::XMLExcepts::File_CouldNotOpenFile, pszFilename );
	}

	m_bOwnHandle = true;
}

CGameFileFormatTarget::CGameFileFormatTarget( IFileSystem* pFileSystem, FileHandle_t hFile, const bool bAdoptHandle )
	: m_pFileSystem( pFileSystem )
{
	ASSERT( pFileSystem );

	if( FILESYSTEM_INVALID_HANDLE == hFile )
	{
		ThrowXML( xercesc::IOException, xercesc::XMLExcepts::File_CouldNotOpenFile );
	}

	m_hFile = hFile;
	m_bOwnHandle = bAdoptHandle;
}

CGameFileFormatTarget::~CGameFileFormatTarget()
{
	//Close if we own the handle
	if( m_bOwnHandle && FILESYSTEM_INVALID_HANDLE != m_hFile )
	{
		m_pFileSystem->Close( m_hFile );
	}
}

void CGameFileFormatTarget::writeChars( const XMLByte* const toWrite, const XMLSize_t count, xercesc::XMLFormatter* const )
{
	if( static_cast<decltype( count)>( m_pFileSystem->Write( toWrite, count, m_hFile ) ) != count )
	{
		ThrowXML( xercesc::IOException, xercesc::XMLExcepts::File_CouldNotWriteToFile );
	}
}
}
