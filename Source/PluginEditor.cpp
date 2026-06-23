// ============================================================
// VOCALIS by Florchis.io - PluginEditor.cpp
// Implementación de la interfaz gráfica (perillas, espectro, osciloscopio, VU)
// ============================================================

#include "PluginProcessor.h"   // Acceso a los parámetros y datos
#include "PluginEditor.h"      // incluye la clase creada

//==============================================================================
// CONSTRUCTOR - Crea TODOS los elementos visuales del plugin
//==============================================================================
VocalisAudioProcessorEditor::VocalisAudioProcessorEditor(VocalisAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)  // Guarda referencia al procesador
{
    // --- Tamaño de la ventana ---
    setSize(950, 580);                              // Ancho x Alto inicial
    setResizable(true, true);                       // Permitir redimensionar
    setResizeLimits(750, 500, 1600, 1000);          // Mínimo y máximo

    // ============================================================
    // PERILLA: CUTOFF (Frecuencia de corte del filtro)
    // ============================================================
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); // Perilla redonda
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);  // Número debajo
    cutoffSlider.setRange(200.0, 20000.0, 1.0);     // Rango: 200 Hz a 20 kHz
    cutoffSlider.setSkewFactor(0.3);                // Escala logarítmica (más resolución en graves)
    cutoffSlider.setValue(10000.0);                  // Valor inicial: 10 kHz (filtro abierto)
    cutoffSlider.setTextValueSuffix(" Hz");          // Sufijo: "Hz"
    addAndMakeVisible(cutoffSlider);                 // Hacer visible

    // Conectar la perilla al parámetro del procesador (bidireccional)
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "cutoff", cutoffSlider);

    // Etiqueta debajo de la perilla
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    cutoffLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(cutoffLabel);

    // ============================================================
    // PERILLA: RESONANCIA (Q del filtro - pico antes del corte)
    // ============================================================
    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    resonanceSlider.setRange(0.5, 5.0, 0.01);       // Rango: 0.5 (suave) a 5.0 (metálico)
    resonanceSlider.setValue(0.707);                  // Valor inicial: Butterworth (natural)
    resonanceSlider.setTextValueSuffix(" Q");
    addAndMakeVisible(resonanceSlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "resonance", resonanceSlider);
    resonanceLabel.setText("Reson", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    resonanceLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(resonanceLabel);

    // ============================================================
    // PERILLA: DELAY TIME (Tiempo de retardo del eco en ms)
    // ============================================================
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayTimeSlider.setRange(0.0, 1000.0, 1.0);      // Rango: 0 a 1000 ms
    delayTimeSlider.setValue(300.0);                   // Valor inicial: 300 ms
    delayTimeSlider.setTextValueSuffix(" ms");
    addAndMakeVisible(delayTimeSlider);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "delayTime", delayTimeSlider);
    delayTimeLabel.setText("Tiempo", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayTimeLabel);

    // ============================================================
    // PERILLA: DELAY FEEDBACK (Realimentación - cuántas repeticiones)
    // ============================================================
    delayFeedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayFeedbackSlider.setRange(0.0, 90.0, 1.0);    // Rango: 0% a 90%
    delayFeedbackSlider.setValue(40.0);                // Valor inicial: 40%
    delayFeedbackSlider.setTextValueSuffix(" %");
    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "delayFeedback", delayFeedbackSlider);
    delayFeedbackLabel.setText("Feedb", juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    delayFeedbackLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayFeedbackLabel);

    // ============================================================
    // PERILLA: DELAY MIX (Balance entre voz seca y eco)
    // ============================================================
    delayMixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    delayMixSlider.setRange(0.0, 100.0, 1.0);        // Rango: 0% a 100%
    delayMixSlider.setValue(30.0);                    // Valor inicial: 30%
    delayMixSlider.setTextValueSuffix(" %");
    addAndMakeVisible(delayMixSlider);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "delayMix", delayMixSlider);
    delayMixLabel.setText("Mezcla", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centred);
    delayMixLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(delayMixLabel);

    // ============================================================
    // PERILLA: COMPRESOR THRESHOLD (Umbral - cuándo empieza a comprimir)
    // ============================================================
    compThresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compThresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compThresholdSlider.setRange(-40.0, 0.0, 1.0);   // Rango: -40 dB a 0 dB
    compThresholdSlider.setValue(-20.0);               // Valor inicial: -20 dB
    compThresholdSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(compThresholdSlider);
    compThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "compThreshold", compThresholdSlider);
    compThresholdLabel.setText("Thresh", juce::dontSendNotification);
    compThresholdLabel.setJustificationType(juce::Justification::centred);
    compThresholdLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compThresholdLabel);

    // ============================================================
    // PERILLA: COMPRESOR RATIO (Relación de compresión)
    // ============================================================
    compRatioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compRatioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compRatioSlider.setRange(1.0, 20.0, 0.5);        // Rango: 1:1 a 20:1
    compRatioSlider.setValue(4.0);                    // Valor inicial: 4:1
    compRatioSlider.setTextValueSuffix(":1");
    addAndMakeVisible(compRatioSlider);
    compRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "compRatio", compRatioSlider);
    compRatioLabel.setText("Ratio", juce::dontSendNotification);
    compRatioLabel.setJustificationType(juce::Justification::centred);
    compRatioLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compRatioLabel);

    // ============================================================
    // PERILLA: COMPRESOR GAIN (Ganancia de maquillaje post-compresión)
    // ============================================================
    compGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    compGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    compGainSlider.setRange(0.0, 24.0, 0.5);         // Rango: 0 dB a 24 dB
    compGainSlider.setValue(6.0);                     // Valor inicial: 6 dB
    compGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(compGainSlider);
    compGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "compGain", compGainSlider);
    compGainLabel.setText("Gain", juce::dontSendNotification);
    compGainLabel.setJustificationType(juce::Justification::centred);
    compGainLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(compGainLabel);

    // ============================================================
    // PERILLA: REVERB ROOM SIZE (Tamaño de la sala)
    // ============================================================
    reverbRoomSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbRoomSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    reverbRoomSizeSlider.setRange(0.0, 1.0, 0.01);   // Rango: 0 (chico) a 1 (enorme)
    reverbRoomSizeSlider.setValue(0.5);                // Valor inicial: sala media
    addAndMakeVisible(reverbRoomSizeSlider);
    reverbRoomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "reverbRoomSize", reverbRoomSizeSlider);
    reverbRoomSizeLabel.setText("Room", juce::dontSendNotification);
    reverbRoomSizeLabel.setJustificationType(juce::Justification::centred);
    reverbRoomSizeLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(reverbRoomSizeLabel);

    // ============================================================
    // PERILLA: REVERB DECAY (Duración de la cola de reverb)
    // ============================================================
    reverbDecaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    reverbDecaySlider.setRange(0.1, 10.0, 0.1);      // Rango: 0.1 s a 10 s
    reverbDecaySlider.setValue(1.5);                   // Valor inicial: 1.5 s
    reverbDecaySlider.setTextValueSuffix(" s");
    addAndMakeVisible(reverbDecaySlider);
    reverbDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "reverbDecay", reverbDecaySlider);
    reverbDecayLabel.setText("Decay", juce::dontSendNotification);
    reverbDecayLabel.setJustificationType(juce::Justification::centred);
    reverbDecayLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(reverbDecayLabel);

    // ============================================================
    // PERILLA: REVERB MIX (Balance entre voz seca y reverb)
    // ============================================================
    reverbMixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 55, 16);
    reverbMixSlider.setRange(0.0, 100.0, 1.0);       // Rango: 0% a 100%
    reverbMixSlider.setValue(20.0);                    // Valor inicial: 20%
    reverbMixSlider.setTextValueSuffix(" %");
    addAndMakeVisible(reverbMixSlider);
    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "reverbMix", reverbMixSlider);
    reverbMixLabel.setText("Mix", juce::dontSendNotification);
    reverbMixLabel.setJustificationType(juce::Justification::centred);
    reverbMixLabel.setFont(juce::Font("Helvetica", 10.0f, juce::Font::bold));
    addAndMakeVisible(reverbMixLabel);

    // --- Timer que repinta la UI 30 veces por segundo ---
    startTimerHz(30);
}

