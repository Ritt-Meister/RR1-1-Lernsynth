// =============================================================================
//  LFO.h – Low Frequency Oscillator (langsamer Oszillator)
//
//  Ein LFO schwingt so langsam (meist unter 20 Hz), dass man ihn nicht als
//  Ton hört. Stattdessen bewegt er andere Regler automatisch:
//
//    → Tonhöhe  : Vibrato (leichtes Schwanken) oder Sirenen-Effekte
//    → Filter   : „Wah-Wah“, langsames Atmen, rhythmisches Pumpen
//
//  Formen: Dreieck (gleichmäßig), Rechteck (springt hin und her),
//  Zufall (springt bei jedem Durchlauf auf einen neuen Zufallswert,
//  bekannt als „Sample & Hold“ – typisch für blubbernde Psy-Sounds).
// =============================================================================
#pragma once
#include <cmath>
#include <cstdint>

namespace rr1
{

class LFO
{
public:
    enum Form { Dreieck = 0, Rechteck = 1, Zufall = 2 };

    void setSampleRate (double sr) { sampleRate = sr; }

    // Liefert einen Wert zwischen -1 und +1
    float process (double tempoHz, int form)
    {
        phase += tempoHz / sampleRate;
        if (phase >= 1.0)
        {
            phase -= 1.0;
            zufallsWert = neueZufallszahl();   // neuer Wert pro Durchlauf
        }

        switch (form)
        {
            case Rechteck: return phase < 0.5 ? 1.0f : -1.0f;
            case Zufall:   return zufallsWert;
            case Dreieck:
            default:       return (float) (1.0 - 4.0 * std::abs (phase - 0.5));
        }
    }

private:
    float neueZufallszahl()
    {
        zustand ^= zustand << 13;
        zustand ^= zustand >> 17;
        zustand ^= zustand << 5;
        return (float) (zustand * (1.0 / 4294967296.0) * 2.0 - 1.0);
    }

    double   sampleRate = 44100.0;
    double   phase = 0.0;
    float    zufallsWert = 0.0f;
    uint32_t zustand = 123456789u;
};

} // namespace rr1
