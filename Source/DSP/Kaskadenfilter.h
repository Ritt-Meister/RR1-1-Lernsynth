// =============================================================================
//  Kaskadenfilter.h – das Herz des „analogen“ Klangs
//
//  Ein Tiefpassfilter lässt tiefe Frequenzen durch und dämpft hohe. Die
//  Grenzfrequenz heißt „Cutoff“. Dreht man sie zu, wird der Klang dunkler.
//
//  Dieses Filter besteht aus VIER gleichen, einfachen Filterstufen
//  hintereinander (eine „Kaskade“ oder „Leiter“). Jede Stufe dämpft oberhalb
//  des Cutoffs mit 6 dB pro Oktave, alle vier zusammen mit 24 dB pro Oktave.
//  Genau dieses Prinzip steckt in den berühmten Transistor-Leiterfiltern der
//  70er Jahre.
//
//  RESONANZ: Ein Teil des Ausgangs wird umgekehrt auf den Eingang
//  zurückgeführt. Dadurch wird der Bereich direkt am Cutoff angehoben – der
//  Klang „pfeift“ oder „quakt“. Bei voller Resonanz schwingt das Filter von
//  selbst und erzeugt einen eigenen Sinuston.
//
//  SÄTTIGUNG: Analoge Schaltungen können nicht unendlich laut werden. Die
//  Funktion tanh() rundet zu große Werte sanft ab – das gibt Wärme und Biss.
//
//  Technik: „Zero-Delay-Feedback“ nach V. Zavalishin. Die Rückkopplung wird
//  pro Abtastwert mathematisch aufgelöst, deshalb bleibt das Filter auch bei
//  hohen Frequenzen stimmig.
// =============================================================================
#pragma once
#include <cmath>
#include <algorithm>

namespace rr1
{

class Kaskadenfilter
{
public:
    void setSampleRate (double sr) { sampleRate = sr; reset(); }

    void reset()
    {
        for (auto& s : speicher)
            s = 0.0;
    }

    // eingang    : Audiosignal aus dem Mixer
    // cutoffHz   : Grenzfrequenz in Hertz
    // resonanz   : 0 … 1 (ab ca. 0,95 schwingt das Filter selbst)
    // antrieb    : Eingangsverstärkung vor der Sättigung (1 = neutral)
    float process (float eingang, double cutoffHz, double resonanz, double antrieb)
    {
        cutoffHz = std::clamp (cutoffHz, 10.0, sampleRate * 0.45);

        // g: wie stark eine Stufe bei dieser Frequenz „durchlässt“
        const double g  = std::tan (3.14159265358979 * cutoffHz / sampleRate);
        const double G  = g / (1.0 + g);
        const double k  = 4.2 * resonanz;          // Rückkopplungsstärke
        const double G2 = G * G;
        const double G4 = G2 * G2;

        // Hohe Resonanz schluckt Bass – ein Teil wird hier ausgeglichen
        const double u = eingang * antrieb * (1.0 + 0.3 * k);

        // Beitrag der gespeicherten Zustände aller vier Stufen
        const double summe = (1.0 - G) * (G2 * G * speicher[0] + G2 * speicher[1]
                                          + G * speicher[2] + speicher[3]);

        // Rückkopplung auflösen: voraussichtlicher Ausgang der 4. Stufe
        const double y4 = (G4 * u + summe) / (1.0 + k * G4);

        // Eingang minus Rückkopplung, dann sanft sättigen
        double stufe = std::tanh (u - k * y4);

        // Die vier Stufen nacheinander durchlaufen
        for (auto& s : speicher)
        {
            const double v = (stufe - s) * G;
            const double y = v + s;
            s = y + v;
            stufe = y;
        }

        // Leise Einstellungen etwas anheben, damit der Pegel gleich bleibt;
        // bei hoher Resonanz zusätzlich etwas lauter (Pegelverlust ausgleichen)
        const double ausgleich = (1.0 + 0.6 * resonanz * resonanz) / std::min (antrieb, 1.0);
        return (float) (stufe * ausgleich);
    }

private:
    double sampleRate = 44100.0;
    double speicher[4] {};
};

} // namespace rr1
