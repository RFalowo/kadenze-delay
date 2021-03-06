/*
  ==============================================================================

    WaveFormCmp.h
    Created: 21 Nov 2020 7:24:15pm
    Author:  Remi Falowo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class WaveFormCmp  : public juce::Component, private juce::Timer
{
public:
    WaveFormCmp(KadenzeChorusFlangerAudioProcessor& processor);
    ~WaveFormCmp() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    float getZoom();
    void setZoom(float zoom);
private:

    juce::Path mInputWaveformPath;
    float mZoomValue = 0.5f;

    KadenzeChorusFlangerAudioProcessor& mProcessor;
    void updateData();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFormCmp)

};