//==============================================================================
// DESTRUCTOR - Detiene el timer al cerrar
//==============================================================================
VocalisAudioProcessorEditor::~VocalisAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
// PAINT - Dibuja TODA la interfaz gráfica (se llama 30 veces/segundo)
//==============================================================================
void VocalisAudioProcessorEditor::paint(juce::Graphics& g)
{
    // --- Fondo con gradiente oscuro ---
    juce::ColourGradient bg(juce::Colour(28, 28, 33), 0, 0,           // Arriba
        juce::Colour(18, 18, 23), 0, getHeight(), false); // Abajo
    g.setGradientFill(bg);
    g.fillAll();

    // --- Área principal (sin título para no tapar el espectro) ---
    auto mainArea = getLocalBounds();

    // --- Panel de controles (210px a la derecha) ---
    auto controlsArea = mainArea.removeFromRight(210);
    g.setColour(juce::Colours::black.withAlpha(0.5f));     // Fondo semi-transparente
    g.fillRect(controlsArea);
    g.setColour(juce::Colours::white.withAlpha(0.08f));    // Línea separadora izquierda
    g.drawLine(controlsArea.getX(), controlsArea.getY(), controlsArea.getX(), controlsArea.getBottom(), 1.0f);

    // --- TÍTULOS DE SECCIÓN (cyan) ---
    g.setFont(juce::Font("Helvetica", 11.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));

    // "FILTRO" - posición Y=25
    g.drawText("FILTRO", controlsArea.getX(), controlsArea.getY() + 25, controlsArea.getWidth(), 16, juce::Justification::centred);

    // Línea separadora 1 (debajo de FILTRO)
    int sepY1 = controlsArea.getY() + 135;
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX() + 15, sepY1, controlsArea.getRight() - 15, sepY1, 0.5f);

    // "DELAY"
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("DELAY", controlsArea.getX(), sepY1 + 5, controlsArea.getWidth(), 16, juce::Justification::centred);

    // Línea separadora 2 (debajo de DELAY)
    int sepY2 = controlsArea.getY() + 220;
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX() + 15, sepY2, controlsArea.getRight() - 15, sepY2, 0.5f);

    // "COMPRESOR" (un poco más abajo para que respire)
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("COMPRESOR", controlsArea.getX(), sepY2 + 30, controlsArea.getWidth(), 16, juce::Justification::centred);

    // Línea separadora 3 (debajo de COMPRESOR)
    int sepY3 = controlsArea.getY() + 365;
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawLine(controlsArea.getX() + 15, sepY3, controlsArea.getRight() - 15, sepY3, 0.5f);

    // "REVERB"
    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawText("REVERB", controlsArea.getX(), sepY3 + 10, controlsArea.getWidth(), 16, juce::Justification::centred);

    // --- VU METER (barra izquierda, 45px ancho) ---
    auto vuArea = mainArea.removeFromLeft(45);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRect(vuArea);
    drawVUMeter(g, vuArea);

    // --- ÁREA DE VISUALIZACIÓN (espectro + osciloscopio) ---
    auto visArea = mainArea.reduced(6, 20);
    auto specArea = visArea.removeFromTop(visArea.getHeight() * 0.58); // 58% espectro
    drawSpectrum(g, specArea);
    drawOscilloscope(g, visArea);                                       // 42% osciloscopio

    // --- PANEL INFERIOR (información de valores) ---
    auto bottomArea = getLocalBounds().removeFromBottom(26);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRect(bottomArea);
    g.setColour(juce::Colours::cyan.withAlpha(0.3f));
    g.drawLine(bottomArea.getX(), bottomArea.getY(), bottomArea.getRight(), bottomArea.getY(), 1.0f);

    // --- TEXTO CON VALORES ACTUALES ---
    float rms = audioProcessor.getRMSLevel();
    float db = juce::Decibels::gainToDecibels(rms, -60.0f);  // Convertir a dB
    float cutoff = audioProcessor.getCutoffFreq();
    float q = audioProcessor.getResonance();
    float dTime = audioProcessor.getDelayTime();
    float dFb = audioProcessor.getDelayFeedback();
    float dMix = audioProcessor.getDelayMix();
    float compRed = audioProcessor.getCompressorReduction();

    juce::String info;
    info << "In: " << juce::String(db, 1) << " dB"                   // Nivel entrada
        << "  |  Cut: " << juce::String(cutoff / 1000.0, 2) << "k" // Cutoff en kHz
        << "  Q: " << juce::String(q, 2)                            // Resonancia
        << "  |  Del: " << juce::String(dTime, 0) << "ms"          // Tiempo delay
        << "  Fb: " << juce::String(dFb, 0) << "%"                 // Feedback
        << "  |  Comp: " << juce::String(compRed, 1) << "dB"       // Reducción compresor
        << "  |  Rev: " << juce::String(audioProcessor.getReverbMix(), 0) << "%"; // Mix reverb

    g.setFont(juce::Font("Helvetica", 10.0f, juce::Font::plain));
    g.setColour(juce::Colours::lightgrey);
    g.drawText(info, bottomArea.reduced(8, 2), juce::Justification::centredLeft);

    // --- FIRMA "Vocalis by Florchis.io" EN DORADO (abajo derecha) ---
    g.setFont(juce::Font("Helvetica", 15.0f, juce::Font::bold));
    g.setColour(juce::Colour(212, 175, 55));  // RGB dorado
    g.drawText("Vocalis by Florchis.io",
        bottomArea.getRight() - 195, bottomArea.getY() + 4,
        190, bottomArea.getHeight() - 8,
        juce::Justification::centred);
}

