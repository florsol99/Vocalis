// ============================================================
// VOCALIS by Florchis.io - PluginEditor.cpp
// Implementación de la interfaz gráfica
// ============================================================

#include "PluginProcessor.h"
#include "PluginEditor.h"

VocalisAudioProcessorEditor::VocalisAudioProcessorEditor(VocalisAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(950, 480);
    setResizable(true, true);
    setResizeLimits(750, 380, 1600, 800);

    // FILTRO
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    cutoffSlider.setRange(200.0, 20000.0, 1.0);
    cutoffSlider.setSkewFactor(0.3);
    cutoffSlider.setValue(10000.0);
    cutoffSlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(cutoffSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "cutoff", cutoffSlider);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    cutoffLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(cutoffLabel);

    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    resonanceSlider.setRange(0.5, 3.0, 0.01);
    resonanceSlider.setValue(0.707);
    resonanceSlider.setTextValueSuffix(" Q");
    addAndMakeVisible(resonanceSlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "resonance", resonanceSlider);
    resonanceLabel.setText("Reson", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    resonanceLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(resonanceLabel);

    // DELAY
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayTimeSlider.setRange(0.0, 1000.0, 1.0);
    delayTimeSlider.setValue(300.0);
    delayTimeSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(delayTimeSlider);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delayTime", delayTimeSlider);
    delayTimeLabel.setText("Tiempo", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayTimeLabel);

    delayFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayFeedbackSlider.setRange(0.0, 90.0, 1.0);
    delayFeedbackSlider.setValue(40.0);
    delayFeedbackSlider.setTextValueSuffix(" %");
    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delayFeedback", delayFeedbackSlider);
    delayFeedbackLabel.setText("Feedb", juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    delayFeedbackLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayFeedbackLabel);

    delayMixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayMixSlider.setRange(0.0, 100.0, 1.0);
    delayMixSlider.setValue(30.0);
    delayMixSlider.setTextValueSuffix(" %");
    addAndMakeVisible(delayMixSlider);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "delayMix", delayMixSlider);
    delayMixLabel.setText("Mezcla", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centred);
    delayMixLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayMixLabel);

    // COMPRESOR
    compThresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compThresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compThresholdSlider.setRange(-40.0, 0.0, 1.0);
    compThresholdSlider.setValue(-20.0);
    compThresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(compThresholdSlider);
    compThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "compThreshold", compThresholdSlider);
    compThresholdLabel.setText("Thresh", juce::dontSendNotification);
    compThresholdLabel.setJustificationType(juce::Justification::centred);
    compThresholdLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compThresholdLabel);

    compRatioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compRatioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compRatioSlider.setRange(1.0, 20.0, 0.5);
    compRatioSlider.setValue(4.0);
    compRatioSlider.setTextValueSuffix(":1");
    addAndMakeVisible(compRatioSlider);
    compRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "compRatio", compRatioSlider);
    compRatioLabel.setText("Ratio", juce::dontSendNotification);
    compRatioLabel.setJustificationType(juce::Justification::centred);
    compRatioLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compRatioLabel);

    compGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compGainSlider.setRange(0.0, 24.0, 0.5);
    compGainSlider.setValue(6.0);
    compGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(compGainSlider);
    compGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "compGain", compGainSlider);
    compGainLabel.setText("Gain", juce::dontSendNotification);
    compGainLabel.setJustificationType(juce::Justification::centred);
    compGainLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compGainLabel);

    startTimerHz(30);
}

VocalisAudioProcessorEditor::~VocalisAudioProcessorEditor() { stopTimer(); }

void VocalisAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient bg(juce::Colour(28, 28, 33), 0, 0, juce::Colour(18, 18, 23), 0, getHeight(), false);
    g.setGradientFill(bg);
    g.fillAll();

    auto mainArea = getLocalBounds();
    auto controlsArea = mainArea.removeFromRight(210);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(controlsArea);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX(), controlsArea.getY(), controlsArea.getX(), controlsArea.getBottom(), 1.0f);

    g.setFont(juce::Font("Helvetica", 11.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("FILTRO", controlsArea.getX(), controlsArea.getY() + 1, controlsArea.getWidth(), 16, juce::Justification::centred);

    int sepY1 = controlsArea.getY() + 95;
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX() + 15, sepY1, controlsArea.getRight() - 15, sepY1, 0.5f);
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("DELAY", controlsArea.getX(), sepY1 + 10, controlsArea.getWidth(), 16, juce::Justification::centred);

    int sepY2 = controlsArea.getY() + 210;
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX() + 15, sepY2, controlsArea.getRight() - 15, sepY2, 0.5f);
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("COMPRESOR", controlsArea.getX(), sepY2 + 10, controlsArea.getWidth(), 16, juce::Justification::centred);

    auto vuArea = mainArea.removeFromLeft(45);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(vuArea);
    drawVUMeter(g, vuArea);

    auto visArea = mainArea.reduced(6, 20);
    auto specArea = visArea.removeFromTop(visArea.getHeight() * 0.58);
    drawSpectrum(g, specArea);
    drawOscilloscope(g, visArea);

    auto bottomArea = getLocalBounds().removeFromBottom(26);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bottomArea);
    g.setColour(juce::Colours::cyan.withAlpha(0.3f));
    g.drawLine(bottomArea.getX(), bottomArea.getY(), bottomArea.getRight(), bottomArea.getY(), 1.0f);

    float rms = audioProcessor.getRMSLevel();
    float db = juce::Decibels::gainToDecibels(rms, -60.0f);
    float cutoff = audioProcessor.getCutoffFreq();
    float q = audioProcessor.getResonance();
    float dTime = audioProcessor.getDelayTime();
    float dFb = audioProcessor.getDelayFeedback();
    float dMix = audioProcessor.getDelayMix();
    float compRed = audioProcessor.getCompressorReduction();

    juce::String info;
    info << "In: " << juce::String(db, 1) << " dB"
        << "  |  Cut: " << juce::String(cutoff / 1000.0, 2) << "k  Q: " << juce::String(q, 2)
        << "  |  Del: " << juce::String(dTime, 0) << "ms  Fb: " << juce::String(dFb, 0) << "%"
        << "  |  Comp: " << juce::String(compRed, 1) << "dB";

    g.setFont(juce::Font("Helvetica", 10.0f, juce::Font::plain));
    g.setColour(juce::Colours::lightgrey);
    g.drawText(info, bottomArea.reduced(8, 2), juce::Justification::centredLeft);

    g.setFont(juce::Font("Helvetica", 15.0f, juce::Font::bold));
    g.setColour(juce::Colour(212, 175, 55));
    g.drawText("Vocalis by Florchis.io", bottomArea.getRight() - 195, bottomArea.getY() + 4, 190, bottomArea.getHeight() - 8, juce::Justification::centred);
}

void VocalisAudioProcessorEditor::resized()
{
    auto controlsArea = getLocalBounds().removeFromRight(210);
    int knobSize = 60, spacing = 10, centerX = controlsArea.getCentreX(), x, y;

    y = controlsArea.getY() + 24;
    int filaWidth = knobSize * 2 + spacing;
    x = centerX - filaWidth / 2;
    cutoffSlider.setBounds(x, y, knobSize, knobSize);
    cutoffLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    resonanceSlider.setBounds(x, y, knobSize, knobSize);
    resonanceLabel.setBounds(x, y - 16, knobSize, 16);

    y = controlsArea.getY() + 130;
    int fila3Width = knobSize * 3 + spacing * 2;
    x = centerX - fila3Width / 2;
    delayTimeSlider.setBounds(x, y, knobSize, knobSize);
    delayTimeLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    delayFeedbackSlider.setBounds(x, y, knobSize, knobSize);
    delayFeedbackLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    delayMixSlider.setBounds(x, y, knobSize, knobSize);
    delayMixLabel.setBounds(x, y - 16, knobSize, 16);

    y = controlsArea.getY() + 260;
    x = centerX - fila3Width / 2;
    compThresholdSlider.setBounds(x, y, knobSize, knobSize);
    compThresholdLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    compRatioSlider.setBounds(x, y, knobSize, knobSize);
    compRatioLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    compGainSlider.setBounds(x, y, knobSize, knobSize);
    compGainLabel.setBounds(x, y - 16, knobSize, 16);

    repaint();
}

