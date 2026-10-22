// =============================================================================
//  MonoSynth.h – hier werden alle Bausteine zu einem Synthesizer verschaltet
//
//  Signalweg (wie auf der Oberfläche von links nach rechts):
//
//    Oszillator 1 ─┐
//    Oszillator 2 ─┼─► MIXER ─► FILTER ─► VERSTÄRKER ─► Ausgang
//    Rauschen ─────┘              ▲            ▲
//                         Filter-Hüllkurve   Lautstärke-Hüllkurve
//                                 ▲
//                    LFO ─────────┴──► (auch auf die Tonhöhe)
//
//  „Monophon“ heißt: es klingt immer nur EIN Ton. Drückt man mehrere Tasten,
//  gewinnt die zuletzt gedrückte. Lässt man sie los, springt der Synth zur
//  vorher gehaltenen Taste zurück – wie bei den klassischen Vorbildern.
// =============================================================================
#pragma once
#include "Oszillator.h"
#include "Kaskadenfilter.h"
#include "Huellkurve.h"
#include "LFO.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace rr1
{

// Alle Reglerwerte, so wie die Oberfläche sie liefert
struct SynthParameter
{
    // Oszillatoren
    int    osz1Lage = 2, osz1Form = 1;      // Lage: 0=32' 1=16' 2=8' 3=4' 4=2'
    int    osz2Lage = 2, osz2Form = 1;
    double osz2Halbton = 0.0;               // -12 … +12
    double osz2Fein = 0.0;                  // Cent (-50 … +50)
    // Mixer
    double pegelOsz1 = 0.8, pegelOsz2 = 0.6, pegelRauschen = 0.0;
    double uebersteuern = 0.3;              // 0 … 1
    // Filter
    double cutoff = 1200.0;                 // Hz
    double resonanz = 0.2;                  // 0 … 1
    double filterHuellkurve = 0.4;          // 0 … 1  (= 0 … 5 Oktaven)
    double keytrack = 0.5;                  // 0, 0,5 oder 1
    double fA = 0.005, fD = 0.4, fS = 0.3, fR = 0.3;
    // Verstärker
    double aA = 0.003, aD = 0.3, aS = 0.8, aR = 0.25;
    double lautstaerkeDb = -10.0;
    double anschlag = 0.0;                  // wie stark die Anschlagstärke wirkt
    // Modulation
    double lfoTempo = 5.0;
    int    lfoForm = 0;
    double lfoTonhoehe = 0.0, lfoFilter = 0.0;
    // Steuerung
    double glide = 0.0;                     // Sekunden
    bool   legato = true;
    double drift = 0.2;
};

class MonoSynth
{
public:
    MonoSynth() { gedrueckt.reserve (128); }

    void prepare (double sr)
    {
        sampleRate = sr;
        osz1.setSampleRate (sr);
        osz2.setSampleRate (sr);
        osz2.setPhase (0.37);
        filter.setSampleRate (sr);
        filterHk.setSampleRate (sr);
        lautHk.setSampleRate (sr);
        lfo.setSampleRate (sr);
        glaettung = 1.0 - std::exp (-1.0 / (0.005 * sr));   // 5 ms
        driftGlaettung = 1.0 - std::exp (-1.0 / (0.4 * sr));
        cutoffGlatt = std::log2 (p.cutoff);
        lautGlatt = 0.0;
        setParameter (p);
    }

    void setParameter (const SynthParameter& neu)
    {
        p = neu;
        filterHk.setParameter (p.fA, p.fD, p.fS, p.fR);
        lautHk.setParameter (p.aA, p.aD, p.aS, p.aR);
        glideFaktor = p.glide < 0.001 ? 1.0
                                      : 1.0 - std::exp (-1.0 / (p.glide / 3.0 * sampleRate));
    }

    // ---------------------------------------------------------- Tastatur ----
    void noteOn (int note, float anschlagstaerke)
    {
        const bool warStill = ! lautHk.istAktiv();
        const bool tasteGehalten = ! gedrueckt.empty();

        entferne (note);
        if (gedrueckt.size() < gedrueckt.capacity())
            gedrueckt.push_back (note);

        zielNote = note;
        velocity = anschlagstaerke;

        if (warStill)                 // aus der Stille: kein Gleiten von alter Note
            aktuelleNote = note;

        // Legato: bei gebundenem Spiel Hüllkurven NICHT neu starten
        if (! tasteGehalten || ! p.legato)
        {
            filterHk.noteOn();
            lautHk.noteOn();
        }
    }

    void noteOff (int note)
    {
        entferne (note);
        if (gedrueckt.empty())
        {
            filterHk.noteOff();
            lautHk.noteOff();
        }
        else
        {
            zielNote = gedrueckt.back();   // zurück zur noch gehaltenen Taste
        }
    }

    void allesAus()
    {
        gedrueckt.clear();
        filterHk.noteOff();
        lautHk.noteOff();
    }

    void setPitchBend (float wert) { pitchBend = wert; }   // -1 … +1
    void setModRad (float wert)    { modRad = wert; }      //  0 … 1

    // ---------------------------------------------------------- Klang -------
    void render (float* ausgang, int anzahl)
    {
        static const double lagen[] = { -24.0, -12.0, 0.0, 12.0, 24.0 }; // Fußlagen

        const double zielCutoff = std::log2 (p.cutoff);
        const double zielLaut = std::pow (10.0, p.lautstaerkeDb / 20.0);
        const double antrieb = 0.5 * std::pow (16.0, p.uebersteuern);   // 0,5 … 8
        const double vibratoTiefe = p.lfoTonhoehe * p.lfoTonhoehe * 12.0 + modRad * 0.5;
        const double anschlagGain = 1.0 - p.anschlag + p.anschlag * velocity;
        const int driftSchritt = (int) (sampleRate * 0.35);

        for (int i = 0; i < anzahl; ++i)
        {
            // 1) Glide: die Tonhöhe gleitet zur Zielnote
            aktuelleNote += (zielNote - aktuelleNote) * glideFaktor;

            // 2) LFO und analoge Drift (langsames, zufälliges Verstimmen)
            const double lfoWert = lfo.process (p.lfoTempo, p.lfoForm);

            if (++driftZaehler >= driftSchritt)
            {
                driftZaehler = 0;
                driftZiel1 = rauschen.process();
                driftZiel2 = rauschen.process();
            }
            drift1 += (driftZiel1 - drift1) * driftGlaettung;
            drift2 += (driftZiel2 - drift2) * driftGlaettung;
            const double driftTiefe = p.drift * 0.15;  // max. ±15 Cent

            // 3) Tonhöhe beider Oszillatoren (in Halbtönen, 69 = Kammerton A)
            const double basis = aktuelleNote + pitchBend * 2.0 + lfoWert * vibratoTiefe;
            const double ton1 = basis + lagen[p.osz1Lage] + drift1 * driftTiefe;
            const double ton2 = basis + lagen[p.osz2Lage] + p.osz2Halbton
                                + p.osz2Fein / 100.0 + drift2 * driftTiefe;

            const double f1 = 440.0 * std::exp2 ((ton1 - 69.0) / 12.0);
            const double f2 = 440.0 * std::exp2 ((ton2 - 69.0) / 12.0);

            // 4) Mixer
            const double mix = osz1.process (f1, p.osz1Form) * p.pegelOsz1
                             + osz2.process (f2, p.osz2Form) * p.pegelOsz2
                             + rauschen.process()            * p.pegelRauschen
                             + rauschen.process()            * 0.0001;   // winziges Grundrauschen
                             // wie in echter Elektronik – damit kann das Filter bei voller
                             // Resonanz auch ohne Eingangssignal von selbst anschwingen

            // 5) Hüllkurven
            const double fHk = filterHk.process();
            const double aHk = lautHk.process();

            // 6) Filter: Cutoff = Regler + Hüllkurve + LFO + Keytracking
            cutoffGlatt += (zielCutoff - cutoffGlatt) * glaettung;
            const double oktaven = fHk * p.filterHuellkurve * 5.0
                                 + lfoWert * p.lfoFilter * 2.0
                                 + p.keytrack * (aktuelleNote - 60.0) / 12.0;
            const double cutoffHz = std::exp2 (cutoffGlatt + oktaven);

            const double gefiltert = filter.process ((float) mix, cutoffHz, p.resonanz, antrieb);

            // 7) Verstärker (VCA) und Gleichspannungs-Filter
            lautGlatt += (zielLaut - lautGlatt) * glaettung;
            double s = gefiltert * aHk * anschlagGain * lautGlatt;

            const double ohneDC = s - dcAlt + 0.9995 * dcAus;
            dcAlt = s;
            dcAus = ohneDC;

            ausgang[i] = (float) ohneDC;
        }
    }

private:
    void entferne (int note)
    {
        gedrueckt.erase (std::remove (gedrueckt.begin(), gedrueckt.end(), note), gedrueckt.end());
    }

    SynthParameter p;
    double sampleRate = 44100.0;

    Oszillator osz1, osz2;
    Rauschen rauschen;
    Kaskadenfilter filter;
    Huellkurve filterHk, lautHk;
    LFO lfo;

    std::vector<int> gedrueckt;
    double zielNote = 60.0, aktuelleNote = 60.0;
    float velocity = 1.0f, pitchBend = 0.0f, modRad = 0.0f;
    double glideFaktor = 1.0, glaettung = 0.01, driftGlaettung = 0.0001;
    double cutoffGlatt = 10.0, lautGlatt = 0.0;
    double drift1 = 0.0, drift2 = 0.0, driftZiel1 = 0.0, driftZiel2 = 0.0;
    int driftZaehler = 0;
    double dcAlt = 0.0, dcAus = 0.0;
};

} // namespace rr1
