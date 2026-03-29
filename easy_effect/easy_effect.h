/*
/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            easy_effect
   vendor:        WolfSound
   version:       0.1.0
   name:          Audio Plugin
   description:   Plugin core
   dependencies:  juce_audio_utils

   website:       https://thewolfsound.com
   license:       Unlicense

END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "include/PluginProcessor.h"
#include "include/PluginEditor.h"

// #include all additional header files below
#include "include/EasyReverb.h"
#include "include/dsp/common.h"
#include "include/dsp/delay.h"
#include "include/dsp/mix-matrix.h"
#include "include/dsp/windows.h"
#include "include/dsp/fft.h"
#include "include/dsp/perf.h"


