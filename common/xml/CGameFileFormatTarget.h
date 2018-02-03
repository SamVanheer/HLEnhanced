#ifndef COMMON_XML_CGAMEFILEFORMATTARGET_H
#define COMMON_XML_CGAMEFILEFORMATTARGET_H

#include <xercesc/framework/XMLFormatter.hpp>

#include "FileSystem.h"

namespace xml
{
/**
*	@brief A format target that can output to a file handle managed by IFileSystem
*/
class CGameFileFormatTarget : public xercesc::XMLFormatTarget
{
public:
	/**
	*	@brief Opens the given file and prepares for writing
	*	@param pFileSystem The file system used to manage the file
	*	@param pszFilename Name of the file to write to
	*	@param pszPathID Optional. Path id to write to, or null for default
	*	@throws xercesc::IOException If the file could not be opened
	*/
	CGameFileFormatTarget( IFileSystem* pFileSystem, const char* pszFilename, const char* pszPathID = nullptr );

	/**
	*	@brief Opens the given file and prepares for writing
	*	@param pFileSystem The file system used to manage the file
	*	@param hFile Handle of the file to write to
	*	@param bAdoptHandle Optional. Whether to adopt the file handle and assume responsibility for closing it upon completion
	*	@throws xercesc::IOException If the file handle is invalid
	*/
	CGameFileFormatTarget( IFileSystem* pFileSystem, FileHandle_t hFile, const bool bAdoptHandle = false );

	~CGameFileFormatTarget();

	void writeChars( const XMLByte* const toWrite, const XMLSize_t count, xercesc::XMLFormatter* const formatter ) override;

private:
	IFileSystem* m_pFileSystem;
	FileHandle_t m_hFile = FILESYSTEM_INVALID_HANDLE;
	bool m_bOwnHandle = false;
};
}

#endif //COMMON_XML_CGAMEFILEFORMATTARGET_H
