/*
  ==============================================================================

    WaveFormCmp.h
    Created: 21 Nov 2020 7:24:15pm
    Author:  Remi Falowo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class WaveFormCmp  : public juce::Component, private juce::Timer
{
public:
    WaveFormCmp();
    ~WaveFormCmp() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFormCmp)
};