void VocalisAudioProcessorEditor::drawVUMeter(juce::Graphics& g, juce::Rectangle<int> meterBounds)
{
    if (meterBounds.getWidth() <= 10 || meterBounds.getHeight() <= 10) return;
    float rmsValue = audioProcessor.getRMSLevel();
    auto bounds = meterBounds.reduced(4, 40);
    if (bounds.isEmpty() || bounds.getHeight() <= 0) return;
    int mW = bounds.getWidth(), mH = bounds.getHeight(), mX = bounds.getX(), mY = bounds.getY();
    float dbValue = juce::Decibels::gainToDecibels(rmsValue, -60.0f);
    float norm = juce::jlimit(0.0f, 1.0f, juce::jmap(dbValue, -60.0f, 0.0f, 0.0f, 1.0f));
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(mX, mY, mW, mH, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(mX, mY, mW, mH, 2.0f, 1.0f);
    juce::Colour mc;
    if (norm < 0.5f) mc = meterColorLow;
    else if (norm < 0.8f) mc = meterColorMid;
    else mc = meterColorHigh;
    if (norm > 0.0f)
    {
        int fh = (int)(mH * norm);
        juce::ColourGradient grad(mc.withAlpha(0.4f), mX, mY + mH, mc, mX, mY, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(mX + 1, mY + mH - fh, mW - 2, fh, 1.0f);
    }
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    for (int db = -50; db <= -10; db += 20)
    {
        float yn = juce::jmap((float)db, -60.0f, 0.0f, 1.0f, 0.0f);
        int y = mY + mH - (int)(mH * yn);
        g.drawLine(mX - 2, y, mX + mW + 2, y, 0.5f);
    }
    g.setFont(7.0f);
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawText(juce::String(dbValue, 0), mX, mY - 12, mW, 10, juce::Justification::centred);
    g.setFont(8.0f);
    g.setColour(textColor.withAlpha(0.6f));
    g.drawText("VU", mX, mY + mH + 2, mW, 12, juce::Justification::centred);
}

void VocalisAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> specBounds)
{
    audioProcessor.getSpectrumData(spectrumData);
    if (spectrumData.empty()) return;
    auto bounds = specBounds;
    bounds.removeFromBottom(22);
    if (bounds.isEmpty() || bounds.getHeight() <= 0) return;
    int numBins = (int)spectrumData.size();
    if (numBins <= 0) return;
    float sr = audioProcessor.getSampleRate();
    if (sr <= 0.0f) sr = 44100.0f;
    const float minF = 20.0f, maxF = 20000.0f;
    float logMin = std::log(minF), logMax = std::log(maxF);
    float cutoffFreq = audioProcessor.getCutoffFreq();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

    for (int i = 0; i < numBins; ++i)
    {
        if (i >= (int)spectrumData.size()) break;
        float freq = i * sr / audioProcessor.fftSize;
        if (freq < minF || freq > maxF) continue;
        float dbVal = juce::Decibels::gainToDecibels(spectrumData[i], -80.0f);
        float normH = juce::jlimit(0.0f, 1.0f, juce::jmap(dbVal, -80.0f, 0.0f, 0.0f, 1.0f));
        if (normH > 0.001f)
        {
            float barH = bounds.getHeight() * normH;
            float logF = std::log(juce::jlimit(minF, maxF, freq));
            float xN = (logF - logMin) / (logMax - logMin);
            float x = bounds.getX() + xN * bounds.getWidth();
            float nf = (i + 1) * sr / audioProcessor.fftSize;
            nf = juce::jlimit(minF, maxF, nf);
            float nxN = (std::log(nf) - logMin) / (logMax - logMin);
            float nx = bounds.getX() + nxN * bounds.getWidth();
            float bw = juce::jmax(1.0f, nx - x - 0.5f);
            float y = bounds.getBottom() - barH;
            juce::Colour bc;
            if (freq > cutoffFreq) bc = juce::Colours::darkgrey.withAlpha(0.15f);
            else if (freq >= 200.0f && freq <= 3500.0f) bc = juce::Colours::cyan.withAlpha(0.75f);
            else if (freq < 200.0f) bc = juce::Colours::blue.withAlpha(0.45f);
            else bc = juce::Colours::white.withAlpha(0.35f);
            g.setColour(bc);
            g.fillRect(x, y, bw, barH);
        }
    }

    if (cutoffFreq < maxF && cutoffFreq > minF)
    {
        float lc = std::log(cutoffFreq);
        float cxN = (lc - logMin) / (logMax - logMin);
        float cx = bounds.getX() + cxN * bounds.getWidth();
        g.setColour(juce::Colours::red.withAlpha(0.15f));
        g.drawLine(cx - 1, bounds.getY(), cx - 1, bounds.getBottom(), 3.0f);
        g.drawLine(cx + 1, bounds.getY(), cx + 1, bounds.getBottom(), 3.0f);
        g.setColour(juce::Colours::red.withAlpha(0.7f));
        g.drawLine(cx, bounds.getY(), cx, bounds.getBottom(), 1.5f);
        g.setFont(juce::Font("Helvetica", 9.0f, juce::Font::bold));
        g.setColour(juce::Colours::red.withAlpha(0.9f));
        juce::String cutoffText;
        if (cutoffFreq >= 1000.0f) cutoffText << juce::String(cutoffFreq / 1000.0f, 1) << "k";
        else cutoffText << juce::String(cutoffFreq, 0) << "Hz";
        g.drawText(cutoffText, cx - 22, bounds.getY() - 14, 44, 14, juce::Justification::centred);
    }

    int refs[] = { 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800 };
    for (int f : refs)
    {
        if (f >= minF && f <= maxF && f < sr / 2.0f)
        {
            float lf = std::log((float)f);
            float xN = (lf - logMin) / (logMax - logMin);
            float x = bounds.getX() + xN * bounds.getWidth();
            g.setColour(juce::Colours::white.withAlpha(0.08f));
            g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.5f);
            juce::String l;
            if (f >= 1000) l << (f / 1000) << "k";
            else l << f;
            g.setFont(8.0f);
            g.setColour(juce::Colours::white.withAlpha(0.25f));
            g.drawText(l, (int)x - 15, bounds.getBottom() + 2, 30, 12, juce::Justification::centred);
        }
    }
}