//==============================================================================
// RESIZED - Posiciona TODAS las perillas cuando la ventana cambia de tamaño
//==============================================================================
void VocalisAudioProcessorEditor::resized()
{
    auto controlsArea = getLocalBounds().removeFromRight(210);
    int knobSize = 60;                       // Todas las perillas miden 60x60
    int spacing = 10;                        // Espacio entre perillas
    int centerX = controlsArea.getCentreX(); // Centro del panel
    int x, y;

    // --- FILTRO (2 perillas en fila horizontal) ---
    y = controlsArea.getY() + 65;
    int filaWidth = knobSize * 2 + spacing;  // Ancho total de 2 perillas
    x = centerX - filaWidth / 2;             // Centrar
    cutoffSlider.setBounds(x, y, knobSize, knobSize);
    cutoffLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    resonanceSlider.setBounds(x, y, knobSize, knobSize);
    resonanceLabel.setBounds(x, y - 16, knobSize, 16);

    // --- DELAY (3 perillas en fila horizontal) ---
    y = controlsArea.getY() + 170;
    int fila3Width = knobSize * 3 + spacing * 2; // Ancho total de 3 perillas
    x = centerX - fila3Width / 2;
    delayTimeSlider.setBounds(x, y, knobSize, knobSize);
    delayTimeLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    delayFeedbackSlider.setBounds(x, y, knobSize, knobSize);
    delayFeedbackLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    delayMixSlider.setBounds(x, y, knobSize, knobSize);
    delayMixLabel.setBounds(x, y - 16, knobSize, 16);

    // --- COMPRESOR (3 perillas en fila horizontal) ---
    y = controlsArea.getY() + 300;
    x = centerX - fila3Width / 2;
    compThresholdSlider.setBounds(x, y, knobSize, knobSize);
    compThresholdLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    compRatioSlider.setBounds(x, y, knobSize, knobSize);
    compRatioLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    compGainSlider.setBounds(x, y, knobSize, knobSize);
    compGainLabel.setBounds(x, y - 16, knobSize, 16);

    // --- REVERB (3 perillas en fila horizontal) ---
    y = controlsArea.getY() + 430;
    x = centerX - fila3Width / 2;
    reverbRoomSizeSlider.setBounds(x, y, knobSize, knobSize);
    reverbRoomSizeLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    reverbDecaySlider.setBounds(x, y, knobSize, knobSize);
    reverbDecayLabel.setBounds(x, y - 16, knobSize, 16);
    x += knobSize + spacing;
    reverbMixSlider.setBounds(x, y, knobSize, knobSize);
    reverbMixLabel.setBounds(x, y - 16, knobSize, 16);

    repaint(); // Redibujar todo
}

