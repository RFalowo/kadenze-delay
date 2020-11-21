/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MAX_DELAY_TIME 2
//==============================================================================
/**
*/
class KadenzeChorusFlangerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KadenzeChorusFlangerAudioProcessor();
    ~KadenzeChorusFlangerAudioProcessor() override;

    enum class Type {
        Chorus,
        Flanger,
    };

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioBuffer<float>& getAudioBuffer ();
    float  getReadPointer ();


private:

    static constexpr int kNumChannels = 2;
    static constexpr float kMaxDelayTime = 1.0f;
    static constexpr int kSafetyBlockSizeScalar = 2;//?

    float mLFOPhase;

    float line_interp(float sample_x, float sample_x1, float inPhase);
    float mdelayTimesmooth;

    int mCircularBufferWritePointer;
    float mDelayTimeInSamples;



    juce::AudioParameterFloat* mDepthParam;
    juce::AudioParameterFloat* mRateParam;
    juce::AudioParameterFloat* mPhaseOffsetParam;

    juce::AudioParameterInt* mTypeParam;

    float mCircularBufferReadPointer;
    juce::AudioBuffer<float> mCircularBuffer;
    juce::AudioBuffer<float> mFeedback;

    float mFeedbackLeft;
    float mFeedbackRight;



    juce::AudioParameterFloat* mDryParam;
    juce::AudioParameterFloat* mWetParam;
    juce::AudioParameterFloat* mFeedbackParam;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KadenzeChorusFlangerAudioProcessor)
};
