/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "WaveFormCmp.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessorEditor::KadenzeChorusFlangerAudioProcessorEditor (KadenzeChorusFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), mCircularBufferRef(p.getAudioBuffer())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    mWaveFormComponent = std::make_unique<WaveFormCmp>(p);
    addAndMakeVisible(*mWaveFormComponent);


    // get all parameters by ID
    // -----------------------------------------------------

    const auto parameters = processor.getParameters();

    //create null pointers for each parameter
    juce::AudioParameterFloat* feedbackParam = nullptr;
    juce::AudioParameterFloat* dryParam = nullptr;
    juce::AudioParameterFloat* wetParam = nullptr;
    juce::AudioParameterFloat* DepthParam = nullptr;
    juce::AudioParameterFloat* RateParam = nullptr;
    juce::AudioParameterFloat* PhaseOffsetParam = nullptr;
    juce::AudioParameterInt* typeParam = nullptr;

    //loop over all parameters, use ID to store in variables
    for (const auto parameter : parameters) { // modern for loop, first part is item second is array to loop over
        // try casting to audioparam float pointer, if successful check the ID and assign to correct variable
        const auto parameterFloat = dynamic_cast<juce::AudioParameterFloat*>(parameter);
        if (parameterFloat != nullptr) {
            if (parameterFloat->paramID == "fdbck") {
                feedbackParam = parameterFloat;
            } else if (parameterFloat->paramID == "dry") {
                dryParam = parameterFloat;
            } else if (parameterFloat->paramID == "wet") {
                wetParam = parameterFloat;
            } else if (parameterFloat->paramID == "mDepthParam") {
                DepthParam = parameterFloat;
            } else if (parameterFloat->paramID == "mRateParam") {
                RateParam = parameterFloat;
            } else if (parameterFloat->paramID == "mPhaseOffsetParam") {
                PhaseOffsetParam = parameterFloat;
            }
        }
        // assigning int parameters (same as above)
        const auto parameterInt = dynamic_cast<juce::AudioParameterInt*>(parameter);
        if (parameterInt != nullptr) {
            if (parameterInt->paramID == "typeParam") {
                typeParam = parameterInt;
            }
        }

    }
    //checking that all params have been assigned (only works in debug mode)
    jassert(dryParam != nullptr);
    jassert(wetParam != nullptr);
    jassert(feedbackParam != nullptr);
    jassert(DepthParam != nullptr);
    jassert(RateParam != nullptr);
    jassert(PhaseOffsetParam != nullptr);
    jassert(typeParam != nullptr);



    /**
     * Helper function to setup a slider
     *
     * @param param - audio param to assign to slider
     * @param slider - slider component for gui
     * @param position - X position of slider
     */
    const auto createSlider = [this](auto param, juce::Slider& slider,juce::Label& label, int xPosition, int yPosition){

        slider.setBounds(xPosition,yPosition,200,200);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setRange(param->range.start, param->range.end);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 200,32);
        slider.setValue(*param);
        this->addAndMakeVisible(slider);

        slider.onValueChange = [param, &slider] { //lambda functon,
            *param = slider.getValue();};

        slider.onDragStart = [param] {param->beginChangeGesture();};
        slider.onDragEnd = [param] {param->endChangeGesture();};

        label.setBounds(xPosition, slider.getBottom() + 5, slider.getWidth(), 30);
        label.setText(param->getName(100), juce::dontSendNotification);
        addAndMakeVisible(label);


    };

    createSlider(dryParam,mDrySlider,mDryLabel,105,0);
    createSlider(wetParam, mWetSlider,mWetLabel, 310,0);
    createSlider(feedbackParam,mFeedbackSlider,mFeedbackLabel, 515,0);
    createSlider(DepthParam, mDepthSlider,mDepthLabel, 115, 210);
    createSlider(RateParam, mRateSlider, mRatelabel, 320, 210) ;
    createSlider(PhaseOffsetParam, mPhaseOffsetSlider, mPhaseOffestLabel, 525, 210) ;


    mTypeCmp.setBounds(319, 515, 100, 30);

    using Processor = KadenzeChorusFlangerAudioProcessor;
    using Mode = Processor::Type; // i understand the using-declaration
    constexpr int chorusId = 1;
    mTypeCmp.addItem("Chorus", chorusId);

    constexpr int flangerId = 2;
    mTypeCmp.addItem("Flanger", flangerId);

    mTypeCmp.onChange = [this, typeParam]() {
        typeParam->beginChangeGesture();

        switch (mTypeCmp.getSelectedId()) {
            case chorusId:
                *typeParam = static_cast<int>(Mode::Chorus);
                break;
            case flangerId:
                *typeParam = static_cast<int>(Mode::Flanger);
                break;
            default:
                jassertfalse;
        }

        typeParam->endChangeGesture();
    };
    mTypeCmp.setSelectedItemIndex(*typeParam);
    addAndMakeVisible(mTypeCmp);

    //waveform zoom slider
    mWaveformZoomSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mWaveformZoomSlider.setRange(0.1, 1.0);
    mWaveformZoomSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 200,32);
    mWaveformZoomSlider.setValue(mWaveFormComponent->getZoom());


    mWaveformZoomSlider.onValueChange = [this] { //lambda functon,
        mWaveFormComponent->setZoom(mWaveformZoomSlider.getValue());
    };

    this->addAndMakeVisible(mWaveformZoomSlider);

    this->setSize (900, 900);

}

KadenzeChorusFlangerAudioProcessorEditor::~KadenzeChorusFlangerAudioProcessorEditor()
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto rect = getLocalBounds();
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
   // const auto waveformpath = getWaveformPath(rect);
   // g.strokePath(waveformpath,juce::PathStrokeType(1));
}

void KadenzeChorusFlangerAudioProcessorEditor::resized()
{
    mWaveFormComponent->setBounds(50,550,800,200);
    mWaveformZoomSlider.setBounds(50,350,100,100);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
