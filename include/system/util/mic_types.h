#if !defined(DEF_MIC_TYPES_H)
#define DEF_MIC_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_ENABLE_MIC_API				/*(bool)(*/1/*)*/	//Enable mic API.

enum Mic_sample_rate
{
	MIC_SAMPLE_RATE_INVALID = -1,

	MIC_SAMPLE_RATE_8182HZ,
	MIC_SAMPLE_RATE_10909HZ,
	MIC_SAMPLE_RATE_16364HZ,
	MIC_SAMPLE_RATE_32728HZ,

	MIC_SAMPLE_RATE_MAX,
};

#endif //!defined(DEF_MIC_TYPES_H)
