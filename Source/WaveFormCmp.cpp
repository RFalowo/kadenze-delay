/*
  ==============================================================================

    WaveFormCmp.cpp
    Created: 21 Nov 2020 7:24:15pm
    Author:  Remi Falowo

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveFormCmp.h"

//==============================================================================
WaveFormCmp::WaveFormCmp(KadenzeChorusFlangerAudioProcessor& processor):
mProcessor(processor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

startTimerHz(60);
}

WaveFormCmp::~WaveFormCmp()
{
    stopTimer();
}

void WaveFormCmp::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);


    g.drawText (juce::String(mReadPointer), getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text

    float xPosition = juce::jmap(mBufferPosition,0.0f, static_cast<float>(getWidth()));
    g.drawVerticalLine(xPosition,0.0f, static_cast<float>(getHeight()));

    // TODO add amplitude level to draw function

}

void WaveFormCmp::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void WaveFormCmp::updateData() {
    auto& buffer = mProcessor.getAudioBuffer();
    mReadPointer = mProcessor.getReadPointer();
    mBufferPosition = mReadPointer / static_cast<float >(buffer.getNumSamples());

}

void  WaveFormCmp::timerCallback() {
    // get samples from editor
    updateData();
    repaint();
}