//==============================================================================
// DRAWVUMETER - Medidor de volumen clásico (verde ? amarillo ? rojo)
//==============================================================================
void VocalisAudioProcessorEditor::drawVUMeter(juce::Graphics& g, juce::Rectangle<int> meterBounds)
{
    if (meterBounds.getWidth() <= 10 || meterBounds.getHeight() <= 10) return;
    float rmsValue = audioProcessor.getRMSLevel();  // Obtener nivel actual

    auto bounds = meterBounds.reduced(4, 40);       // Márgenes
    if (bounds.isEmpty() || bounds.getHeight() <= 0) return;

    int mW = bounds.getWidth(), mH = bounds.getHeight();
    int mX = bounds.getX(), mY = bounds.getY();

    // Convertir RMS lineal a dB para la escala
    float dbValue = juce::Decibels::gainToDecibels(rmsValue, -60.0f);
    float norm = juce::jlimit(0.0f, 1.0f, juce::jmap(dbValue, -60.0f, 0.0f, 0.0f, 1.0f));

    // Fondo negro con borde sutil
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(mX, mY, mW, mH, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(mX, mY, mW, mH, 2.0f, 1.0f);

    // Color según nivel: verde (<50%), amarillo (<80%), rojo (>80%)
    juce::Colour mc;
    if (norm < 0.5f)       mc = meterColorLow;   // Verde
    else if (norm < 0.8f)  mc = meterColorMid;   // Amarillo
    else                   mc = meterColorHigh;  // Rojo

    // Barra de nivel con gradiente
    if (norm > 0.0f)
    {
        int fh = (int)(mH * norm);  // Altura proporcional al nivel
        juce::ColourGradient grad(mc.withAlpha(0.4f), mX, mY + mH,
            mc, mX, mY, false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(mX + 1, mY + mH - fh, mW - 2, fh, 1.0f);
    }

    // Líneas de referencia cada 20 dB
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    for (int db = -50; db <= -10; db += 20)
    {
        float yn = juce::jmap((float)db, -60.0f, 0.0f, 1.0f, 0.0f);
        int y = mY + mH - (int)(mH * yn);
        g.drawLine(mX - 2, y, mX + mW + 2, y, 0.5f);
    }

    // Etiqueta de dB actual
    g.setFont(7.0f);
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.drawText(juce::String(dbValue, 0), mX, mY - 12, mW, 10, juce::Justification::centred);

    // Etiqueta "VU"
    g.setFont(8.0f);
    g.setColour(textColor.withAlpha(0.6f));
    g.drawText("VU", mX, mY + mH + 2, mW, 12, juce::Justification::centred);
}

//==============================================================================
// DRAWSPECTRUM - Espectro de frecuencias logarítmico con línea de cutoff
//==============================================================================
void VocalisAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> specBounds)
{
    audioProcessor.getSpectrumData(spectrumData);  // 1024 bins de magnitud
    if (spectrumData.empty()) return;

    auto bounds = specBounds;
    bounds.removeFromBottom(22);  // Espacio para etiquetas de frecuencia
    if (bounds.isEmpty() || bounds.getHeight() <= 0) return;

    int numBins = (int)spectrumData.size();
    if (numBins <= 0) return;

    float sr = audioProcessor.getSampleRate();
    if (sr <= 0.0f) sr = 44100.0f;  // Por defecto si no está inicializado

    const float minF = 20.0f, maxF = 20000.0f;  // Rango visible
    float logMin = std::log(minF), logMax = std::log(maxF);
    float cutoffFreq = audioProcessor.getCutoffFreq();  // Para la línea roja

    // Fondo con bordes redondeados
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

    // --- Dibujar cada barra del espectro ---
    for (int i = 0; i < numBins; ++i)
    {
        if (i >= (int)spectrumData.size()) break;
        float freq = i * sr / audioProcessor.fftSize;  // Frecuencia del bin
        if (freq < minF || freq > maxF) continue;

        // Convertir magnitud a dB y normalizar (0 a 1)
        float dbVal = juce::Decibels::gainToDecibels(spectrumData[i], -80.0f);
        float normH = juce::jlimit(0.0f, 1.0f, juce::jmap(dbVal, -80.0f, 0.0f, 0.0f, 1.0f));

        if (normH > 0.001f)  // Solo dibujar si es visible
        {
            float barH = bounds.getHeight() * normH;

            // Posición X logarítmica
            float logF = std::log(juce::jlimit(minF, maxF, freq));
            float xN = (logF - logMin) / (logMax - logMin);
            float x = bounds.getX() + xN * bounds.getWidth();

            // Ancho de barra variable (más anchas en graves)
            float nf = (i + 1) * sr / audioProcessor.fftSize;
            nf = juce::jlimit(minF, maxF, nf);
            float nxN = (std::log(nf) - logMin) / (logMax - logMin);
            float nx = bounds.getX() + nxN * bounds.getWidth();
            float bw = juce::jmax(1.0f, nx - x - 0.5f);

            float y = bounds.getBottom() - barH;

            // Color según zona de frecuencia
            juce::Colour bc;
            if (freq > cutoffFreq)
                bc = juce::Colours::darkgrey.withAlpha(0.15f);       // Filtrado (gris)
            else if (freq >= 200.0f && freq <= 3500.0f)
                bc = juce::Colours::cyan.withAlpha(0.75f);           // Formantes (cyan)
            else if (freq < 200.0f)
                bc = juce::Colours::blue.withAlpha(0.45f);           // Graves (azul)
            else
                bc = juce::Colours::white.withAlpha(0.35f);          // Agudos (blanco)

            g.setColour(bc);
            g.fillRect(x, y, bw, barH);
        }
    }

    // --- Línea roja del cutoff con sombra ---
    if (cutoffFreq < maxF && cutoffFreq > minF)
    {
        float lc = std::log(cutoffFreq);
        float cxN = (lc - logMin) / (logMax - logMin);
        float cx = bounds.getX() + cxN * bounds.getWidth();

        // Sombra difusa
        g.setColour(juce::Colours::red.withAlpha(0.15f));
        g.drawLine(cx - 1, bounds.getY(), cx - 1, bounds.getBottom(), 3.0f);
        g.drawLine(cx + 1, bounds.getY(), cx + 1, bounds.getBottom(), 3.0f);

        // Línea principal
        g.setColour(juce::Colours::red.withAlpha(0.7f));
        g.drawLine(cx, bounds.getY(), cx, bounds.getBottom(), 1.5f);

        // Etiqueta con la frecuencia
        g.setFont(juce::Font("Helvetica", 9.0f, juce::Font::bold));
        g.setColour(juce::Colours::red.withAlpha(0.9f));
        juce::String cutoffText;
        if (cutoffFreq >= 1000.0f)
            cutoffText << juce::String(cutoffFreq / 1000.0f, 1) << "k";
        else
            cutoffText << juce::String(cutoffFreq, 0) << "Hz";
        g.drawText(cutoffText, cx - 22, bounds.getY() - 14, 44, 14, juce::Justification::centred);
    }

    // --- Líneas de referencia de frecuencia (50Hz, 100Hz, 200Hz...) ---
    int refs[] = { 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800 };
    for (int f : refs)
    {
        if (f >= minF && f <= maxF && f < sr / 2.0f)  // Solo si está en rango
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

//==============================================================================
// DRAWOSCILLOSCOPE - Forma de onda verde en tiempo real
//==============================================================================
void VocalisAudioProcessorEditor::drawOscilloscope(juce::Graphics& g, juce::Rectangle<int> oscBounds)
{
    audioProcessor.getWaveformData(waveformData);  // 512 muestras
    auto bounds = oscBounds.reduced(4, 4);
    if (bounds.isEmpty() || waveformData.empty() || bounds.getWidth() <= 0 || bounds.getHeight() <= 0) return;

    // Fondo
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

    // Área de dibujo (con margen para etiqueta)
    auto waveArea = bounds.reduced(4, 12);
    if (waveArea.isEmpty() || waveArea.getHeight() <= 0) return;

    float midY = waveArea.getCentreY();           // Centro vertical
    float h = waveArea.getHeight() * 0.45f;       // Amplitud máxima (45% del alto)
    int n = (int)waveformData.size();
    if (n <= 1) return;

    // Línea central de referencia
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(waveArea.getX(), midY, waveArea.getRight(), midY, 0.5f);

    // Construir el path de la forma de onda
    juce::Path path;
    path.startNewSubPath(waveArea.getX(), midY);
    float dx = waveArea.getWidth() / (float)(n - 1);  // Espaciado entre puntos

    for (int i = 0; i < n; ++i)
    {
        float x = waveArea.getX() + i * dx;
        float sample = waveformData[i];
        if (std::isnan(sample)) sample = 0.0f;  // Protección contra NaN
        // Mapear muestra [-1, 1] a posición Y [midY-h, midY+h]
        float y = juce::jlimit(midY - h, midY + h, midY - sample * h * 2.0f);
        path.lineTo(x, y);
    }

    // Dibujar la onda en verde
    g.setColour(juce::Colours::lime.withAlpha(0.7f));
    g.strokePath(path, juce::PathStrokeType(1.0f));

    // Etiqueta "OSCILOSCOPIO"
    g.setFont(juce::Font("Helvetica", 8.0f, juce::Font::plain));
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawText("OSCILOSCOPIO", bounds.getX(), bounds.getY() + 1, bounds.getWidth(), 12, juce::Justification::centred);
}