// =============================================================================
//  Oszillator.h – die Klangquelle
//
//  Ein Oszillator erzeugt eine sich ständig wiederholende Schwingung. Wie oft
//  sie sich pro Sekunde wiederholt, ist die Frequenz (in Hertz) und damit die
//  Tonhöhe. Die FORM der Schwingung bestimmt die Klangfarbe:
//
//    Dreieck    – weich, fast wie eine Flöte (wenige Obertöne)
//    Sägezahn   – hell und kräftig, alle Obertöne (Streicher, Bässe, Leads)
//    Rechteck   – hohl, „holzig“ (nur ungerade Obertöne, Klarinette)
//    Puls       – schmales Rechteck: dünn, nasal
//    Sinus      – ganz rein, keine Obertöne (Sub-Bass, Pfeifen, Flöte)
//
//  Technik: Die „Phase“ läuft von 0 bis 1 und springt dann zurück auf 0.
//  Daraus wird die Wellenform berechnet. Harte Sprünge (beim Sägezahn und
//  Rechteck) erzeugen im Digitalen hässliche Störtöne („Aliasing“). Das
//  PolyBLEP-Verfahren rundet die Sprünge minimal ab und verhindert das.
// =============================================================================
#pragma once
#include <cmath>
#include <cstdint>

namespace rr1
{

class Oszillator
{
public:
    enum Form { Dreieck = 0, Saegezahn = 1, Rechteck = 2, Puls = 3, Sinus = 4 };

    void setSampleRate (double sr) { sampleRate = sr; }
    void setPhase (double p)       { phase = p; }

    // Liefert den nächsten Abtastwert (ungefähr zwischen -1 und +1).
    float process (double frequenz, int form)
    {
        // dt = wie weit die Phase pro Abtastwert weiterläuft
        double dt = frequenz / sampleRate;
        if (dt < 0.0)  dt = 0.0;
        if (dt > 0.45) dt = 0.45;

        const double t = phase;
        double aus = 0.0;

        switch (form)
        {
            case Saegezahn:
                aus = 2.0 * t - 1.0;          // Rampe von -1 nach +1
                aus -= polyBlep (t, dt);      // Sprung glätten
                break;

            case Rechteck:
            case Puls:
            {
                const double breite = (form == Rechteck) ? 0.5 : 0.12;
                aus = (t < breite) ? 1.0 : -1.0;
                aus += polyBlep (t, dt);                 // Sprung nach oben
                double t2 = t - breite;
                if (t2 < 0.0) t2 += 1.0;
                aus -= polyBlep (t2, dt);                // Sprung nach unten

                if (form == Puls)                        // Gleichanteil entfernen
                    aus = (aus - (2.0 * breite - 1.0)) * 0.75;
                break;
            }

            case Sinus:
                // Reine Sinusschwingung: hat keine Sprünge, braucht kein PolyBLEP
                aus = std::sin (2.0 * 3.14159265358979323846 * t);
                break;

            case Dreieck:
            default:
                aus = 1.0 - 4.0 * std::abs (t - 0.5);    // hat keine Sprünge
                break;
        }

        phase += dt;
        if (phase >= 1.0)
            phase -= 1.0;

        return (float) aus;
    }

private:
    // PolyBLEP: kleine Korrekturkurve rund um einen Sprung
    static double polyBlep (double t, double dt)
    {
        if (dt <= 0.0)
            return 0.0;
        if (t < dt)            { t /= dt;               return t + t - t * t - 1.0; }
        if (t > 1.0 - dt)      { t = (t - 1.0) / dt;    return t * t + t + t + 1.0; }
        return 0.0;
    }

    double sampleRate = 44100.0;
    double phase = 0.0;
};

// -----------------------------------------------------------------------------
//  Rauschgenerator: zufällige Werte = „weißes Rauschen“ (alle Frequenzen
//  gleich stark). Gut für Wind, Hi-Hats, Brandung oder als Beimischung.
// -----------------------------------------------------------------------------
class Rauschen
{
public:
    float process()
    {
        // „Xorshift“: ein schneller, einfacher Zufallszahlengenerator
        zustand ^= zustand << 13;
        zustand ^= zustand >> 17;
        zustand ^= zustand << 5;
        return (float) (zustand * (1.0 / 4294967296.0) * 2.0 - 1.0);
    }

private:
    uint32_t zustand = 2463534242u;
};

} // namespace rr1
