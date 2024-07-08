//
// Created by Tomasz Rudzki on 02/07/2024.
//

#include "libiamfbr.h"

#ifndef IAMFBR_IAMFBR_WRAPPER_H
    #define IAMFBR_IAMFBR_WRAPPER_H

#endif //IAMFBR_IAMFBR_WRAPPER_H

class iamfbrWrapper
{
public:
    iamfbrWrapper()
    {
    }

    void initialize()
    {
        libiamfbr_ = std::make_unique<iamfbr::libiamfbr> (
            bs, fs, operational_ambisonic_order, requested_filter_length);
    }

    void deinitialize()
    {
        libiamfbr_.reset();
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        // update global variables
        fs = static_cast<int> (sampleRate);
        bs = static_cast<size_t> (samplesPerBlock);
        operational_ambisonic_order = 7;
        requested_filter_length = 0;

        initialize();
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        // get the number of channels
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int number_of_ears = 2;

        // declare int16_t buffers
        std::vector<int16_t> input_buffer_int16(buffer.getNumSamples() * buffer.getNumChannels());
        std::vector<int16_t> output_buffer_int16(buffer.getNumSamples() * number_of_ears);

        // convert float to int16_t
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* source = buffer.getReadPointer(channel);

            // Assuming each int16_t sample occupies 2 bytes
            juce::AudioDataConverters::convertFloatToInt16LE(source, input_buffer_int16.data() + channel * numSamples, numSamples, 2);
        }

        // process
        libiamfbr_->process_planar(input_buffer_int16, output_buffer_int16);

//        // copy input to the output
//        for (size_t i = 0; i < output_buffer_int16.size(); i++)
//        {
//            output_buffer_int16[i] = input_buffer_int16[i];
//        }

        // convert int16_t to float
        for (int channel = 0; channel < number_of_ears; ++channel)
        {
            float* dest = buffer.getWritePointer(channel);

            // Assuming each int16_t sample occupies 2 bytes
            juce::AudioDataConverters::convertInt16LEToFloat(output_buffer_int16.data() + channel * numSamples, dest, numSamples, 2);
        }

        // clear remaining channels
        for (auto i = number_of_ears; i < buffer.getNumChannels(); ++i)
            buffer.clear (i, 0, buffer.getNumSamples());
    }

private:
    std::unique_ptr<iamfbr::libiamfbr> libiamfbr_;

    int fs;
    size_t bs;
    int operational_ambisonic_order;
    size_t requested_filter_length;
};
