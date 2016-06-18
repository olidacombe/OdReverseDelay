/*
  ==============================================================================

    AudioParameterAtomicFloat.h
    Created: 17 Jun 2016 8:07:33pm
    Author:  Oli

  ==============================================================================
*/

#ifndef AUDIOPARAMETERATOMICFLOAT_H_INCLUDED
#define AUDIOPARAMETERATOMICFLOAT_H_INCLUDED


class AudioParameterAtomicFloat : public AudioParameterFloat
{
public:
    AudioParameterAtomicFloat (String parameterID, String name,
         NormalisableRange<float> normalisableRange,
         float defaultValue) : AudioParameterFloat (parameterID, name,
         normalisableRange,
         defaultValue) {}
    AudioParameterAtomicFloat (String parameterID, String name,
         float minValue,
         float maxValue,
         float defaultValue) : AudioParameterFloat (parameterID, name, minValue, maxValue, defaultValue) {}

    float get() const noexcept { return value.load(); }
    operator float() const noexcept { return value.load(); }
    //AudioParameterFloat& operator = (float newValue) {  }
private:
    std::atomic<float> value;
};



#endif  // AUDIOPARAMETERATOMICFLOAT_H_INCLUDED
