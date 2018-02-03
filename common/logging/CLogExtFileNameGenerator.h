#ifndef COMMON_LOGGING_CLOGEXTFILENAMEGENERATOR_H
#define COMMON_LOGGING_CLOGEXTFILENAMEGENERATOR_H

#include <spdlog/common.h>

namespace logging
{
/**
*	@brief A file name generator that takes the generated name from another generator and appends ".log" to it.
*	TODO: this can probably be generalized by providing the extension as another template parameter
*/
template<typename BASE_GENERATOR>
struct CLogExtFileNameGenerator
{
	using base_generator_t = BASE_GENERATOR;

	static spdlog::filename_t calc_filename( const spdlog::filename_t& basename )
	{
		return BASE_GENERATOR::calc_filename( basename ) + ".log";
	}
};
}

#endif //COMMON_LOGGING_CLOGEXTFILENAMEGENERATOR_H
