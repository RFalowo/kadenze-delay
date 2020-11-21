/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessor::KadenzeChorusFlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
  mCircularBufferWritePointer = 0;
  mCircularBufferReadPointer = 0.0f;

    addParameter(mDepthParam =
      new juce::AudioParameterFloat("mDepthParam", "Depth", 0.0f, 1.0f, 0.5f));
    addParameter(mRateParam =
      new juce::AudioParameterFloat("mRateParam", "Rate", 0.1f, 20.0f, 10.0f));
    addParameter(mPhaseOffsetParam = new juce::AudioParameterFloat(
      "mPhaseOffsetParam", "Phase Offest", 0.0f, 1.0f, 0.0f));
    addParameter(mTypeParam = new juce::AudioParameterInt("typeParam", "Effect Type", 0, 1, 0));

    addParameter(mDryParam = new juce::AudioParameterFloat(
                   "dry", "Dry Amount", 0.0f, 1.0f, 1.0f));

    addParameter(mWetParam = new juce::AudioParameterFloat(
            "wet", "Wet Amount", 0.0f, 1.0f, 0.5f));

    addParameter(mFeedbackParam = new juce::AudioParameterFloat(
                   "fdbck", "Feedback", 0.0f, 0.98f, 0.5f));
  mFeedbackLeft = 0.0f;
  mFeedbackRight = 0.0f;

  mdelayTimesmooth = 0;

  mLFOPhase = 0;
}

KadenzeChorusFlangerAudioProcessor::~KadenzeChorusFlangerAudioProcessor() {}

//==============================================================================
const juce::String KadenzeChorusFlangerAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool KadenzeChorusFlangerAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool KadenzeChorusFlangerAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool KadenzeChorusFlangerAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double KadenzeChorusFlangerAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int KadenzeChorusFlangerAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int KadenzeChorusFlangerAudioProcessor::getCurrentProgram() { return 0; }

void KadenzeChorusFlangerAudioProcessor::setCurrentProgram(int index) {}

const juce::String
KadenzeChorusFlangerAudioProcessor::getProgramName(int index) {
  return {};
}

void KadenzeChorusFlangerAudioProcessor::changeProgramName(
    int index, const juce::String &newName) {}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::prepareToPlay(double sampleRate,
                                                       int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..

  mCircularBuffer.setSize(kNumChannels, static_cast<int>(sampleRate * MAX_DELAY_TIME));
  mCircularBuffer.clear();

  mFeedback.setSize(kNumChannels,sampleRate * kSafetyBlockSizeScalar);

  mCircularBufferWritePointer = 0;
  mdelayTimesmooth = 1.0f;
  mLFOPhase = 0.0f;
}

void KadenzeChorusFlangerAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KadenzeChorusFlangerAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void KadenzeChorusFlangerAudioProcessor::processBlock(
    juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    // helper variables
    const auto sampleRate = static_cast<float>(getSampleRate());
    const auto circularBufferNumSamples = static_cast<float>(mCircularBuffer.getNumSamples());

    for (int b = 0; b < buffer.getNumSamples(); b++) {

        float lfoOut = sin(2 * M_PI * mLFOPhase); // sine wave with period of 1
        mLFOPhase += *mRateParam / getSampleRate();
        if (mLFOPhase > 1) {//wrap lfo phase
            mLFOPhase -= 1;
        }


        float lfoMin = 0.0f;
        float lfoMax = 0.0f;
        switch (static_cast<Type>(mTypeParam->get())) {
            case Type::Chorus:
                lfoMin = 0.005f;
                lfoMax = 0.030f;
                break;

            case Type::Flanger:
                lfoMin = 0.001f;
                lfoMax = 0.005f;
                break;

            default:
                jassertfalse;
        }
        // channel loop is lower risk because the values are copied and cannot change
        const auto baseLfoPhase = mLFOPhase;
        const auto phaseOffset = mPhaseOffsetParam->get();
        const auto depth = mDepthParam->get();
        const auto delayWriteIndex = mCircularBufferWritePointer;
        const auto dryAmount = mDryParam->get();
        const auto wetAmount = mWetParam->get();
        const auto feedbackAmount = mFeedbackParam->get();

        for (int c = 0; c < buffer.getNumChannels(); ++c) {// why preincrement
            //put signal into circular buffer
            mCircularBuffer.setSample(c, delayWriteIndex, buffer.getSample(c, b));
            mCircularBuffer.addSample(c, delayWriteIndex, mFeedback.getSample(c, b));

            // calculate the LFO phase for this channel, with increasing offset per channel
            float lfoPhase = baseLfoPhase + (static_cast<float>(c) * phaseOffset);

            // ensure our LFO phase is between 0.0f and 1.0f
            if (lfoPhase > 1.0f) {
                lfoPhase -= 1.0f;
            }

            // map LFO output to delay times
            const float lfoOut = sin(juce::MathConstants<float>::twoPi * lfoPhase) * depth;
            const float lfoOutMapped = juce::jmap(lfoOut, -1.0f, 1.0f, lfoMin, lfoMax);

            // calculate delay lengths in samples
            const float delayTimeInSamples = sampleRate * lfoOutMapped;
            auto delayReadIndex = static_cast<float>(delayWriteIndex) - delayTimeInSamples;
            if (delayReadIndex < 0.0f) {
                delayReadIndex += circularBufferNumSamples;
            }

            // not using else if, as if delayReadIndex is just below 0.0f
            // the delayReadIndex + circularBufferNumSamples == circularBufferNumSamples
            if (delayReadIndex >= circularBufferNumSamples) {
                delayReadIndex -= circularBufferNumSamples;
            }

            jassert(delayReadIndex >= 0.0f);
            jassert(delayReadIndex < circularBufferNumSamples);

            // calculate read indices and interpolation
            const auto readIndex1 = static_cast<int>(delayReadIndex);
            const auto readIndex2 = readIndex1 == mCircularBuffer.getNumSamples() - 1 ? 0 : readIndex1 + 1;

            const auto startSample = mCircularBuffer.getSample(c, readIndex1);
            const auto endSample = mCircularBuffer.getSample(c, readIndex2);

            const auto readPhase = delayReadIndex - static_cast<float>(readIndex1);
            const auto delayedSample = line_interp(startSample, endSample, readPhase);

            // add delayed signal to feedback buffer
            mFeedback.setSample(c, b, delayedSample * feedbackAmount);

            // add dry and wet signals output
            const auto dry = buffer.getSample(c, b) * dryAmount;
            const auto wet = delayedSample * wetAmount;
            buffer.setSample(c, b, dry + wet);
            // TODO write to monitoring buffer
            // mOutputbuffer.setsample(c, b, dry + wet);

        }

        // update our LFO phase
        mLFOPhase += *mRateParam / sampleRate;
        if (mLFOPhase > 1.0f) {
            mLFOPhase -= 1.0f;
        }

        // update our delay write index
        mCircularBufferWritePointer++;
        if (mCircularBufferWritePointer >= mCircularBuffer.getNumSamples()) {
            mCircularBufferWritePointer = 0;
        }
    }
}

//==============================================================================
bool KadenzeChorusFlangerAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *KadenzeChorusFlangerAudioProcessor::createEditor() {
  return new KadenzeChorusFlangerAudioProcessorEditor(*this);
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
///////////////////////////////////////////////
//Wasn't sure about all the xml code in this section
}

void KadenzeChorusFlangerAudioProcessor::setStateInformation(const void *data,
                                                             int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new KadenzeChorusFlangerAudioProcessor();
}

float KadenzeChorusFlangerAudioProcessor::line_interp(float sample_x, float sample_x1, float inPhase) {
  //return (1 - inPhase) * sample_x + inPhase * sample_x1; //////////////////////check variables line up
    return (sample_x * (1.0f - inPhase)) + (sample_x * inPhase);
}

juce::AudioBuffer<float>& KadenzeChorusFlangerAudioProcessor::getAudioBuffer() {
    return mCircularBuffer;
}

float KadenzeChorusFlangerAudioProcessor::getReadPointer() {
    DBG(mCircularBufferReadPointer);
    return mCircularBufferReadPointer;

}