/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessorEditor::KadenzeChorusFlangerAudioProcessorEditor (KadenzeChorusFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 800);

    auto& params = processor.getParameters();


    const auto createSlider = [&params, this](int paramIndex, juce::Slider& slider, int position){



        auto param = (juce::AudioParameterFloat*)params.getUnchecked(paramIndex);

        slider.setBounds(position,0,200,200);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setRange(param->range.start, param->range.end);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 200,32);
        slider.setValue(*param);
        addAndMakeVisible(slider);

        slider.onValueChange = [this, param, &slider] {
            *param = slider.getValue();};

        slider.onDragStart = [param] {param->beginChangeGesture();};
        slider.onDragEnd = [param] {param->endChangeGesture();};

    };

    createSlider(0,mDelayTimeSlider,0);
    createSlider(1,mDryWetSlider,205);
    createSlider (2,mFeedbackSlider,410);
}

KadenzeChorusFlangerAudioProcessorEditor::~KadenzeChorusFlangerAudioProcessorEditor()
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void KadenzeChorusFlangerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
