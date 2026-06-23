// ============================================================
// VOCALIS by Florchis.io - PluginProcessor.cpp
// Implementación del procesador de audio
// ============================================================

#include "PluginProcessor.h"   // incluir la clase creada
#include "PluginEditor.h"      // Para crear el Editor

//==============================================================================
// CONSTRUCTOR
// Inicializa TODO el plugin: buses de audio, parámetros, punteros, buffers
//==============================================================================
VocalisAudioProcessor::VocalisAudioProcessor()
// --- Configuración de buses de entrada/salida ---
// Le decimos al DAW: "Necesito entrada mono, salida estéreo"
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect      // Si no es efecto MIDI
#if !JucePlugin_IsSynth           // Si no es sintetizador
        .withInput("Input", juce::AudioChannelSet::mono(), true)    // Entrada mono
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true) // Salida estéreo
#endif
    ),
    // --- Árbol de parámetros (TODAS las perillas) ---
    // El ValueTreeState maneja: comunicación processor?editor, automatización DAW, guardado
    parameters(*this, nullptr, juce::Identifier("Vocalis"),
        {
            // --- RMS Level (medidor interno, no visible) ---
            std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { "rms", 1 }, "RMS Level",
                0.0f, 1.0f, 0.0f),

                // --- FILTRO: Cutoff (200 Hz a 20 kHz, escala logarítmica skew=0.3) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "cutoff", 1 }, "Cutoff",
                    juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 10000.0f),

                // --- FILTRO: Resonancia / Q (0.5 = suave, 5.0 = metálico) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "resonance", 1 }, "Resonance",
                    juce::NormalisableRange<float>(0.5f, 5.0f, 0.01f), 0.707f),

                // --- DELAY: Tiempo (0 a 1000 ms) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "delayTime", 1 }, "Delay Time",
                    juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f), 300.0f),

                // --- DELAY: Feedback / realimentación (0% a 90%) ---
                // Arranca en 0 para evitar auto-oscilación al cargar
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "delayFeedback", 1 }, "Feedback",
                    0.0f, 90.0f, 0.0f),

                // --- DELAY: Mix / mezcla eco-voz (0% a 100%) ---
                // Arranca en 0 = sin delay
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "delayMix", 1 }, "Delay Mix",
                    0.0f, 100.0f, 0.0f),

                // --- COMPRESOR: Threshold / umbral (-40 dB a 0 dB) ---
                // Arranca en 0 dB = sin compresión
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "compThreshold", 1 }, "Threshold",
                    juce::NormalisableRange<float>(-40.0f, 0.0f, 1.0f), 0.0f),

                // --- COMPRESOR: Ratio / relación (1:1 a 20:1) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "compRatio", 1 }, "Ratio",
                    juce::NormalisableRange<float>(1.0f, 20.0f, 0.5f), 4.0f),

                // --- COMPRESOR: Makeup Gain (0 dB a 24 dB) ---
                // Arranca en 0 dB = sin ganancia extra
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "compGain", 1 }, "Makeup Gain",
                    0.0f, 24.0f, 0.0f),

                // --- REVERB: Room Size / tamaño de sala (0 = chico, 1 = enorme) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "reverbRoomSize", 1 }, "Room Size",
                    0.0f, 1.0f, 0.5f),

                // --- REVERB: Decay / decaimiento (0.1 s a 10 s) ---
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "reverbDecay", 1 }, "Decay",
                    0.1f, 10.0f, 1.5f),

                // --- REVERB: Mix / mezcla (0% a 100%) ---
                // Arranca en 0 = sin reverb
                std::make_unique<juce::AudioParameterFloat>(
                    juce::ParameterID { "reverbMix", 1 }, "Reverb Mix",
                    0.0f, 100.0f, 0.0f)
        })
{
    // ============================================================
    // OBTENER PUNTEROS A LOS PARÁMETROS REGISTRADOS
    // dynamic_cast convierte AudioParameter* a AudioParameterFloat*
    // Esto nos da acceso directo a get() y setValueNotifyingHost()
    // ============================================================
    rmsParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("rms"));
    cutoffParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("cutoff"));
    resonanceParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("resonance"));
    delayTimeParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayTime"));
    delayFeedbackParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayFeedback"));
    delayMixParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("delayMix"));
    compThresholdParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compThreshold"));
    compRatioParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compRatio"));
    compGainParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("compGain"));
    reverbRoomSizeParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("reverbRoomSize"));
    reverbDecayParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("reverbDecay"));
    reverbMixParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("reverbMix"));

    // ============================================================
    // INICIALIZAR MOTOR FFT (Fast Fourier Transform)
    // fftOrder=11 ? 2^11=2048 muestras ? 1024 bins de frecuencia
    // ============================================================
    forwardFFT = std::make_unique<juce::dsp::FFT>(fftOrder);

    // Buffers para la FFT (formato complejo: real+imag intercalados)
    fftTimeDomain.resize(fftSize * 2, 0.0f);       // Entrada (con ventana)
    fftFreqDomain.resize(fftSize * 2, 0.0f);       // Salida (compleja)
    spectrumMagnitudes.resize(numSpectrumBins, 0.0f); // 1024 barras
    spectrumForUI.resize(numSpectrumBins, 0.0f);      // Copia thread-safe para UI

    // Buffer del osciloscopio (512 muestras para la onda verde)
    waveformForUI.resize(waveformUISize, 0.0f);
}

