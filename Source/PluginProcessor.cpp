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
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mCircularBufferWritePointer = 0;
    mCircularBufferReadPointer = 0.0f;
    mTimeParam = new juce::AudioParameterFloat("time","Delay Time",0.01f,MAX_DELAY_TIME,0.5f);
    addParameter(mTimeParam);

    addParameter(mDryWetParam = new juce::AudioParameterFloat("dry/wet", "Dry/Wet", 0.0f, 1.0f, 0.5f ));

    addParameter(mFeedbcakParam = new juce::AudioParameterFloat("fdbck", "Feedback", 0.0f, 0.98f, 0.5f));
    mFeedbackLeft = 0.0f;
    mFeedbackRight = 0.0f;

    mdelayTimesmooth = 0;
}

KadenzeChorusFlangerAudioProcessor::~KadenzeChorusFlangerAudioProcessor()
{

}

//==============================================================================
const juce::String KadenzeChorusFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KadenzeChorusFlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KadenzeChorusFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KadenzeChorusFlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KadenzeChorusFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KadenzeChorusFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KadenzeChorusFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void KadenzeChorusFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    mCircularBuffer.setSize(2, sampleRate * MAX_DELAY_TIME);
    mCircularBuffer.clear();



    mCircularBufferWritePointer = 0;
    mdelayTimesmooth = *mTimeParam;

}

void KadenzeChorusFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KadenzeChorusFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void KadenzeChorusFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.



    for (int i = 0; i < buffer.getNumSamples(); i++){

        mdelayTimesmooth = mdelayTimesmooth - 0.001 * (mdelayTimesmooth - *mTimeParam);

        mCircularBuffer.setSample(0,mCircularBufferWritePointer,buffer.getSample(0,i) + mFeedbackLeft); //Copy values from audio input buffer into circular buffer
        mCircularBuffer.setSample(1,mCircularBufferWritePointer,buffer.getSample(1,i) + mFeedbackRight);

        mDelayTimeInSamples = static_cast<float>(getSampleRate()) * (mdelayTimesmooth); //Convert delay from time to samples and adjust circular read pointer accordingly
        mCircularBufferReadPointer = static_cast<float>(mCircularBufferWritePointer) - mDelayTimeInSamples;

        if (mCircularBufferReadPointer < 0){                               // Loop read pointer index so it is always behind the write pointer by the correct delay
            mCircularBufferReadPointer += mCircularBuffer.getNumSamples();
        }

        int readPointerX  = (int)mCircularBufferReadPointer;
        int readPointerX1 = readPointerX + 1;
        if (readPointerX1 >= mCircularBuffer.getNumSamples()){
            readPointerX1 -= mCircularBuffer.getNumSamples();
        }

        float readPointerFloat = (float)mCircularBufferReadPointer - readPointerX;

       // const int readindex = static_cast<int>(mCircularBufferReadPointer); // Casting to int



        float delay_sample_left = line_interp(mCircularBuffer.getSample(0,readPointerX),mCircularBuffer.getSample(0,readPointerX1),readPointerFloat);
        float delay_sample_right = line_interp(mCircularBuffer.getSample(1,readPointerX),mCircularBuffer.getSample(1,readPointerX1),readPointerFloat);

        mFeedbackLeft = delay_sample_left * *mFeedbcakParam;
        mFeedbackRight = delay_sample_right * *mFeedbcakParam;

        buffer.setSample(0,i,buffer.getSample(0,i) * *mDryWetParam + delay_sample_left * (1 - *mDryWetParam)); // Add delayed samples to output
        buffer.setSample(1,i,buffer.getSample(1,i) * *mDryWetParam + delay_sample_right * (1 - *mDryWetParam));
        mCircularBufferWritePointer++;

        if (mCircularBufferWritePointer >= mCircularBuffer.getNumSamples()){   //Circular buffer
            mCircularBufferWritePointer = 0;
        }

    }
}

//==============================================================================
bool KadenzeChorusFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KadenzeChorusFlangerAudioProcessor::createEditor()
{
    return new KadenzeChorusFlangerAudioProcessorEditor (*this);
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KadenzeChorusFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KadenzeChorusFlangerAudioProcessor();
}

float KadenzeChorusFlangerAudioProcessor::line_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase *sample_x1;
}