// =============================================================================
//  Presets.h – fertige Klänge zum Ausprobieren und Lernen
//
//  Jeder Klang startet von der Grundeinstellung aller Regler und ändert nur
//  die hier aufgeführten Werte. So sieht man genau, was den Klang ausmacht.
//  Werte stehen in echten Einheiten: Sekunden, Hertz, 0 … 1 für Prozent,
//  bei Auswahlreglern die Nummer der Stellung (ab 0 gezählt).
//
//  Fußlage: 0=32'  1=16'  2=8'  3=4'  4=2'
//  Form:    0=Dreieck  1=Sägezahn  2=Rechteck  3=Puls
//  Keytrack: 0=Aus  1=Halb  2=Voll      LFO-Form: 0=Dreieck 1=Rechteck 2=Zufall
// =============================================================================
#pragma once
#include "Parameter.h"
#include <vector>
#include <utility>

namespace rr1
{

struct Preset
{
    const char* name;
    std::vector<std::pair<const char*, float>> werte;
};

inline const std::vector<Preset>& allePresets()
{
    static const std::vector<Preset> liste
    {
        { "Grundklang", {} },

        { "Fetter Bass", {
            { ID::osz1Lage, 1 }, { ID::osz2Lage, 1 }, { ID::osz2Form, 2 }, { ID::osz2Fein, -8 },
            { ID::pegelOsz1, 1.0f }, { ID::pegelOsz2, 0.8f }, { ID::uebersteuern, 0.5f },
            { ID::cutoff, 400 }, { ID::resonanz, 0.25f }, { ID::filterMenge, 0.45f },
            { ID::fA, 0.002f }, { ID::fD, 0.25f }, { ID::fS, 0.1f }, { ID::fR, 0.15f },
            { ID::aA, 0.002f }, { ID::aD, 0.3f }, { ID::aS, 0.9f }, { ID::aR, 0.12f } } },

        { "Psy-Rollbass", {
            { ID::osz1Lage, 1 }, { ID::osz2Lage, 0 }, { ID::osz2Form, 2 }, { ID::pegelOsz2, 0.5f },
            { ID::uebersteuern, 0.4f }, { ID::cutoff, 250 }, { ID::resonanz, 0.55f },
            { ID::filterMenge, 0.55f }, { ID::fA, 0.001f }, { ID::fD, 0.18f }, { ID::fS, 0.0f },
            { ID::fR, 0.1f }, { ID::aD, 0.2f }, { ID::aS, 0.6f }, { ID::aR, 0.08f },
            { ID::legato, 0 }, { ID::drift, 0.1f } } },

        { "Weicher Lead", {
            { ID::osz2Fein, 9 }, { ID::glide, 0.08f }, { ID::cutoff, 2200 }, { ID::resonanz, 0.15f },
            { ID::filterMenge, 0.3f }, { ID::fA, 0.05f }, { ID::fD, 0.6f }, { ID::fS, 0.5f },
            { ID::aA, 0.02f }, { ID::aS, 0.9f }, { ID::aR, 0.4f },
            { ID::lfoTempo, 5.5f }, { ID::lfoTonhoehe, 0.15f } } },

        { "Acid-artig", {
            { ID::pegelOsz2, 0.0f }, { ID::cutoff, 300 }, { ID::resonanz, 0.75f },
            { ID::filterMenge, 0.6f }, { ID::fA, 0.001f }, { ID::fD, 0.2f }, { ID::fS, 0.0f },
            { ID::uebersteuern, 0.6f }, { ID::glide, 0.06f }, { ID::aS, 1.0f }, { ID::aR, 0.05f },
            { ID::anschlag, 0.4f } } },

        { "Zufalls-Blubbern", {
            { ID::osz1Lage, 1 }, { ID::pegelOsz2, 0.3f }, { ID::cutoff, 700 }, { ID::resonanz, 0.6f },
            { ID::filterMenge, 0.1f }, { ID::lfoForm, 2 }, { ID::lfoTempo, 8 }, { ID::lfoFilter, 0.5f },
            { ID::aS, 1.0f }, { ID::aR, 0.3f } } },

        { "Langsamer Drone", {
            { ID::osz1Lage, 1 }, { ID::osz2Halbton, 7 }, { ID::osz2Fein, 4 }, { ID::pegelOsz2, 0.5f },
            { ID::cutoff, 600 }, { ID::resonanz, 0.3f }, { ID::filterMenge, 0.3f },
            { ID::fA, 3.0f }, { ID::fD, 4.0f }, { ID::fS, 0.5f }, { ID::fR, 4.0f },
            { ID::aA, 2.0f }, { ID::aS, 1.0f }, { ID::aR, 4.0f },
            { ID::lfoTempo, 0.1f }, { ID::lfoFilter, 0.35f }, { ID::drift, 0.5f } } },

        { "Pfeifendes Filter", {
            { ID::pegelOsz1, 0.0f }, { ID::pegelOsz2, 0.0f }, { ID::cutoff, 1000 },
            { ID::resonanz, 1.0f }, { ID::filterMenge, 0.0f }, { ID::keytrack, 2 },
            { ID::aS, 1.0f }, { ID::aR, 0.4f }, { ID::glide, 0.1f }, { ID::lautstaerke, -2.0f } } },

        { "Wind", {
            { ID::pegelOsz1, 0.0f }, { ID::pegelOsz2, 0.0f }, { ID::pegelRauschen, 0.8f },
            { ID::cutoff, 800 }, { ID::resonanz, 0.7f }, { ID::filterMenge, 0.0f },
            { ID::lfoTempo, 0.15f }, { ID::lfoFilter, 0.6f },
            { ID::aA, 1.5f }, { ID::aS, 1.0f }, { ID::aR, 2.5f } } },
    };
    return liste;
}

// Setzt alle Regler auf Grundeinstellung und dann die Werte des Presets
inline void ladePreset (juce::AudioProcessorValueTreeState& zustand, int nummer)
{
    const auto& liste = allePresets();
    if (nummer < 0 || nummer >= (int) liste.size())
        return;

    for (auto* p : zustand.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, wert] : liste[(size_t) nummer].werte)
        if (auto* rp = zustand.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (wert));
}

} // namespace rr1
