#pragma once
// ============================================================
// VOCALIS by Florchis.io
// Plugin de procesamiento de voz en tiempo real con:
// - Compresor, Reverb, Filtro pasa-bajos, Delay
// - Visualización: Espectro logarítmico, Osciloscopio, VU Meter
// ============================================================

#include <juce_audio_processors/juce_audio_processors.h>  // Framework de plugins VST/AU
#include <juce_dsp/juce_dsp.h>                           // FFT, filtros, reverb

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
    VocalisAudioProcessor();          // Inicializa todo: parámetros, FFT, buffers
    ~VocalisAudioProcessor() override; // Limpia recursos al cerrar el plugin

    // ============================================================
    // CICLO DE VIDA DEL PLUGIN
    // ============================================================

    // Se llama cuando el DAW conecta el plugin (sample rate, tamaño de buffer)
    // Acá reservamos memoria para todos los buffers de audio
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    // Se llama al desconectar el plugin del DAW
    // Liberamos toda la memoria reservada en prepareToPlay
    void releaseResources() override;

    // Le dice al DAW qué configuraciones de entrada/salida aceptamos
    // Vocalis acepta: entrada mono ? salida estéreo
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    // ============================================================
    // PROCESAMIENTO DE AUDIO - EL CORAZÓN DEL PLUGIN
    // ============================================================

    // Se llama cada vez que hay un bloque de audio para procesar
    // Cadena: Mic ? Compresor ? Reverb ? Filtro ? Delay ? Salida
    // También alimenta los buffers de visualización (FFT, osciloscopio)
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // ============================================================
    // INTERFAZ GRÁFICA (EDITOR)
    // ============================================================

    // Crea la ventana con perillas, espectro, osciloscopio, VU Meter
    juce::AudioProcessorEditor* createEditor() override;

    // Vocalis SÍ tiene interfaz gráfica ? true
    bool hasEditor() const override;

    // ============================================================
    // METADATOS DEL PLUGIN
    // ============================================================

    const juce::String getName() const override;    // "Vocalis" (aparece en el DAW)
    bool acceptsMidi() const override;              // No recibe MIDI ? false
    bool producesMidi() const override;             // No genera MIDI ? false
    bool isMidiEffect() const override;             // No es efecto MIDI ? false
    double getTailLengthSeconds() const override;   // Sin cola de audio ? 0.0

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

    // Guarda los valores de todas las perillas al cerrar el proyecto
    void getStateInformation(juce::MemoryBlock& destData) override;

    // Restaura los valores de las perillas al abrir el proyecto
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ============================================================
    // API PÚBLICA - GETTERS PARA EL EDITOR
    // El Editor consulta estos valores para mostrarlos en pantalla
    // std::atomic es thread-safe (seguro entre hilos)
    // ============================================================

    float getRMSLevel() const { return currentRMS.load(); }           // Nivel de entrada
    float getCutoffFreq() const { return cutoffFreq.load(); }         // Frecuencia de corte del filtro
    float getResonance() const { return resonance.load(); }           // Resonancia (Q) del filtro
    float getDelayTime() const { return delayTime.load(); }           // Tiempo del delay (ms)
    float getDelayFeedback() const { return delayFeedback.load(); }   // Realimentación del delay (%)
    float getDelayMix() const { return delayMix.load(); }             // Mezcla delay (%)
    float getCompressorReduction() const { return compressorReduction.load(); } // Reducción de ganancia (dB)
    float getReverbRoomSize() const { return reverbRoomSize.load(); } // Tamaño de sala (0-1)
    float getReverbDecay() const { return reverbDecay.load(); }       // Decaimiento (segundos)
    float getReverbMix() const { return reverbMix.load(); }           // Mezcla reverb (%)

    // ============================================================
    // ESPECTRO (ANÁLISIS DE FRECUENCIAS)
    // ============================================================

    static constexpr int fftOrder = 11;               // Orden FFT: 2^11 = 2048 muestras
    static constexpr int fftSize = 1 << fftOrder;      // Tamaño de la ventana FFT
    static constexpr int numSpectrumBins = fftSize / 2; // Cantidad de barras: 1024

    // Devuelve los datos del espectro para que el Editor los dibuje
    void getSpectrumData(std::vector<float>& dest);

    // Devuelve los datos de forma de onda para el osciloscopio
    void getWaveformData(std::vector<float>& dest);

    // ============================================================
    // ÁRBOL DE PARÁMETROS
    // Contiene TODAS las perillas. Maneja comunicación processor?editor,
    // automatización del DAW, guardado/carga de presets
    // ============================================================
    juce::AudioProcessorValueTreeState parameters;

