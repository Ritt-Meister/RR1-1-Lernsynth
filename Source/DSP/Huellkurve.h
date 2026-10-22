// =============================================================================
//  Huellkurve.h – der zeitliche Verlauf eines Tons (ADSR)
//
//  Eine Hüllkurve ist eine Steuerspannung, die beim Tastendruck startet:
//
//    A  Attack   – wie lange sie von 0 bis zum Maximum ansteigt
//    D  Decay    – wie lange sie danach auf den Haltepegel abfällt
//    S  Sustain  – der Haltepegel, solange die Taste gedrückt bleibt (0 … 1)
//    R  Release  – wie lange sie nach dem Loslassen auf 0 abfällt
//
//  Der RR-1 hat zwei davon: eine steuert die Lautstärke, die andere den
//  Cutoff des Filters (dadurch klingt der Ton z. B. am Anfang heller).
//
//  Wie bei analogen Geräten verlaufen die Kurven nicht gerade, sondern
//  „exponentiell“: am Anfang schnell, dann immer langsamer – wie ein
//  Kondensator, der sich auflädt oder entlädt.
// =============================================================================
#pragma once
#include <cmath>

namespace rr1
{

class Huellkurve
{
public:
    void setSampleRate (double sr) { sampleRate = sr; }

    // Zeiten in Sekunden, Sustain 0 … 1
    void setParameter (double attack, double decay, double sustain, double release)
    {
        // Der Attack zielt auf 1,3 und wird bei 1,0 gestoppt – so wie bei
        // vielen analogen Schaltungen. Daraus folgt die Zeitkonstante.
        attackFaktor  = faktor (attack / 1.466);
        decayFaktor   = faktor (decay / 3.0);
        releaseFaktor = faktor (release / 3.0);
        haltePegel    = sustain;
    }

    // Taste gedrückt: von der aktuellen Höhe aus neu starten (kein Knacksen)
    void noteOn()  { phase = Phase::Attack; }

    // Taste losgelassen
    void noteOff() { if (phase != Phase::Ruhe) phase = Phase::Release; }

    bool istAktiv() const { return phase != Phase::Ruhe; }

    float process()
    {
        switch (phase)
        {
            case Phase::Attack:
                pegel += (1.3 - pegel) * attackFaktor;
                if (pegel >= 1.0) { pegel = 1.0; phase = Phase::Decay; }
                break;

            case Phase::Decay:   // fällt auf den Haltepegel und bleibt dort
                pegel += (haltePegel - pegel) * decayFaktor;
                break;

            case Phase::Release:
                pegel -= pegel * releaseFaktor;
                if (pegel < 0.00001) { pegel = 0.0; phase = Phase::Ruhe; }
                break;

            case Phase::Ruhe:
                pegel = 0.0;
                break;
        }
        return (float) pegel;
    }

private:
    enum class Phase { Ruhe, Attack, Decay, Release };

    // Wandelt eine Zeitkonstante in einen Glättungsfaktor pro Abtastwert um
    double faktor (double sekunden) const
    {
        if (sekunden < 0.0001)
            return 1.0;
        return 1.0 - std::exp (-1.0 / (sekunden * sampleRate));
    }

    double sampleRate = 44100.0;
    Phase  phase = Phase::Ruhe;
    double pegel = 0.0;
    double attackFaktor = 1.0, decayFaktor = 1.0, releaseFaktor = 1.0;
    double haltePegel = 1.0;
};

} // namespace rr1
