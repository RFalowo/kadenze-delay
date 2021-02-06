/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class WaveFormCmp;//  forward declaration

//==============================================================================
/**
*/
class KadenzeChorusFlangerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    KadenzeChorusFlangerAudioProcessorEditor (KadenzeChorusFlangerAudioProcessor&);
    ~KadenzeChorusFlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    KadenzeChorusFlangerAudioProcessor& audioProcessor;
    juce::Slider mDrySlider;
    juce::Label mDryLabel;

    juce::Slider mWetSlider;
    juce::Label mWetLabel;

    juce::Slider mFeedbackSlider;
    juce::Label mFeedbackLabel;

    juce::Slider mDepthSlider;
    juce::Label mDepthLabel;

    juce::Slider mRateSlider;
    juce::Label mRatelabel;

    juce::Slider mPhaseOffsetSlider;
    juce::Label mPhaseOffestLabel;

    juce::ComboBox mTypeCmp;

    juce::Path getWaveformPath(const juce::Rectangle<int>& rect);

    juce::AudioBuffer<float>& mCircularBufferRef;

    std::unique_ptr<WaveFormCmp> mWaveFormComponent; // How does this work?
    juce::Slider mWaveformZoomSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KadenzeChorusFlangerAudioProcessorEditor)
};