//==============================================================================
// DESTRUCTOR - Limpieza automática por smart pointers y vectores
//==============================================================================
VocalisAudioProcessor::~VocalisAudioProcessor() {}

//==============================================================================
// METADATOS - Información que aparece en el DAW
//==============================================================================
const juce::String VocalisAudioProcessor::getName() const { return JucePlugin_Name; }  // "Vocalis"
bool VocalisAudioProcessor::acceptsMidi() const { return false; }    // No recibe notas MIDI
bool VocalisAudioProcessor::producesMidi() const { return false; }   // No genera MIDI
bool VocalisAudioProcessor::isMidiEffect() const { return false; }   // No procesa MIDI
double VocalisAudioProcessor::getTailLengthSeconds() const { return 0.0; } // Sin cola

//==============================================================================
// PRESETS - Soporte mínimo (1 programa, sin nombres editables)
//==============================================================================
int VocalisAudioProcessor::getNumPrograms() { return 1; }
int VocalisAudioProcessor::getCurrentProgram() { return 0; }
void VocalisAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String VocalisAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void VocalisAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

//==============================================================================
// PREPARETOPLAY - Se llama al conectar el plugin al DAW
// Reserva toda la memoria necesaria según el sample rate
//==============================================================================
void VocalisAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // --- Buffer circular para FFT (1 segundo de audio) ---
    int bufferSize = static_cast<int>(sampleRate);  // Ej: 44100 muestras
    if (bufferSize > 0)
    {
        circularBuffer.setSize(1, bufferSize); // 1 canal, 44100 muestras
        circularBuffer.clear();
        circularBufferWritePos = 0;             // Empezar al principio
    }

    // --- Inicializar filtro pasa-bajos (estéreo) ---
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;    // Tamaño máximo de bloque
    spec.numChannels = 2;                       // Estéreo
    lowPassFilter.prepare(spec);
    lowPassFilter.reset();
    updateFilter();                             // Aplicar valores iniciales

    // --- Buffer de delay (1 segundo de audio estéreo) ---
    delayBufferSize = static_cast<int>(sampleRate);
    delayBuffer.setSize(2, delayBufferSize);    // 2 canales
    delayBuffer.clear();
    delayWritePos = 0;

    // --- Calcular coeficientes del detector RMS del compresor ---
    // Ataque rápido (5ms), release lento (50ms)
    float attackMs = 5.0f;
    float releaseMs = 50.0f;
    // Fórmula: alpha = 1 - e^(-1 / (tiempo_en_segundos * sampleRate))
    compAlphaAttack = 1.0f - std::exp(-1.0f / (attackMs / 1000.0f * sampleRate));
    compAlphaRelease = 1.0f - std::exp(-1.0f / (releaseMs / 1000.0f * sampleRate));
    compRmsFiltered = 0.0f;  // Detector empieza en silencio

    // --- Inicializar reverb ---
    juce::dsp::ProcessSpec revSpec;
    revSpec.sampleRate = sampleRate;
    revSpec.maximumBlockSize = samplesPerBlock;
    revSpec.numChannels = 2;
    reverb.prepare(revSpec);
    reverb.reset();
}

//==============================================================================
// RELEASERESOURCES - Libera memoria al desconectar
//==============================================================================
void VocalisAudioProcessor::releaseResources()
{
    circularBuffer.setSize(0, 0);  // Liberar buffer FFT
    delayBuffer.setSize(0, 0);     // Liberar buffer delay
}

