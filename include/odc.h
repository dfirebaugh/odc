#ifndef ODC_H
#define ODC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ODC_API __declspec(dllexport)
#else
#define ODC_API __attribute__((visibility("default")))
#endif
#include "odc_audio.h"
#include "odc_debug.h"
#include "odc_engine.h"
#include "odc_font.h"
#include "odc_input.h"
#include "odc_note_parser.h"
#include "odc_oscillator.h"
#include "odc_renderer.h"
#include "odc_shader.h"
#ifdef __cplusplus
}
#endif

#endif // ODC_H
