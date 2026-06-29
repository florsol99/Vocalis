// ============================================================
// VOCALIS by Florchis.io - PluginProcessor.cpp
// Implementación del procesador de audio
// Cadena: Mic ? Compresor ? Filtro ? Delay ? Salida
// ============================================================

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONSTRUCTOR
//==============================================================================
VocalisAudioProcessor::VocalisAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::mono(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier("Vocalis"),
        {
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "rms", 1 }, "RMS Level", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "cutoff", 1 }, "Cutoff",
                juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 10000.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "resonance", 1 }, "Resonance",
                juce::NormalisableRange<float>(0.5f, 3.0f, 0.01f), 0.707f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "delayTime", 1 }, "Delay Time",
                juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f), 300.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "delayFeedback", 1 }, "Feedback", 0.0f, 90.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "delayMix", 1 }, "Delay Mix", 0.0f, 100.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "compThreshold", 1 }, "Threshold",
                juce::NormalisableRange<float>(-40.0f, 0.0f, 1.0f), 0.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "compRatio", 1 }, "Ratio",
                juce::NormalisableRange<float>(1.0f, 20.0f, 0.5f), 4.0f),
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "compGain", 1 }, "Makeup Gain", 0.0f, 24.0f, 0.0f)
        })
{
    rmsParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("rms"));
    cutoffParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("cutoff"));
    resonanceParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("resonance"));
    delayTimeParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayTime"));
    delayFeedbackParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayFeedback"));
    delayMixParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayMix"));
    compThresholdParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compThreshold"));
    compRatioParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compRatio"));
    compGainParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compGain"));

    forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);
    fftTimeDomain.resize(fftSize * 2, 0.0f);
    fftFreqDomain.resize(fftSize * 2, 0.0f);
    spectrumMagnitudes.resize(numSpectrumBins, 0.0f);
    spectrumForUI.resize(numSpectrumBins, 0.0f);
    waveformForUI.resize(waveformUISize, 0.0f);

    delayBuffer.clear();
    delayWritePos = 0;
    compRmsFiltered = 0.0f;
}

VocalisAudioProcessor::~VocalisAudioProcessor() {}

//==============================================================================
// METADATOS
//==============================================================================
const juce::String VocalisAudioProcessor::getName() const { return JucePlugin_Name; }
bool VocalisAudioProcessor::acceptsMidi() const { return false; }
bool VocalisAudioProcessor::producesMidi() const { return false; }
bool VocalisAudioProcessor::isMidiEffect() const { return false; }
double VocalisAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VocalisAudioProcessor::getNumPrograms() { return 1; }
int VocalisAudioProcessor::getCurrentProgram() { return 0; }
void VocalisAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String VocalisAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void VocalisAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

//==============================================================================
// PREPARETOPLAY
//==============================================================================
void VocalisAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int bufferSize = static_cast<int>(sampleRate);
    if (bufferSize > 0)
    {
        circularBuffer.setSize(1, bufferSize);
        circularBuffer.clear();
        circularBufferWritePos = 0;
    }

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    lowPassFilter.prepare(spec);
    lowPassFilter.reset();
    updateFilter();

    delayBufferSize = static_cast<int>(sampleRate);
    delayBuffer.setSize(2, delayBufferSize);
    delayBuffer.clear();
    delayWritePos = 0;

    float attackMs = 5.0f;
    float releaseMs = 50.0f;
    compAlphaAttack = 1.0f - std::exp(-1.0f / (attackMs / 1000.0f * sampleRate));
    compAlphaRelease = 1.0f - std::exp(-1.0f / (releaseMs / 1000.0f * sampleRate));
    compRmsFiltered = 0.0f;

    delayBuffer.clear();
    delayWritePos = 0;
    for (int i = 0; i < delayBufferSize; ++i)
    {
        delayBuffer.setSample(0, i, 0.0f);
        delayBuffer.setSample(1, i, 0.0f);
    }
}

void VocalisAudioProcessor::releaseResources()
{
    circularBuffer.setSize(0, 0);
    delayBuffer.setSize(0, 0);
}

bool VocalisAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

