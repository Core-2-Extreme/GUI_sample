#pragma once

#if __GNUC__
//We don't want to see warnings in 3rd party headers.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif //__GNUC__

#ifdef CITRO3D_BUILD
#error "This header file is only for external users of citro3d."
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c3d/types.h"

#include "c3d/maths.h"
#include "c3d/mtxstack.h"

#include "c3d/uniforms.h"
#include "c3d/attribs.h"
#include "c3d/buffers.h"
#include "c3d/base.h"

#include "c3d/texenv.h"
#include "c3d/effect.h"
#include "c3d/texture.h"
#include "c3d/proctex.h"
#include "c3d/light.h"
#include "c3d/lightlut.h"
#include "c3d/fog.h"

#include "c3d/framebuffer.h"
#include "c3d/renderqueue.h"

#ifdef __cplusplus
}
#endif

#if __GNUC__
#pragma GCC diagnostic pop
#endif //__GNUC__
