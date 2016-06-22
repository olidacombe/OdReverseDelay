/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AudioParameterAtomicFloat.h"


//==============================================================================
OdReverseDelayAudioProcessor::OdReverseDelayAudioProcessor()
    :   ctsDelayParameter(nullptr),
        feedbackParameter(nullptr),
        delayLengthSamples(1),
        delayPosition(0)
{
    addParameter(ctsDelayParameter = new AudioParameterFloat("continuousDelay", "Continuous Delay Time", 0.0f, 1.0f, 0.5f));
    addParameter(feedbackParameter = new AudioParameterFloat("feedback", "Feedback", 0.0f, 1.0f, 0.4f));
}

OdReverseDelayAudioProcessor::~OdReverseDelayAudioProcessor()
{
}

//==============================================================================
const String OdReverseDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OdReverseDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OdReverseDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double OdReverseDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OdReverseDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OdReverseDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OdReverseDelayAudioProcessor::setCurrentProgram (int index)
{
}

const String OdReverseDelayAudioProcessor::getProgramName (int index)
{
    return String();
}

void OdReverseDelayAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void OdReverseDelayAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    sampleSize = samplesPerBlock;
    sampleRate=newSampleRate;
    const int maxDelayBufferSize = sampleRate * maxDelayTimeSeconds;
    
    delayBufferFloat.setSize(2, std::max(maxDelayBufferSize, sampleSize));
    
    /*
    if(delayBufferFloat.getNumSamples() < samplesPerBlock) {
        delayBufferFloat.setSize(2, samplesPerBlock);
    }
    */


}

void OdReverseDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OdReverseDelayAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif


template <typename FloatType>
void OdReverseDelayAudioProcessor::process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    /*
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    */
    // check for a change of delay length - probably do this in gui thread later
    // and just do a quick comparison here with a current ctsDelayParamter->getValue
    normalizedDelayLength = ctsDelayParameter->get();
    int newDelayLengthSamples = normalizedDelayLengthToSamples(normalizedDelayLength);
    if( newDelayLengthSamples != delayLengthSamples ) {
        // do stretch
        
        dillateBuffer(delayBuffer, newDelayLengthSamples);
        
        // check this!
        delayPosition *= newDelayLengthSamples/static_cast<float>(delayLengthSamples);
        delayLengthSamples = newDelayLengthSamples;
    }
    applyDelay(buffer, delayBuffer);
}

template<typename FloatType>
void OdReverseDelayAudioProcessor::dillateBuffer(AudioBuffer<FloatType>& buffer, const int newLength)
{
    /*
    M =
    My initial idea is, when scaling from N+1 samples to M+1 samples, a[] -> b[].
    b[0]=a[0], b[M+1]=a[N+1]
    and for 0<i<=M:
        b[i] =  { (iN % M)a[iN/M] + (M - (iN % M))a[iN/M + 1] } / M
     
        call j = iN/M
             k = j+1
             a[j]=A
             a[j+1]=B
    */
    /*
    if(buffer.getNumSamples() < newLength) {
        buffer.setSize(buffer.getNumChannels(), exp2((int)log2(newLength) + 1), true);
    }
    */
    
    AudioBuffer<FloatType> bufferCopy = AudioBuffer<FloatType>(buffer);
    
    const int M = newLength - 1;
    const int N = delayLengthSamples - 1;
    //const int NoverM = N/M;
    const int numChannels = buffer.getNumChannels();
    
    for(int channel=0; channel<numChannels; channel++) {
        
        const FloatType* const a = bufferCopy.getReadPointer(channel);
        FloatType* const b = buffer.getWritePointer(channel);
    
        int j=0;
        int r=0;
        FloatType A = a[0];
        FloatType B = a[1];
        
        for(int i=1; i<=M; i++) {
            r+=N;
            if(r>=M) {
                j+=r/M;
                r%=M;
                A = a[j];
                B = a[j+1]; // not sure, when 1 != N/M
            }
            int R=M-r;
            b[i] = (R*A + r*B)/M;
        }
    }
    
    
}

template<typename FloatType>
const int OdReverseDelayAudioProcessor::normalizedDelayLengthToSamples(const FloatType &normDelayLength) {
    //const FloatType maxDelaySeconds = 2.0;
    const int retval = maxDelayTimeSeconds * normDelayLength * sampleRate;
    return std::max(retval, sampleSize);
}

template<typename FloatType>
void OdReverseDelayAudioProcessor::applyDelay(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer)
{
    
    const int numSamples = buffer.getNumSamples();
    const float delayLevel = feedbackParameter->get();
    const int initialDelayPos = delayPosition;
    // maybe we should smooth these parameters here
    // 
    
    const int delaySize = delayLengthSamples;
    int delayPos;
    
    for(int channel = 0; channel < getTotalNumInputChannels(); ++channel) {
        FloatType* const channelData = buffer.getWritePointer(channel);
        FloatType* const delayData = delayBuffer.getWritePointer (jmin (channel, delayBuffer.getNumChannels() - 1));
        delayPos = initialDelayPos;
        
        for(int i=0; i<numSamples; ++i) {
            const int delayAntiPos = delaySize - 1 - delayPos;
            //const FloatType in = channelData[i];
            delayData[delayAntiPos] = delayData[delayAntiPos] * delayLevel + channelData[i];
            channelData[i] += delayData[delayPos];
            //delayData[delayAntiPos] = (delayData[delayAntiPos] + channelData[i]) * delayLevel;
            //delayData[delayAntiPos] = (delayData[delayAntiPos] + in) * delayLevel;
            
            if (++delayPos >= delaySize) {
                delayPos = 0;
            }
        }
    }
    
    delayPosition = delayPos;
}

//==============================================================================
bool OdReverseDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* OdReverseDelayAudioProcessor::createEditor()
{
    return new OdReverseDelayAudioProcessorEditor (*this);
}

//==============================================================================
void OdReverseDelayAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OdReverseDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OdReverseDelayAudioProcessor();
}
