//
// Created by Tomasz Rudzki on 09/08/2024.
//

#include "juce_audio_processors/juce_audio_processors.h"

#ifndef IAMFBR_EXTRAS_H
#define IAMFBR_EXTRAS_H

// plugin state stuff (to be replaced with a proper state management system)
enum class InputType { Ambisonics, Loudspeaker_feeds, Individual_sources };

struct source_properties {
  juce::String source_id;
  float gain, azimuth, elevation, distance;
};

struct plugin_state {
  InputType input_type;
  int ambisonic_order;
  int selected_preset_id;

  std::vector<source_properties> sources;
};



#endif  // IAMFBR_EXTRAS_H
