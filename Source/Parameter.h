// =============================================================================
//  Parameter.h – alle Regler des RR-1 an einer Stelle
//
//  Jeder Regler ist ein „Parameter“ mit Kennung (ID), Namen, Wertebereich
//  und Grundeinstellung. Ableton sieht diese Parameter, kann sie speichern
//  und automatisieren (Configure-Modus / Automationshüllkurven).
//
//  Tipp: Wer einen Regler hinzufügen möchte, legt ihn hier an, verwendet ihn
//  in PluginProcessor.cpp und setzt ihn in PluginEditor.cpp auf die Oberfläche.
// =============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace rr1
{

// Hilfsfunktion: Text mit Umlauten korrekt an JUCE übergeben
inline juce::String T (const char* utf8) { return juce::String::fromUTF8 (utf8); }

namespace ID
{
    inline constexpr const char* osz1Lage      = "osz1_lage";
    inline constexpr const char* osz1Form      = "osz1_form";
    inline constexpr const char* osz2Lage      = "osz2_lage";
    inline constexpr const char* osz2Form      = "osz2_form";
    inline constexpr const char* osz2Halbton   = "osz2_halbton";
    inline constexpr const char* osz2Fein      = "osz2_fein";
    inline constexpr const char* pegelOsz1     = "mix_osz1";
    inline constexpr const char* pegelOsz2     = "mix_osz2";
    inline constexpr const char* pegelRauschen = "mix_rauschen";
    inline constexpr const char* uebersteuern  = "mix_drive";
    inline constexpr const char* cutoff        = "flt_cutoff";
    inline constexpr const char* resonanz      = "flt_resonanz";
    inline constexpr const char* filterMenge   = "flt_huellkurve";
    inline constexpr const char* keytrack      = "flt_keytrack";
    inline constexpr const char* fA = "fhk_attack";
    inline constexpr const char* fD = "fhk_decay";
    inline constexpr const char* fS = "fhk_sustain";
    inline constexpr const char* fR = "fhk_release";
    inline constexpr const char* aA = "lhk_attack";
    inline constexpr const char* aD = "lhk_decay";
    inline constexpr const char* aS = "lhk_sustain";
    inline constexpr const char* aR = "lhk_release";
    inline constexpr const char* lautstaerke   = "aus_lautstaerke";
    inline constexpr const char* anschlag      = "aus_anschlag";
    inline constexpr const char* lfoTempo      = "lfo_tempo";
    inline constexpr const char* lfoForm       = "lfo_form";
    inline constexpr const char* lfoTonhoehe   = "lfo_tonhoehe";
    inline constexpr const char* lfoFilter     = "lfo_filter";
    inline constexpr const char* glide         = "st_glide";
    inline constexpr const char* legato        = "st_legato";
    inline constexpr const char* drift         = "st_drift";
}

// ------------------------------------------------------------- Anzeige ----
inline juce::String zeigeZeit (float s, int)
{
    return s < 1.0f ? juce::String (juce::roundToInt (s * 1000.0f)) + " ms"
                    : juce::String (s, 2) + " s";
}
inline juce::String zeigeProzent (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; }
inline juce::String zeigeHz (float hz, int)
{
    return hz < 1000.0f ? juce::String (hz, hz < 10.0f ? 2 : 0) + " Hz"
                        : juce::String (hz / 1000.0f, 1) + " kHz";
}

// ------------------------------------------------------------- Aufbau -----
inline juce::AudioProcessorValueTreeState::ParameterLayout erstelleParameter()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto prozent = [&] (const char* id, const char* name, float standard)
    {
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, T (name), NormalisableRange<float> (0.0f, 1.0f), standard,
            AudioParameterFloatAttributes().withStringFromValueFunction (zeigeProzent)));
    };

    auto zeit = [&] (const char* id, const char* name, float min, float max, float mitte, float standard)
    {
        NormalisableRange<float> bereich (min, max);
        bereich.setSkewForCentre (mitte);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, T (name), bereich, standard,
            AudioParameterFloatAttributes().withStringFromValueFunction (zeigeZeit)));
    };

    const StringArray lagen  { "32'", "16'", "8'", "4'", "2'" };
    const StringArray formen { T ("Dreieck"), T ("Sägezahn"), T ("Rechteck"), T ("Puls") };

    // Oszillatoren
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::osz1Lage, 1 }, T ("Osz 1 Fußlage"), lagen, 2));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::osz1Form, 1 }, T ("Osz 1 Wellenform"), formen, 1));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::osz2Lage, 1 }, T ("Osz 2 Fußlage"), lagen, 2));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::osz2Form, 1 }, T ("Osz 2 Wellenform"), formen, 1));
    layout.add (std::make_unique<AudioParameterInt> (ParameterID { ID::osz2Halbton, 1 }, T ("Osz 2 Halbton"), -12, 12, 0,
        AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int)
            { return (v > 0 ? "+" : "") + String (v); })));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ID::osz2Fein, 1 }, T ("Osz 2 Feinstimmung"),
        NormalisableRange<float> (-50.0f, 50.0f), 7.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
            { return (v > 0.0f ? "+" : "") + String (roundToInt (v)) + " ct"; })));

    // Mixer
    prozent (ID::pegelOsz1,     "Pegel Osz 1",   0.8f);
    prozent (ID::pegelOsz2,     "Pegel Osz 2",   0.6f);
    prozent (ID::pegelRauschen, "Pegel Rauschen", 0.0f);
    prozent (ID::uebersteuern,  "Übersteuern",   0.3f);

    // Filter
    {
        NormalisableRange<float> bereich (20.0f, 18000.0f);
        bereich.setSkewForCentre (800.0f);
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ID::cutoff, 1 }, T ("Filter Cutoff"),
            bereich, 1200.0f, AudioParameterFloatAttributes().withStringFromValueFunction (zeigeHz)));
    }
    prozent (ID::resonanz,    "Filter Resonanz",  0.2f);
    prozent (ID::filterMenge, "Filter Hüllkurve", 0.4f);
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::keytrack, 1 }, T ("Filter Keytrack"),
        StringArray { T ("Aus"), T ("Halb"), T ("Voll") }, 1));

    zeit (ID::fA, "Filter Attack",  0.001f, 10.0f, 0.5f, 0.005f);
    zeit (ID::fD, "Filter Decay",   0.005f, 10.0f, 0.6f, 0.4f);
    prozent (ID::fS, "Filter Sustain", 0.3f);
    zeit (ID::fR, "Filter Release", 0.005f, 10.0f, 0.6f, 0.3f);

    // Lautstärke
    zeit (ID::aA, "Laut Attack",  0.001f, 10.0f, 0.5f, 0.003f);
    zeit (ID::aD, "Laut Decay",   0.005f, 10.0f, 0.6f, 0.3f);
    prozent (ID::aS, "Laut Sustain", 0.8f);
    zeit (ID::aR, "Laut Release", 0.005f, 10.0f, 0.6f, 0.25f);

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ID::lautstaerke, 1 }, T ("Lautstärke"),
        NormalisableRange<float> (-48.0f, 6.0f), -10.0f,
        AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
            { return String (v, 1) + " dB"; })));
    prozent (ID::anschlag, "Anschlagdynamik", 0.0f);

    // Modulation
    {
        NormalisableRange<float> bereich (0.05f, 20.0f);
        bereich.setSkewForCentre (2.0f);
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ID::lfoTempo, 1 }, T ("LFO Tempo"),
            bereich, 5.0f, AudioParameterFloatAttributes().withStringFromValueFunction (zeigeHz)));
    }
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ID::lfoForm, 1 }, T ("LFO Wellenform"),
        StringArray { T ("Dreieck"), T ("Rechteck"), T ("Zufall") }, 0));
    prozent (ID::lfoTonhoehe, "LFO auf Tonhöhe", 0.0f);
    prozent (ID::lfoFilter,   "LFO auf Filter",  0.0f);

    // Steuerung
    zeit (ID::glide, "Glide", 0.0f, 2.0f, 0.25f, 0.0f);
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { ID::legato, 1 }, T ("Legato"), true,
        AudioParameterBoolAttributes().withStringFromValueFunction ([] (bool an, int)
            { return an ? T ("An") : T ("Aus"); })));
    prozent (ID::drift, "Analoge Drift", 0.2f);

    return layout;
}

} // namespace rr1
