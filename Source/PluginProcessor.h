// ============================================================
// VOCALIS by Florchis.io - PluginProcessor.h
// Plugin de procesamiento de voz en tiempo real con:
// - Compresor, Filtro pasa-bajos, Delay
// - Visualización: Espectro logarítmico, Osciloscopio, VU Meter
// ============================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
// Clase principal del procesador de audio
// Hereda de juce::AudioProcessor (clase base de JUCE para plugins)
//==============================================================================
class VocalisAudioProcessor : public juce::AudioProcessor
{
public:
    // ============================================================
    // CONSTRUCTOR / DESTRUCTOR
    // ============================================================
    VocalisAudioProcessor();
    ~VocalisAudioProcessor() override;

    // ============================================================
    // CICLO DE VIDA DEL PLUGIN
    // ============================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // ============================================================
    // PROCESAMIENTO DE AUDIO - EL CORAZÓN DEL PLUGIN
    // ============================================================
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // ============================================================
    // INTERFAZ GRÁFICA (EDITOR)
    // ============================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // ============================================================
    // METADATOS DEL PLUGIN
    // ============================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // ============================================================
    // SISTEMA DE PRESETS (1 programa básico)
    // ============================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    // ============================================================
    // GUARDAR / CARGAR ESTADO
    // ============================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ============================================================
    // API PÚBLICA - GETTERS PARA EL EDITOR
    // ============================================================
    float getRMSLevel() const { return currentRMS.load(); }
    float getCutoffFreq() const { return cutoffFreq.load(); }
    float getResonance() const { return resonance.load(); }
    float getDelayTime() const { return delayTime.load(); }
    float getDelayFeedback() const { return delayFeedback.load(); }
    float getDelayMix() const { return delayMix.load(); }
    float getCompressorReduction() const { return compressorReduction.load(); }

    // ============================================================
    // ESPECTRO (ANÁLISIS DE FRECUENCIAS)
    // ============================================================
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int numSpectrumBins = fftSize / 2;
    void getSpectrumData(std::vector<float>& dest);
    void getWaveformData(std::vector<float>& dest);

    // ============================================================
    // ÁRBOL DE PARÁMETROS
    // ============================================================
    juce::AudioProcessorValueTreeState parameters;

private:
    // ============================================================
    // MEDICIÓN DE VOLUMEN (RMS)
    // ============================================================
    std::atomic<float> currentRMS{ 0.0f };
    juce::AudioParameterFloat* rmsParam = nullptr;

    // ============================================================
    // FILTRO PASA-BAJOS
    // ============================================================
    std::atomic<float> cutoffFreq{ 10000.0f };
    std::atomic<float> resonance{ 0.707f };
    juce::AudioParameterFloat* cutoffParam = nullptr;
    juce::AudioParameterFloat* resonanceParam = nullptr;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> lowPassFilter;

    // ============================================================
    // DELAY (ECO)
    // ============================================================
    std::atomic<float> delayTime{ 300.0f };
    std::atomic<float> delayFeedback{ 40.0f };
    std::atomic<float> delayMix{ 30.0f };
    juce::AudioParameterFloat* delayTimeParam = nullptr;
    juce::AudioParameterFloat* delayFeedbackParam = nullptr;
    juce::AudioParameterFloat* delayMixParam = nullptr;
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferSize = 0;
    int delayWritePos = 0;

    // ============================================================
    // COMPRESOR (FEED-FORWARD CON DETECTOR RMS)
    // ============================================================
    std::atomic<float> compressorReduction{ 0.0f };
    juce::AudioParameterFloat* compThresholdParam = nullptr;
    juce::AudioParameterFloat* compRatioParam = nullptr;
    juce::AudioParameterFloat* compGainParam = nullptr;
    float compRmsFiltered = 0.0f;
    float compAlphaAttack = 0.0f;
    float compAlphaRelease = 0.0f;

    // ============================================================
    // OSCILOSCOPIO (FORMA DE ONDA)
    // ============================================================
    std::vector<float> waveformForUI;
    static constexpr int waveformUISize = 512;
    juce::CriticalSection waveformLock;

    // ============================================================
    // ESPECTRO (FFT - ANÁLISIS DE FRECUENCIAS)
    // ============================================================
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    juce::AudioBuffer<float> circularBuffer;
    int circularBufferWritePos = 0;
    std::vector<float> fftTimeDomain;
    std::vector<float> fftFreqDomain;
    std::vector<float> spectrumMagnitudes;
    juce::AbstractFifo spectrumFifo{ 2048 };
    std::vector<float> spectrumForUI;

    // ============================================================
    // MÉTODOS PRIVADOS DE PROCESAMIENTO
    // ============================================================
    void pushSampleIntoFIFO(float sample);
    void performFFTIfReady();
    void updateFilter();
    void processCompressor(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalisAudioProcessor)
};