private:
    // ============================================================
    // MEDICIÓN DE VOLUMEN (RMS)
    // ============================================================
    std::atomic<float> currentRMS{ 0.0f };     // Nivel actual (thread-safe)
    juce::AudioParameterFloat* rmsParam = nullptr; // Puntero al parámetro registrado

    // ============================================================
    // FILTRO PASA-BAJOS
    // Corta frecuencias agudas a partir de cutoff.
    // Resonancia (Q) crea un pico justo antes del corte.
    // ============================================================
    std::atomic<float> cutoffFreq{ 10000.0f };     // Hz (200 - 20000)
    std::atomic<float> resonance{ 0.707f };         // Q (0.5 - 5.0)
    juce::AudioParameterFloat* cutoffParam = nullptr;
    juce::AudioParameterFloat* resonanceParam = nullptr;
    // Filtro IIR pasa-bajos duplicado a estéreo
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> lowPassFilter;

    // ============================================================
    // DELAY (ECO)
    // Buffer circular que graba y reproduce con retardo.
    // Feedback = cuántas repeticiones. Mix = balance seco/eco.
    // ============================================================
    std::atomic<float> delayTime{ 300.0f };        // ms (0 - 1000)
    std::atomic<float> delayFeedback{ 40.0f };     // % (0 - 90)
    std::atomic<float> delayMix{ 30.0f };          // % (0 - 100)
    juce::AudioParameterFloat* delayTimeParam = nullptr;
    juce::AudioParameterFloat* delayFeedbackParam = nullptr;
    juce::AudioParameterFloat* delayMixParam = nullptr;
    juce::AudioBuffer<float> delayBuffer;          // Buffer circular para el eco
    int delayBufferSize = 0;                       // Tamaño en samples
    int delayWritePos = 0;                         // Posición actual de escritura

    // ============================================================
    // COMPRESOR (FEED-FORWARD CON DETECTOR RMS)
    // Reduce picos de volumen y sube lo bajito.
    // Threshold = umbral. Ratio = relación de compresión.
    // Gain = maquillaje de ganancia post-compresión.
    // ============================================================
    std::atomic<float> compressorReduction{ 0.0f }; // dB de reducción actual
    juce::AudioParameterFloat* compThresholdParam = nullptr; // dB (-40 a 0)
    juce::AudioParameterFloat* compRatioParam = nullptr;     // :1 (1 a 20)
    juce::AudioParameterFloat* compGainParam = nullptr;      // dB (0 a 24)
    float compRmsFiltered = 0.0f;    // RMS suavizado (detector)
    float compAlphaAttack = 0.0f;    // Coeficiente de ataque (rápido)
    float compAlphaRelease = 0.0f;   // Coeficiente de release (lento)

    // ============================================================
    // REVERB (AMBIENTE ESPACIAL)
    // Simula una habitación/teatro/catedral.
    // Room Size = tamaño. Decay = duración. Mix = balance.
    // ============================================================
    std::atomic<float> reverbRoomSize{ 0.5f };     // 0 (chico) - 1 (enorme)
    std::atomic<float> reverbDecay{ 1.5f };        // segundos (0.1 - 10)
    std::atomic<float> reverbMix{ 20.0f };         // % (0 - 100)
    juce::AudioParameterFloat* reverbRoomSizeParam = nullptr;
    juce::AudioParameterFloat* reverbDecayParam = nullptr;
    juce::AudioParameterFloat* reverbMixParam = nullptr;
    juce::dsp::Reverb reverb;                      // Motor de reverb de JUCE

    // ============================================================
    // OSCILOSCOPIO (FORMA DE ONDA)
    // Guarda 512 muestras de audio para dibujar la onda verde
    // ============================================================
    std::vector<float> waveformForUI;               // 512 muestras para el Editor
    static constexpr int waveformUISize = 512;      // Tamaño fijo
    juce::CriticalSection waveformLock;             // Mutex para acceso thread-safe

    // ============================================================
    // ESPECTRO (FFT - ANÁLISIS DE FRECUENCIAS)
    // Convierte audio de dominio tiempo ? dominio frecuencia
    // ============================================================
    std::unique_ptr<juce::dsp::FFT> forwardFFT;         // Motor FFT
    juce::AudioBuffer<float> circularBuffer;            // Buffer circular (1 seg)
    int circularBufferWritePos = 0;                     // Posición de escritura
    std::vector<float> fftTimeDomain;                   // Entrada con ventana de Hann
    std::vector<float> fftFreqDomain;                   // Salida compleja (real+imag)
    std::vector<float> spectrumMagnitudes;              // Magnitudes (1024 bins)
    juce::AbstractFifo spectrumFifo{ 2048 };            // FIFO thread-safe
    std::vector<float> spectrumForUI;                   // Buffer protegido por FIFO

    // ============================================================
    // MÉTODOS PRIVADOS DE PROCESAMIENTO
    // ============================================================

    // Agrega una muestra al buffer circular para la FFT
    void pushSampleIntoFIFO(float sample);

    // Cuando hay 2048 muestras, ejecuta la FFT y actualiza el espectro
    void performFFTIfReady();

    // Recalcula los coeficientes del filtro pasa-bajos según cutoff y Q
    void updateFilter();

    // Aplica compresión feed-forward con detector RMS al buffer de audio
    void processCompressor(juce::AudioBuffer<float>& buffer);

    // ============================================================
    // MACRO DE JUCE: Prohíbe copiar el objeto y detecta memory leaks
    // ============================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalisAudioProcessor)
};