//==============================================================================
// UPDATEFILTER
//==============================================================================
void VocalisAudioProcessor::updateFilter()
{
    float cutoff = cutoffParam ? cutoffParam->get() : 10000.0f;
    cutoffFreq.store(cutoff);
    float q = resonanceParam ? resonanceParam->get() : 0.707f;
    resonance.store(q);
    float rms = currentRMS.load();
    if (rms < 0.001f) q = juce::jlimit(0.5f, 1.5f, q);
    else              q = juce::jlimit(0.5f, 3.0f, q);
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoff, q);
}

//==============================================================================
// FFT
//==============================================================================
void VocalisAudioProcessor::pushSampleIntoFIFO(float sample)
{
    if (circularBuffer.getNumSamples() > 0)
    {
        circularBuffer.setSample(0, circularBufferWritePos, sample);
        circularBufferWritePos++;
        if (circularBufferWritePos >= circularBuffer.getNumSamples())
            circularBufferWritePos = 0;
    }
}

void VocalisAudioProcessor::performFFTIfReady()
{
    if (circularBuffer.getNumSamples() < fftSize) return;
    if (fftTimeDomain.size() < fftSize * 2 || fftFreqDomain.size() < fftSize * 2
        || spectrumMagnitudes.size() < numSpectrumBins) return;

    for (int i = 0; i < fftSize; ++i)
    {
        int readPos = circularBufferWritePos - fftSize + i;
        if (readPos < 0) readPos += circularBuffer.getNumSamples();
        if (readPos < 0 || readPos >= circularBuffer.getNumSamples()) return;
        float sample = circularBuffer.getSample(0, readPos);
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
        fftTimeDomain[i * 2] = sample * window;
        fftTimeDomain[i * 2 + 1] = 0.0f;
    }

    std::copy(fftTimeDomain.begin(), fftTimeDomain.end(), fftFreqDomain.begin());
    forwardFFT->performRealOnlyForwardTransform(fftFreqDomain.data(), true);

    for (int i = 0; i < numSpectrumBins; ++i)
    {
        float real = fftFreqDomain[i * 2];
        float imag = fftFreqDomain[i * 2 + 1];
        spectrumMagnitudes[i] = std::sqrt(real * real + imag * imag) / fftSize;
    }

    int start1, size1, start2, size2;
    spectrumFifo.prepareToWrite(numSpectrumBins, start1, size1, start2, size2);
    if (size1 > 0 && start1 >= 0 && start1 + size1 <= (int)spectrumForUI.size())
        std::copy(spectrumMagnitudes.begin(), spectrumMagnitudes.begin() + size1, spectrumForUI.begin() + start1);
    if (size2 > 0 && start2 >= 0 && start2 + size2 <= (int)spectrumForUI.size())
        std::copy(spectrumMagnitudes.begin() + size1, spectrumMagnitudes.begin() + size1 + size2, spectrumForUI.begin() + start2);
    spectrumFifo.finishedWrite(size1 + size2);
}

void VocalisAudioProcessor::getSpectrumData(std::vector<float>& dest)
{
    dest.assign(numSpectrumBins, 0.0f);
    int ready = spectrumFifo.getNumReady();
    if (ready >= numSpectrumBins)
    {
        int start1, size1, start2, size2;
        spectrumFifo.prepareToRead(numSpectrumBins, start1, size1, start2, size2);
        if (size1 > 0 && start1 >= 0 && start1 + size1 <= (int)spectrumForUI.size())
            std::copy(spectrumForUI.begin() + start1, spectrumForUI.begin() + start1 + size1, dest.begin());
        if (size2 > 0 && start2 >= 0 && start2 + size2 <= (int)spectrumForUI.size())
            std::copy(spectrumForUI.begin() + start2, spectrumForUI.begin() + start2 + size2, dest.begin() + size1);
        spectrumFifo.finishedRead(size1 + size2);
    }
}

void VocalisAudioProcessor::getWaveformData(std::vector<float>& dest)
{
    juce::ScopedLock sl(waveformLock);
    dest = waveformForUI;
}

