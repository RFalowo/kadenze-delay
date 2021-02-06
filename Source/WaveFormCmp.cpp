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

    // clear the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // draw an outline around the component
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);

    // draw waveform
    g.setColour (juce::Colours::white);
    g.strokePath(mInputWaveformPath, juce::PathStrokeType(1.0f));


}

void WaveFormCmp::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void WaveFormCmp::updateData() {
    auto& buffer = mProcessor.getAudioBuffer();
    const int readPointer = mProcessor.getReadPointer();

    mInputWaveformPath.startNewSubPath(0.0f, getHeight() / 2.0f);

    const int width = 4000;
    jassert(width<buffer.getNumSamples());
    int start = readPointer - width;
    if (readPointer < width){
        start += buffer.getNumSamples();
    }
    //const int start2 = mReadPointer - width + (mReadPointer < width ? buffer.getNumSamples() : 0); same as above

    float stepSize = static_cast<float >(width) / static_cast<float >(getWidth());
    for (int i = 0; i < getWidth() ; ++i) {
        int offset = i * stepSize;
        float AmpValue = buffer.getSample(0, (start + offset) % buffer.getNumSamples());//get sample value
        float yPosition = juce::jmap(AmpValue,-1.0f,1.0f,static_cast<float>(getHeight()), 0.0f); //map to component height
        mInputWaveformPath.lineTo(i, yPosition);

    }
}

void  WaveFormCmp::timerCallback() {
    // get samples from editor
    updateData();
    repaint();
}