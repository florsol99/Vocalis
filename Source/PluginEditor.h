// ============================================================
// VOCALIS by Florchis.io - PluginEditor.h
// Declaración de la interfaz gráfica del plugin
// ============================================================

#pragma once  // Evita inclusión múltiple

#include <juce_audio_processors/juce_audio_processors.h>  // Framework de plugins
#include "PluginProcessor.h"                               // Acceso al procesador

//==============================================================================
// CLASE DEL EDITOR (INTERFAZ GRÁFICA)
// Hereda de AudioProcessorEditor (ventana del plugin)
// Hereda de Timer (para repintar 30 veces por segundo)
//==============================================================================
class VocalisAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer       // ? Timer para refresco
{
public:
    // ============================================================
    // CONSTRUCTOR / DESTRUCTOR
    // ============================================================

    // Recibe referencia al procesador para leer parámetros y datos
    VocalisAudioProcessorEditor(VocalisAudioProcessor&);

    // Detiene el timer al cerrar la ventana
    ~VocalisAudioProcessorEditor() override;

    // ============================================================
    // MÉTODOS DE DIBUJO (llamados por JUCE)
    // ============================================================

    // Dibuja TODA la interfaz: fondo, perillas, espectro, osciloscopio, VU
    void paint(juce::Graphics&) override;

    // Reposiciona todos los elementos cuando la ventana cambia de tamaño
    void resized() override;

private:
    // ============================================================
    // CALLBACK DEL TIMER
    // Se ejecuta 30 veces por segundo para refrescar la UI
    // Solo repinta si la ventana es visible
    // ============================================================
    void timerCallback() override
    {
        if (isVisible())
            repaint();
    }

    // ============================================================
    // REFERENCIA AL PROCESADOR
    // Para leer valores de perillas y datos de visualización
    // ============================================================
    VocalisAudioProcessor& audioProcessor;

    // ============================================================
    // MÉTODOS PRIVADOS DE DIBUJO
    // Cada uno dibuja una parte específica de la interfaz
    // ============================================================

    // Medidor de volumen VU (barra vertical verde/amarillo/rojo)
    void drawVUMeter(juce::Graphics& g, juce::Rectangle<int> bounds);

    // Espectro de frecuencias logarítmico (1024 barras de colores)
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<int> bounds);

    // Osciloscopio (forma de onda verde en tiempo real)
    void drawOscilloscope(juce::Graphics& g, juce::Rectangle<int> bounds);

    // ============================================================
    // BUFFERS LOCALES PARA DATOS DE VISUALIZACIÓN
    // El procesador llena estos vectores, el editor los dibuja
    // ============================================================
    std::vector<float> spectrumData;   // 1024 bins de magnitud FFT
    std::vector<float> waveformData;   // 512 muestras para el osciloscopio

    // ============================================================
    // PERILLAS Y ETIQUETAS - SECCIÓN FILTRO
    // ============================================================
    juce::Slider cutoffSlider;         // Perilla de frecuencia de corte
    juce::Slider resonanceSlider;      // Perilla de resonancia (Q)
    juce::Label cutoffLabel;           // Texto "Cutoff"
    juce::Label resonanceLabel;        // Texto "Reson"
    // Attachments: conectan perilla ? parámetro (bidireccional)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    // ============================================================
    // PERILLAS Y ETIQUETAS - SECCIÓN DELAY
    // ============================================================
    juce::Slider delayTimeSlider;      // Tiempo de retardo (ms)
    juce::Slider delayFeedbackSlider;  // Realimentación (%)
    juce::Slider delayMixSlider;       // Mezcla eco/voz (%)
    juce::Label delayTimeLabel;        // Texto "Tiempo"
    juce::Label delayFeedbackLabel;    // Texto "Feedb"
    juce::Label delayMixLabel;         // Texto "Mezcla"
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;

    // ============================================================
    // PERILLAS Y ETIQUETAS - SECCIÓN COMPRESOR
    // ============================================================
    juce::Slider compThresholdSlider;  // Umbral (dB)
    juce::Slider compRatioSlider;      // Relación (:1)
    juce::Slider compGainSlider;       // Ganancia maquillaje (dB)
    juce::Label compThresholdLabel;    // Texto "Thresh"
    juce::Label compRatioLabel;        // Texto "Ratio"
    juce::Label compGainLabel;         // Texto "Gain"
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compGainAttachment;

    // ============================================================
    // PERILLAS Y ETIQUETAS - SECCIÓN REVERB
    // ============================================================
    juce::Slider reverbRoomSizeSlider; // Tamaño de sala (0-1)
    juce::Slider reverbDecaySlider;    // Decaimiento (segundos)
    juce::Slider reverbMixSlider;      // Mezcla reverb/voz (%)
    juce::Label reverbRoomSizeLabel;   // Texto "Room"
    juce::Label reverbDecayLabel;      // Texto "Decay"
    juce::Label reverbMixLabel;        // Texto "Mix"
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbRoomSizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAttachment;

    // ============================================================
    // COLORES PARA EL VU METER
    // ============================================================
    juce::Colour meterColorLow{ juce::Colours::green };  // -60 a -30 dB
    juce::Colour meterColorMid{ juce::Colours::yellow };  // -30 a -12 dB
    juce::Colour meterColorHigh{ juce::Colours::red };  // -12 a 0 dB
    juce::Colour textColor{ juce::Colours::white };  // Texto general

    // ============================================================
    // MACRO DE JUCE
    // Prohíbe copiar el objeto + detecta memory leaks en debug
    // ============================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalisAudioProcessorEditor)
};