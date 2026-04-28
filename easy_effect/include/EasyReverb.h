#pragma once

#include "dsp/delay.h"
#include "dsp/mix-matrix.h"
#include <cstdlib>

using Delay = signalsmith::delay::Delay<double>;

inline double randomInRange(double low, double high) {
  // There are better randoms than this, and you should use them instead 😛
  double unitRand = rand() / double(RAND_MAX);
  return low + unitRand * (high - low);
}

template <int channels = 8>
struct MultiChannelMixedFeedback {
  using Array = std::array<double, channels>;
  double delayMs = 50;
  double decayGain = 0.85;

  std::array<int, channels> delaySamples;
  std::array<Delay, channels> delays;

  void configure(double sampleRate) {
    double delaySamplesBase = delayMs * 0.001 * sampleRate;
    for (int c = 0; c < channels; ++c) {
      double r = c * 1.0 / channels;
      delaySamples[c] = std::pow(2, r) * delaySamplesBase;
      delays[c].resize(delaySamples[c] + 1);
      delays[c].reset();
    }
  }

  Array process(Array input) {
    Array delayed;
    for (int c = 0; c < channels; ++c) {
      delayed[c] = delays[c].read(delaySamples[c]);
    }

    // Mix using a Householder matrix
    Array mixed = delayed;
    Householder<double, channels>::inPlace(mixed.data());

    for (int c = 0; c < channels; ++c) {
      double sum = input[c] + mixed[c] * decayGain;
      delays[c].write(sum);
    }

    return delayed;
  }
};

template <int channels = 8>
struct DiffusionStep {
  using Array = std::array<double, channels>;
  double delayMsRange = 20;

  std::array<int, channels> delaySamples;
  std::array<Delay, channels> delays;
  std::array<bool, channels> flipPolarity;

  void configure(double sampleRate) {
    double delaySamplesRange = delayMsRange * 0.001 * sampleRate;
    for (int c = 0; c < channels; ++c) {
      double rangeLow = delaySamplesRange * c / channels;
      double rangeHigh = delaySamplesRange * (c + 1) / channels;
      delaySamples[c] = randomInRange(rangeLow, rangeHigh);
      delays[c].resize(delaySamples[c] + 1);
      delays[c].reset();
      flipPolarity[c] = rand() % 2;
    }
  }

  Array process(Array input) {
    // Delay
    Array delayed;
    for (int c = 0; c < channels; ++c) {
      delays[c].write(input[c]);
      delayed[c] = delays[c].read(delaySamples[c]);
    }

    // Mix with a Hadamard matrix
    Array mixed = delayed;
    Hadamard<double, channels>::inPlace(mixed.data());

    // Flip some polarities
    for (int c = 0; c < channels; ++c) {
      if (flipPolarity[c])
        mixed[c] *= -1;
    }

    return mixed;
  }
};