//==============================================================================
// PROCESSCOMPRESSOR
//==============================================================================
void VocalisAudioProcessor::processCompressor(juce::AudioBuffer<float>& buffer)
{
    float threshold = compThresholdParam ? compThresholdParam->get() : 0.0f;
    float ratio = compRatioParam ? compRatioParam->get() : 4.0f;
    float makeupGain = compGainParam ? compGainParam->get() : 0.0f;
    if (threshold >= 0.0f) return;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float inputSquared = data[s] * data[s];
            if (inputSquared > compRmsFiltered)
                compRmsFiltered += compAlphaAttack * (inputSquared - compRmsFiltered);
            else
                compRmsFiltered += compAlphaRelease * (inputSquared - compRmsFiltered);
            float rmsDb = juce::Decibels::gainToDecibels(std::sqrt(compRmsFiltered + 1e-10f));
            float gainReductionDb = 0.0f;
            if (rmsDb > threshold)
            {
                float overshoot = rmsDb - threshold;
                gainReductionDb = overshoot * (1.0f - 1.0f / ratio);
            }
            compressorReduction.store(gainReductionDb);
            float gainDb = -gainReductionDb + makeupGain;
            data[s] *= juce::Decibels::decibelsToGain(gainDb);
        }
    }
}

//==============================================================================
// PROCESSBLOCK - EL CORAZÓN DEL PLUGIN
// Cadena: Mic ? Compresor ? Filtro ? Delay ? Salida
//==============================================================================
void VocalisAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    for (int channel = buffer.getNumChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() == 0) return;

    float rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    currentRMS.store(rms);
    if (rmsParam) rmsParam->setValueNotifyingHost(rms);

    if (rms < 0.0001f)
    {
        delayBuffer.clear();
        delayWritePos = 0;
        compRmsFiltered = 0.0f;
        return;
    }

    auto* channelData = buffer.getReadPointer(0);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        pushSampleIntoFIFO(channelData[sample]);
    performFFTIfReady();

    {
        juce::ScopedLock sl(waveformLock);
        auto* waveData = buffer.getReadPointer(0);
        for (int s = 0; s < buffer.getNumSamples(); s += juce::jmax(1, buffer.getNumSamples() / waveformUISize))
        {
            int idx = s * waveformUISize / buffer.getNumSamples();
            if (idx < waveformUISize) waveformForUI[idx] = waveData[s];
        }
    }

    if (buffer.getNumChannels() < 2 && getTotalNumOutputChannels() >= 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());

    // 1. COMPRESOR
    float threshold = compThresholdParam ? compThresholdParam->get() : 0.0f;
    if (threshold < -0.5f) processCompressor(buffer);
    else compRmsFiltered = 0.0f;

    // 2. FILTRO
    updateFilter();
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        lowPassFilter.process(context);
    }

    // 3. DELAY
    float dTime = delayTimeParam ? delayTimeParam->get() : 0.0f;
    float dFeedback = delayFeedbackParam ? delayFeedbackParam->get() / 100.0f : 0.0f;
    float dMix = delayMixParam ? delayMixParam->get() / 100.0f : 0.0f;

    dTime = juce::jlimit(0.0f, 1000.0f, dTime);
    dFeedback = juce::jlimit(0.0f, 0.5f, dFeedback);
    dMix = juce::jlimit(0.0f, 1.0f, dMix);

    delayTime.store(dTime);
    delayFeedback.store(dFeedback * 100.0f);
    delayMix.store(dMix * 100.0f);

    if (dMix < 0.001f) { delayBuffer.clear(); delayWritePos = 0; }

    int delaySamples = static_cast<int>(dTime / 1000.0 * getSampleRate());
    delaySamples = juce::jlimit(0, delayBufferSize - 1, delaySamples);

    if (delaySamples > 0 && dMix > 0.001f)
    {
        auto* outL = buffer.getWritePointer(0);
        auto* outR = buffer.getWritePointer(1);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputL = outL[sample], inputR = outR[sample];
            int readPos = delayWritePos - delaySamples;
            if (readPos < 0) readPos += delayBufferSize;
            float delayL = delayBuffer.getSample(0, readPos);
            float delayR = delayBuffer.getSample(1, readPos);
            delayBuffer.setSample(0, delayWritePos, inputL + delayL * dFeedback);
            delayBuffer.setSample(1, delayWritePos, inputR + delayR * dFeedback);
            outL[sample] = inputL * (1.0f - dMix) + delayL * dMix;
            outR[sample] = inputR * (1.0f - dMix) + delayR * dMix;
            delayWritePos = (delayWritePos + 1) % delayBufferSize;
        }
    }
}

//==============================================================================
// EDITOR Y ESTADO
//==============================================================================
bool VocalisAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* VocalisAudioProcessor::createEditor() { return new VocalisAudioProcessorEditor(*this); }

void VocalisAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VocalisAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VocalisAudioProcessor(); }