void VocalisAudioProcessorEditor::drawOscilloscope(juce::Graphics& g, juce::Rectangle<int> oscBounds)
{
    audioProcessor.getWaveformData(waveformData);
    auto bounds = oscBounds.reduced(4, 4);
    if (bounds.isEmpty() || waveformData.empty() || bounds.getWidth() <= 0 || bounds.getHeight() <= 0) return;
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    auto waveArea = bounds.reduced(4, 12);
    if (waveArea.isEmpty() || waveArea.getHeight() <= 0) return;
    float midY = waveArea.getCentreY();
    float h = waveArea.getHeight() * 0.45f;
    int n = (int)waveformData.size();
    if (n <= 1) return;
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(waveArea.getX(), midY, waveArea.getRight(), midY, 0.5f);
    juce::Path path;
    path.startNewSubPath(waveArea.getX(), midY);
    float dx = waveArea.getWidth() / (float)(n - 1);
    for (int i = 0; i < n; ++i)
    {
        float x = waveArea.getX() + i * dx;
        float sample = waveformData[i];
        if (std::isnan(sample)) sample = 0.0f;
        float y = juce::jlimit(midY - h, midY + h, midY - sample * h * 2.0f);
        path.lineTo(x, y);
    }
    g.setColour(juce::Colours::lime.withAlpha(0.7f));
    g.strokePath(path, juce::PathStrokeType(1.0f));
    g.setFont(juce::Font("Helvetica", 8.0f, juce::Font::plain));
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawText("OSCILOSCOPIO", bounds.getX(), bounds.getY() + 1, bounds.getWidth(), 12, juce::Justification::centred);
}