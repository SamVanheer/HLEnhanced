#ifndef COMMON_XML_LOGGING_H
#define COMMON_XML_LOGGING_H

#include <spdlog/logger.h>

namespace xml
{
/**
*	@brief The logger used by XML code. All XML processing code should use this
*/
extern std::shared_ptr<spdlog::logger> log;
}

#endif //COMMON_XML_LOGGING_H
