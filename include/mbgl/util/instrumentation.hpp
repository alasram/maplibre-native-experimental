#pragma once

#include <tracy/Tracy.hpp>

#define MLB_TRACE_FUNC() ZoneScoped
#define MLB_TRACE_ZONE(label) ZoneScopedN(#label)
#define MLB_END_FRAME() FrameMark
