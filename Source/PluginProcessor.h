/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "ThreadSafeLinearSmoothedValue.h"
#include "AudioParameterAtomicFloat.h"


//==============================================================================
/**
*/
class OdReverseDelayAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    OdReverseDelayAudioProcessor();
    ~OdReverseDelayAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (! isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferFloat);
    }

    void processBlock (AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override
    {
        jassert (isUsingDoublePrecision());
        process (buffer, midiMessages, delayBufferDouble);
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioParameterFloat *ctsDelayParameter, *feedbackParameter;
    
    
private:
    template <typename FloatType>
    void process(AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer);
    template <typename FloatType>
    void applyDelay(AudioBuffer<FloatType>&, AudioBuffer<FloatType>& delayBuffer);
    template <typename FloatType>
    const int normalizedDelayLengthToSamples(const FloatType& delayLength);
    
    ThreadSafeLinearSmoothedValue<float> normalizedDelayLengthFloat;
    ThreadSafeLinearSmoothedValue<float> normalizedDelayLengthDouble;
    ThreadSafeLinearSmoothedValue<float> feedbackParameterFloat;
    ThreadSafeLinearSmoothedValue<float> feedbackParameterDouble;
    
    
    AudioBuffer<float> delayBufferFloat;
    AudioBuffer<double> delayBufferDouble;
    
    double sampleRate;
    float normalizedDelayLength;
    int delayLengthSamples;
    int delayPosition;
    const int maxDelayBufferSize = 131072;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OdReverseDelayAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