//==============================================================================
// ISBUSESLAYOUTSUPPORTED - Solo acepta mono?estéreo
//==============================================================================
bool VocalisAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Entrada: solo mono (o deshabilitada para plugins offline)
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;
    // Salida: solo estéreo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

//==============================================================================
// UPDATEFILTER - Recalcula los coeficientes del filtro según cutoff y Q
// Se llama en cada processBlock para reflejar cambios de perillas
//==============================================================================
void VocalisAudioProcessor::updateFilter()
{
    // Leer valores actuales (con protección si el puntero es nulo)
    float cutoff = cutoffParam ? cutoffParam->get() : 10000.0f;
    cutoffFreq.store(cutoff);  // Guardar para el Editor

    float q = resonanceParam ? resonanceParam->get() : 0.707f;
    resonance.store(q);

    // Crear filtro Butterworth pasa-bajos con cutoff y Q actuales
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        getSampleRate(), cutoff, q);
}

//==============================================================================
// PUSHSAMPLEINTOFIFO - Agrega una muestra al buffer circular
// Este buffer alimenta la FFT para el espectro
//==============================================================================
void VocalisAudioProcessor::pushSampleIntoFIFO(float sample)
{
    if (circularBuffer.getNumSamples() > 0)
    {
        circularBuffer.setSample(0, circularBufferWritePos, sample);
        circularBufferWritePos++;
        // Cuando llega al final, vuelve al principio (buffer circular)
        if (circularBufferWritePos >= circularBuffer.getNumSamples())
            circularBufferWritePos = 0;
    }
}

//==============================================================================
// PERFORMFFTIFREADY - Ejecuta la FFT cuando hay 2048 muestras acumuladas
// Convierte audio de dominio tiempo ? dominio frecuencia (espectro)
//==============================================================================
void VocalisAudioProcessor::performFFTIfReady()
{
    // ¿Hay suficientes muestras?
    if (circularBuffer.getNumSamples() < fftSize) return;
    // ¿Están los buffers correctamente dimensionados?
    if (fftTimeDomain.size() < fftSize * 2 || fftFreqDomain.size() < fftSize * 2
        || spectrumMagnitudes.size() < numSpectrumBins) return;

    // --- Leer 2048 muestras del buffer circular con ventana de Hann ---
    for (int i = 0; i < fftSize; ++i)
    {
        // Leer hacia atrás desde writePos (las últimas 2048 muestras)
        int readPos = circularBufferWritePos - fftSize + i;
        if (readPos < 0) readPos += circularBuffer.getNumSamples();
        if (readPos < 0 || readPos >= circularBuffer.getNumSamples()) return;

        float sample = circularBuffer.getSample(0, readPos);
        // Ventana de Hann: suaviza los bordes para evitar artefactos
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
        fftTimeDomain[i * 2] = sample * window;  // Parte real
        fftTimeDomain[i * 2 + 1] = 0.0f;              // Parte imaginaria (cero)
    }

    // --- Copiar al buffer de salida y ejecutar FFT ---
    std::copy(fftTimeDomain.begin(), fftTimeDomain.end(), fftFreqDomain.begin());
    forwardFFT->performRealOnlyForwardTransform(fftFreqDomain.data(), true);

    // --- Calcular magnitudes (espectro de potencia) ---
    for (int i = 0; i < numSpectrumBins; ++i)
    {
        float real = fftFreqDomain[i * 2];
        float imag = fftFreqDomain[i * 2 + 1];
        // Magnitud = ?(real² + imag²), normalizada por fftSize
        spectrumMagnitudes[i] = std::sqrt(real * real + imag * imag) / fftSize;
    }

    // --- Pasar datos al buffer thread-safe (FIFO) para la UI ---
    int start1, size1, start2, size2;
    spectrumFifo.prepareToWrite(numSpectrumBins, start1, size1, start2, size2);
    if (size1 > 0 && start1 >= 0 && start1 + size1 <= (int)spectrumForUI.size())
        std::copy(spectrumMagnitudes.begin(), spectrumMagnitudes.begin() + size1, spectrumForUI.begin() + start1);
    if (size2 > 0 && start2 >= 0 && start2 + size2 <= (int)spectrumForUI.size())
        std::copy(spectrumMagnitudes.begin() + size1, spectrumMagnitudes.begin() + size1 + size2, spectrumForUI.begin() + start2);
    spectrumFifo.finishedWrite(size1 + size2);
}

