#include <JuceHeader.h>
#include "Plugin.h"

AudioPlugin::AudioPlugin()
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo())
                                      .withOutput("Output", AudioChannelSet::stereo()))
{
    addParameter(delay_gain = new AudioParameterFloat({"delay_gain", 1}, "Delay Gain", 0.0f, 1.0f, 0.5f));
    addParameter(delay_time = new AudioParameterFloat({"delay_time", 1}, "Delay Time", 1.0f, 1000.0f, 1.0f));
}

void AudioPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    play_head = 0;
    for (size_t i = 0; i < 192000; i++)
    {
        delay_buffer[i] = 0.0f;
    }
}

void AudioPlugin::releaseResources()
{

}

template <typename T>
void AudioPlugin::processSamples(AudioBuffer<T> &audioBuffer, MidiBuffer &midiBuffer)
{
    auto reader = audioBuffer.getArrayOfReadPointers();
    auto writer = audioBuffer.getArrayOfWritePointers();

    for (size_t sample = 0; sample < (size_t)audioBuffer.getNumSamples(); sample++)
    {
        float sample_dry = 0.0;
        for (size_t channel = 0; channel < (size_t)audioBuffer.getNumChannels(); channel++)
        {
            sample_dry += (float)reader[channel][sample];
        }
        
        float sample_wet = delay_buffer[play_head];
        delay_buffer[play_head] = sample_dry + (sample_wet * delay_gain->get());

        play_head = (play_head + 1) % (size_t)(delay_time->get() * (getSampleRate() / 1000.0));

        for (size_t channel = 0; channel < (size_t)audioBuffer.getNumChannels(); channel++)
        {
            writer[channel][sample] = sample_dry + (sample_wet * delay_gain->get());
        }
        
    }
    
}

void AudioPlugin::processBlock(AudioBuffer<float> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<float>(audioBuffer, midiBuffer);
}

void AudioPlugin::processBlock(AudioBuffer<double> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<double>(audioBuffer, midiBuffer);
}

void AudioPlugin::getStateInformation(MemoryBlock &destData)
{
    MemoryOutputStream *memoryOutputStream = new MemoryOutputStream(destData, true);
    memoryOutputStream->writeFloat(*delay_gain);
    memoryOutputStream->writeFloat(*delay_time);
    delete memoryOutputStream;
}

void AudioPlugin::setStateInformation(const void *data, int sizeInBytes)
{
    MemoryInputStream *memoryInputStream = new MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false);
    delay_gain->setValueNotifyingHost(memoryInputStream->readFloat());
    delay_time->setValueNotifyingHost(memoryInputStream->readFloat());
    delete memoryInputStream;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPlugin();
}