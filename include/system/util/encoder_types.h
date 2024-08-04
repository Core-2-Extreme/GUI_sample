#if !defined(DEF_ENCODER_TYPES_H)
#define DEF_ENCODER_TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_ENABLE_IMAGE_ENCODER_API			/*(bool)(*/1/*)*/	//Enable image encoder API. This will use stb_image functions.
#define DEF_ENABLE_VIDEO_AUDIO_ENCODER_API		/*(bool)(*/1/*)*/	//Enable video/audio encoder API. This will use ffmpeg functions.

#define DEF_ENCODER_MAX_SESSIONS				(uint8_t)(2)

#endif //!defined(DEF_ENCODER_TYPES_H)
