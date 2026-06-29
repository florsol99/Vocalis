// ============================================================
// VOCALIS by Florchis.io - PluginEditor.h
// Declaración de la interfaz gráfica del plugin
// ============================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class VocalisAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    VocalisAudioProcessorEditor(VocalisAudioProcessor&);
    ~VocalisAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { if (isVisible()) repaint(); }
    VocalisAudioProcessor& audioProcessor;

    void drawVUMeter(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawOscilloscope(juce::Graphics& g, juce::Rectangle<int> bounds);
    std::vector<float> spectrumData;
    std::vector<float> waveformData;

    // Filtro
    juce::Slider cutoffSlider, resonanceSlider;
    juce::Label cutoffLabel, resonanceLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment, resonanceAttachment;

    // Delay
    juce::Slider delayTimeSlider, delayFeedbackSlider, delayMixSlider;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayMixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment, delayFeedbackAttachment, delayMixAttachment;

    // Compresor
    juce::Slider compThresholdSlider, compRatioSlider, compGainSlider;
    juce::Label compThresholdLabel, compRatioLabel, compGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compThresholdAttachment, compRatioAttachment, compGainAttachment;

    juce::Colour meterColorLow{ juce::Colours::green };
    juce::Colour meterColorMid{ juce::Colours::yellow };
    juce::Colour meterColorHigh{ juce::Colours::red };
    juce::Colour textColor{ juce::Colours::white };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalisAudioProcessorEditor)
};