//==============================================================================
// GETSPECTRUMDATA - El Editor la llama 30 veces/segundo para dibujar el espectro
//==============================================================================
void VocalisAudioProcessor::getSpectrumData(std::vector<float>& dest)
{
    dest.assign(numSpectrumBins, 0.0f);  // Inicializar con ceros
    int ready = spectrumFifo.getNumReady();
    if (ready >= numSpectrumBins)  // ¿Hay datos nuevos?
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

//==============================================================================
// GETWAVEFORMDATA - El Editor la llama para dibujar el osciloscopio
//==============================================================================
void VocalisAudioProcessor::getWaveformData(std::vector<float>& dest)
{
    juce::ScopedLock sl(waveformLock);  // Thread-safe
    dest = waveformForUI;                // Copia simple
}

//==============================================================================
// PROCESSCOMPRESSOR - Compresor feed-forward con detector RMS
// Reduce picos y sube lo bajito para nivelar la voz
//==============================================================================
void VocalisAudioProcessor::processCompressor(juce::AudioBuffer<float>& buffer)
{
    // Leer valores de las perillas (con valores por defecto si son nulos)
    float threshold = compThresholdParam ? compThresholdParam->get() : 0.0f;
    float ratio = compRatioParam ? compRatioParam->get() : 4.0f;
    float makeupGain = compGainParam ? compGainParam->get() : 0.0f;

    // Si el threshold está en 0 dB, no comprime (bypass)
    if (threshold >= 0.0f) return;

    // Procesar cada canal (0 = L, 1 = R)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);  // Acceso directo a muestras

        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            // --- Detector RMS (raíz cuadrada media) ---
            float inputSquared = data[s] * data[s];
            // Ataque rápido, release lento (suavizado exponencial)
            if (inputSquared > compRmsFiltered)
                compRmsFiltered += compAlphaAttack * (inputSquared - compRmsFiltered);
            else
                compRmsFiltered += compAlphaRelease * (inputSquared - compRmsFiltered);

            // Convertir a dB (con protección de división por cero)
            float rmsDb = juce::Decibels::gainToDecibels(std::sqrt(compRmsFiltered + 1e-10f));

            // --- Calcular reducción de ganancia ---
            float gainReductionDb = 0.0f;
            if (rmsDb > threshold)
            {
                float overshoot = rmsDb - threshold;           // Cuánto se pasó
                gainReductionDb = overshoot * (1.0f - 1.0f / ratio); // Curva de compresión
            }
            compressorReduction.store(gainReductionDb);  // Para mostrar en UI

            // --- Aplicar ganancia (reducción + maquillaje) ---
            float gainDb = -gainReductionDb + makeupGain;
            data[s] *= juce::Decibels::decibelsToGain(gainDb);  // dB ? lineal
        }
    }
}

