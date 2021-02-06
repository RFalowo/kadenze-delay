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

    float xPosition = juce::jmap(mBufferPosition,0.0f, static_cast<float>(getWidth())); // getwidth of what?
    //g.drawVerticalLine(xPosition,0.0f, static_cast<float>(getHeight()));

    float yPosition = juce::jmap(mAmpValue,-1.0f,1.0f,static_cast<float>(getHeight()), 0.0f);
    //g.drawHorizontalLine(yPosition,0.0f,static_cast<float>(getWidth()));
    // TODO add amplitude level to draw function

    juce::Path path;
    path.startNewSubPath(0.0f, getHeight()/2.0f);

    auto& buffer = mProcessor.getAudioBuffer();
    const int width = 4000;
    jassert(width<buffer.getNumSamples());
    int start = mReadPointer - width;
    if (mReadPointer < width){
        start += buffer.getNumSamples();
    }
    //const int start2 = mReadPointer - width + (mReadPointer < width ? buffer.getNumSamples() : 0); same as above

    float stepSize = static_cast<float >(width) / static_cast<float >(getWidth());
    for (int i = 0; i < getWidth() ; ++i) {
        int offset = i * stepSize;
        float AmpValue = buffer.getSample(0, (start + offset) % buffer.getNumSamples());//get sample value
        float yPosition = juce::jmap(AmpValue,-1.0f,1.0f,static_cast<float>(getHeight()), 0.0f); //map to component height
        path.lineTo(i,yPosition);

    }

    g.strokePath(path, juce::PathStrokeType(1.0f));
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
    mAmpValue = buffer.getSample(0, static_cast<int>(mReadPointer));


}

void  WaveFormCmp::timerCallback() {
    // get samples from editor
    updateData();
    repaint();
}