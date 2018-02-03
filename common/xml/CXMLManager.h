#ifndef COMMON_XML_CXMLMANAGER_H
#define COMMON_XML_CXMLMANAGER_H

#include <memory>
#include <ostream>

#include <spdlog/logger.h>

#include <xercesc/util/XercesDefs.hpp>

#include "FileSystem.h"

#include "Logging.h"
#include "XMLUtils.h"

XERCES_CPP_NAMESPACE_BEGIN

class DOMDocument;
class DOMImplementation;
class InputSource;
class XercesDOMParser;
class XMLFormatTarget;

XERCES_CPP_NAMESPACE_END

namespace xml
{
class CDOMPrintErrorHandler;
class CErrorHandler;

class CXMLManager final
{
public:
	CXMLManager();
	~CXMLManager();

	bool Initialize();

	void Shutdown();

	/**
	*	@brief Parses the given input source and returns the document if parsing succeeded
	*	@return If parsing succeeded, a DOM document. The caller owns the document and must free it themselves
	*/
	unique_xml_ptr<xercesc::DOMDocument> ParseInputSource( const xercesc::InputSource& source );

	/**
	*	@brief Parses the given buffer and returns the document if parsing succeeded
	*	@param pszBufId Id for this buffer, usually the filename
	*	@param pszBuffer Buffer to parse
	*	@param uiBufferSize Size of the buffer, in bytes
	*	@return If parsing succeeded, a DOM document. The caller owns the document and must free it themselves
	*/
	unique_xml_ptr<xercesc::DOMDocument> ParseBuffer( const char* pszBufId, const char* pszBuffer, const size_t uiBufferSize );

	/**
	*	@brief Parses the data from the given file handle and returns the document if parsing succeeded
	*	Will seek to head and parse the entire file, the seek position is undefined after this call
	*	@param pszBufId Id for this buffer, usually the filename
	*	@param hFile Handle to the file to parse
	*	@return If parsing succeeded, a DOM document. The caller owns the document and must free it themselves
	*/
	unique_xml_ptr<xercesc::DOMDocument> ParseFile( const char* pszBufId, FileHandle_t hFile );

	/**
	*	@brief Parses the given file and returns the document if parsing succeeded
	*	@param pszFilename Name of the file to parse
	*	@param pszPathID Optional. Path to search in, or null for all paths
	*	@return If parsing succeeded, a DOM document. The caller owns the document and must free it themselves
	*/
	unique_xml_ptr<xercesc::DOMDocument> ParseFile( const char* pszFilename, const char* pszPathID = nullptr );

	/**
	*	@brief Writes a DOM document to a form target as an XML text file
	*	@return true on success, false on failure
	*/
	bool WriteToFormTarget( const xercesc::DOMDocument& document, xercesc::XMLFormatTarget& formTarget );

	/**
	*	@brief Writes a DOM document to standard output
	*	@return true on success, false on failure
	*/
	bool WriteToStdOut( const xercesc::DOMDocument& document );

	/**
	*	@brief Writes a DOM document to the console
	*	@param bDebug Whether to output using developer print
	*	@return true on success, false on failure
	*/
	bool WriteToConsole( const xercesc::DOMDocument& document, const bool bDebug = false );

	/**
	*	@brief Writes a DOM document to a given stream
	*	@return true on success, false on failure
	*/
	bool WriteToStream( const xercesc::DOMDocument& document, std::ostream& stream );

	/**
	*	@brief Writes a DOM document to a given file
	*	@param pszPathID Optional. Path id to write to, or null for default
	*	@return true on success, false on failure
	*/
	bool WriteToFile( const xercesc::DOMDocument& document, const char* pszFilename, const char* pszPathID = nullptr );

	/**
	*	@brief Writes a DOM document to a given file handle
	*	@param bAdoptHandle Optional. Whether to adopt the file handle and assume responsibility for closing it upon completion
	*	@return true on success, false on failure
	*/
	bool WriteToFile( const xercesc::DOMDocument& document, FileHandle_t hFile, const bool bAdoptHandle = false );

private:
	bool m_bInitialized = false;

	//TODO: Currently one instance, if threaded loading is required this needs to be per-thread or use an async queue - Solokiller
	std::unique_ptr<xercesc::XercesDOMParser> m_Parser;

	std::unique_ptr<CErrorHandler> m_ErrorHandler;

	std::unique_ptr<CDOMPrintErrorHandler> m_PrintErrorHandler;

	//For serialization
	xercesc::DOMImplementation* m_pDOMImpl = nullptr;

private:
	CXMLManager( const CXMLManager& ) = delete;
	CXMLManager& operator=( const CXMLManager& ) = delete;
};

/**
*	@brief Gets the XML manager
*/
CXMLManager& XMLManager();
}

#endif //COMMON_XML_CXMLMANAGER_H