//==============================================================================
// PROCESSBLOCK - EL CORAZÓN DEL PLUGIN
// Se llama cada vez que hay audio para procesar
// Cadena: Mic ? Compresor ? Reverb ? Filtro ? Delay ? Salida
//==============================================================================
void VocalisAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);  // No usamos MIDI

    // Limpiar canales extra (si el DAW manda más de 2)
    for (int channel = buffer.getNumChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() == 0) return;  // Sin entrada, salir

    // --- Medir nivel RMS de entrada ---
    float rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    currentRMS.store(rms);  // Thread-safe
    if (rmsParam) rmsParam->setValueNotifyingHost(rms);

    // --- Alimentar buffer circular para FFT (espectro) ---
    auto* channelData = buffer.getReadPointer(0);  // Canal izquierdo/mono
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        pushSampleIntoFIFO(channelData[sample]);
    performFFTIfReady();  // Ejecutar FFT si hay 2048 muestras

    // --- Guardar waveform para osciloscopio (antes de procesar) ---
    {
        juce::ScopedLock sl(waveformLock);
        // Diezmar: tomar 512 muestras equiespaciadas del bloque
        for (int s = 0; s < buffer.getNumSamples(); s += juce::jmax(1, buffer.getNumSamples() / waveformUISize))
        {
            int idx = s * waveformUISize / buffer.getNumSamples();
            if (idx < waveformUISize)
                waveformForUI[idx] = channelData[s];
        }
    }

    // --- Expandir mono a estéreo si es necesario ---
    if (buffer.getNumChannels() < 2 && getTotalNumOutputChannels() >= 2)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());

    // ============================================================
    // CADENA DE PROCESAMIENTO DE AUDIO
    // ============================================================

    // 1. COMPRESOR - Nivela el volumen
    processCompressor(buffer);

    // 2. REVERB - Agrega ambiente espacial (si Mix > 0)
    float rvMix = reverbMixParam ? reverbMixParam->get() / 100.0f : 0.0f;
    float rvSize = reverbRoomSizeParam ? reverbRoomSizeParam->get() : 0.5f;
    float rvDecay = reverbDecayParam ? reverbDecayParam->get() : 1.5f;
    reverbRoomSize.store(rvSize);
    reverbDecay.store(rvDecay);
    reverbMix.store(rvMix * 100.0f);

    if (rvMix > 0.001f)  // Solo procesar si la mezcla es significativa
    {
        juce::Reverb::Parameters rvParams;
        rvParams.roomSize = rvSize;        // 0=chico, 1=enorme
        rvParams.damping = 0.5f;          // Absorción de agudos (fijo)
        rvParams.wetLevel = rvMix;         // Señal con reverb
        rvParams.dryLevel = 1.0f - rvMix;  // Señal seca
        rvParams.width = 1.0f;          // Estéreo completo
        rvParams.freezeMode = 0.0f;          // Sin congelar
        reverb.setParameters(rvParams);

        juce::dsp::AudioBlock<float> rvBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> rvContext(rvBlock);
        reverb.process(rvContext);
    }

    // 3. FILTRO PASA-BAJOS - Corta frecuencias agudas
    updateFilter();  // Actualizar coeficientes según perillas
    {
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        lowPassFilter.process(context);
    }

    // 4. DELAY - Eco con buffer circular
    float dTime = delayTimeParam ? delayTimeParam->get() : 300.0f;
    float dFeedback = delayFeedbackParam ? delayFeedbackParam->get() / 100.0f : 0.0f;
    float dMix = delayMixParam ? delayMixParam->get() / 100.0f : 0.0f;
    delayTime.store(dTime);
    delayFeedback.store(dFeedback * 100.0f);
    delayMix.store(dMix * 100.0f);

    // Convertir ms a cantidad de muestras
    int delaySamples = static_cast<int>(dTime / 1000.0 * getSampleRate());
    delaySamples = juce::jlimit(0, delayBufferSize - 1, delaySamples);

    if (delaySamples > 0 && dMix > 0.001f)
    {
        auto* outL = buffer.getWritePointer(0);
        auto* outR = buffer.getWritePointer(1);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputL = outL[sample];
            float inputR = outR[sample];

            // Leer del buffer circular (posición actual - retardo)
            int readPos = delayWritePos - delaySamples;
            if (readPos < 0) readPos += delayBufferSize;

            float delayL = delayBuffer.getSample(0, readPos);
            float delayR = delayBuffer.getSample(1, readPos);

            // Escribir en buffer: entrada + feedback (realimentación)
            delayBuffer.setSample(0, delayWritePos, inputL + delayL * dFeedback);
            delayBuffer.setSample(1, delayWritePos, inputR + delayR * dFeedback);

            // Mezclar: seco + eco
            outL[sample] = inputL * (1.0f - dMix) + delayL * dMix;
            outR[sample] = inputR * (1.0f - dMix) + delayR * dMix;

            // Avanzar posición de escritura (circular)
            delayWritePos = (delayWritePos + 1) % delayBufferSize;
        }
    }
}

//==============================================================================
// CREACIÓN DEL EDITOR (interfaz gráfica)
//==============================================================================
bool VocalisAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* VocalisAudioProcessor::createEditor()
{
    return new VocalisAudioProcessorEditor(*this);
}

//==============================================================================
// GUARDAR ESTADO - Serializa los valores de todas las perillas a XML
//==============================================================================
void VocalisAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();              // Copiar estado actual
    std::unique_ptr<juce::XmlElement> xml(state.createXml());  // Convertir a XML
    copyXmlToBinary(*xml, destData);                  // Serializar a binario
}

//==============================================================================
// CARGAR ESTADO - Restaura los valores desde XML
//==============================================================================
void VocalisAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// FACTORY FUNCTION - Crea una instancia del plugin (llamada por JUCE)
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalisAudioProcessor();
}