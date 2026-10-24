#include "PluginEditor.h"
#include "Presets.h"

using namespace rr1;

namespace
{
    // ---------------------------------------------------------- Maße (Pixel)
    constexpr int holz = 34;          // Breite der Holzseitenteile
    constexpr int innen = 18;          // Innenabstand
    constexpr int spalte = 64;        // Breite einer Knopfspalte
    constexpr int luecke = 14;        // Abstand zwischen den Sektionen
    constexpr int kopfH = 62;         // Kopfleiste
    constexpr int titelH = 24;        // Sektionsüberschrift
    constexpr int untertitelH = 16;   // kleine Zeile über jeder Knopfreihe
    constexpr int reiheH = 88;        // eine Knopfreihe
    constexpr int sektionH = titelH + untertitelH + reiheH + untertitelH + reiheH + 8;
    constexpr int tastaturH = 86;

    struct Sektion { const char* titel; int spalten; const char* unter1; const char* unter2; };
    const Sektion sektionen[] =
    {
        { "STEUERUNG",    2, "SPIELWEISE",   "CHARAKTER" },
        { "MODULATION",   2, "LFO",          "ZIELE" },
        { "OSZILLATOREN", 4, "OSZILLATOR 1", "OSZILLATOR 2" },
        { "MIXER",        2, "PEGEL",        "RAUSCHEN · DRIVE" },
        { "FILTER",       4, "TIEFPASS 24 dB/OKT.", "FILTER-HÜLLKURVE" },
        { "LAUTSTÄRKE",   4, "AUSGANG",      "LAUTSTÄRKE-HÜLLKURVE" },
    };
    constexpr int anzahlSektionen = 6;

    int gesamtSpalten()
    {
        int n = 0;
        for (const auto& s : sektionen) n += s.spalten;
        return n;
    }

    // Was wo sitzt: Sektion, Reihe, Spalte, Parameter, Name, Erklärung
    struct Eintrag { int sektion, reihe; float spalte; const char* id; const char* name; const char* hilfe; };
    const Eintrag eintraege[] =
    {
        { 0, 0, 0, ID::glide,    "Glide",    "Glide (Portamento): Die Tonhöhe gleitet von Note zu Note. Mehr = langsameres Gleiten." },
        { 0, 0, 1, ID::legato,   "Legato",   "Legato An: Überlappend gespielte Noten starten die Hüllkurven nicht neu – gebundenes Spiel." },
        { 0, 1, 0, ID::drift,    "Drift",    "Analoge Drift: Die Oszillatoren verstimmen sich langsam und zufällig, wie bei alten Analoggeräten." },
        { 0, 1, 1, ID::anschlag, "Anschlag", "Anschlagdynamik: Wie stark die Anschlagstärke die Lautstärke beeinflusst. 0 % = immer gleich laut." },

        { 1, 0, 0, ID::lfoTempo,    "Tempo",      "Geschwindigkeit des LFO in Schwingungen pro Sekunde (Hz)." },
        { 1, 0, 1, ID::lfoForm,     "Form",       "LFO-Form: Dreieck = gleichmäßig, Rechteck = springend, Zufall = Sample & Hold (Blubbern)." },
        { 1, 1, 0, ID::lfoTonhoehe, "→ Tonhöhe",  "Wie stark der LFO die Tonhöhe bewegt (Vibrato). Das Modulationsrad fügt zusätzlich Vibrato hinzu." },
        { 1, 1, 1, ID::lfoFilter,   "→ Filter",   "Wie stark der LFO den Filter-Cutoff bewegt: Wah-Wah, Atmen, Blubbern." },

        { 2, 0, 0, ID::osz1Lage,    "Fußlage",    "Fußlage wie bei der Orgel: 32' = tiefste, 2' = höchste Oktave. 8' = normale Tonhöhe." },
        { 2, 0, 1, ID::osz1Form,    "Wellenform", "Wellenform = Klangfarbe: Sägezahn hell, Rechteck hohl, Dreieck weich, Puls dünn und nasal, Sinus ganz rein (Sub-Bass)." },
        { 2, 1, 0, ID::osz2Lage,    "Fußlage",    "Fußlage von Oszillator 2. Eine Oktave tiefer als Oszillator 1 macht Bässe fetter." },
        { 2, 1, 1, ID::osz2Form,    "Wellenform", "Wellenform von Oszillator 2. Zwei verschiedene Formen mischen ergibt neue Klangfarben. Tipp: Sinus eine Oktave tiefer = Sub-Bass." },
        { 2, 1, 2, ID::osz2Halbton, "Halbton",    "Verschiebt Oszillator 2 in Halbtönen: +7 = Quinte, +12 = Oktave, +3/+4 = Terz." },
        { 2, 1, 3, ID::osz2Fein,    "Fein",       "Feinverstimmung in Cent (1/100 Halbton). Leicht verstimmt gegen Oszillator 1 entsteht Schwebung." },

        { 3, 0, 0, ID::pegelOsz1,     "Osz 1",     "Lautstärke von Oszillator 1 im Mixer." },
        { 3, 0, 1, ID::pegelOsz2,     "Osz 2",     "Lautstärke von Oszillator 2 im Mixer." },
        { 3, 1, 0, ID::pegelRauschen, "Rauschen",  "Lautstärke des Rauschgenerators: für Wind, Atem und Perkussion." },
        { 3, 1, 1, ID::uebersteuern,  "Drive",     "Übersteuern: Wie heiß der Mixer das Filter anfährt. Mehr = wärmer, schmutziger, kompakter." },

        { 4, 0, 0, ID::cutoff,      "Cutoff",    "Grenzfrequenz des Tiefpassfilters: zu = dunkel, auf = hell. Der wichtigste Klangregler!" },
        { 4, 0, 1, ID::resonanz,    "Resonanz",  "Hebt die Frequenzen am Cutoff an: Pfeifen, Quaken. Ganz rechts schwingt das Filter von selbst." },
        { 4, 0, 2, ID::filterMenge, "Hüllkurve", "Wie weit die Filter-Hüllkurve den Cutoff aufmacht (bis zu 5 Oktaven)." },
        { 4, 0, 3, ID::keytrack,    "Keytrack",  "Der Cutoff folgt der Tastatur: Bei Halb/Voll klingen hohe Töne nicht dumpfer als tiefe." },
        { 4, 1, 0, ID::fA, "Attack",  "Filter-Hüllkurve, Attack: wie lange das Filter zum Öffnen braucht." },
        { 4, 1, 1, ID::fD, "Decay",   "Filter-Hüllkurve, Decay: wie schnell es danach auf den Sustain-Wert zurückgeht." },
        { 4, 1, 2, ID::fS, "Sustain", "Filter-Hüllkurve, Sustain: wie weit das Filter offen bleibt, solange die Taste gehalten wird." },
        { 4, 1, 3, ID::fR, "Release", "Filter-Hüllkurve, Release: wie schnell es nach dem Loslassen schließt." },

        { 5, 0, 1.5f, ID::lautstaerke, "Lautstärke", "Ausgangslautstärke des RR-1." },
        { 5, 1, 0, ID::aA, "Attack",  "Lautstärke-Hüllkurve, Attack: Einschwingzeit. Kurz = Anschlag, lang = langsames Anschwellen." },
        { 5, 1, 1, ID::aD, "Decay",   "Lautstärke-Hüllkurve, Decay: Abklingzeit bis zum Sustain-Pegel." },
        { 5, 1, 2, ID::aS, "Sustain", "Lautstärke-Hüllkurve, Sustain: Haltepegel, solange die Taste gedrückt ist." },
        { 5, 1, 3, ID::aR, "Release", "Lautstärke-Hüllkurve, Release: Ausklingzeit nach dem Loslassen der Taste." },
    };
}

// ============================================================== Bausteine ==
void RR1Editor::Knopf::resized()
{
    auto r = getLocalBounds();
    name.setBounds (r.removeFromTop (14));
    regler.setBounds (r);
}

void RR1Editor::Schalter::resized()
{
    auto r = getLocalBounds();
    name.setBounds (r.removeFromTop (14));
    knopf.setBounds (r);
}

// ============================================================== Editor =====
RR1Editor::RR1Editor (RR1Processor& p)
    : AudioProcessorEditor (&p), prozessor (p),
      tastatur (p.tastaturZustand, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&look);

    for (const auto& e : eintraege)
    {
        auto* parameter = prozessor.zustand.getParameter (e.id);

        auto bereiteNameVor = [&] (juce::Label& l)
        {
            l.setText (T (e.name), juce::dontSendNotification);
            l.setJustificationType (juce::Justification::centred);
            l.setFont (juce::FontOptions (11.0f, juce::Font::bold));
            l.setInterceptsMouseClicks (false, false);
        };

        if (dynamic_cast<juce::AudioParameterBool*> (parameter) != nullptr)
        {
            auto* s = schalter.add (new Schalter());
            bereiteNameVor (s->name);
            s->knopf.setTooltip (T (e.hilfe));
            s->verbindung = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
                prozessor.zustand, e.id, s->knopf);
            s->addAndMakeVisible (s->name);
            s->addAndMakeVisible (s->knopf);
            addAndMakeVisible (s);
            plaetze.push_back ({ e.sektion, e.reihe, e.spalte, s });
        }
        else
        {
            auto* k = knoepfe.add (new Knopf());
            bereiteNameVor (k->name);
            k->regler.setLookAndFeel (&look);   // vor dem Textfeld setzen, damit es den Stil erbt
            k->regler.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k->regler.setTextBoxStyle (juce::Slider::TextBoxBelow, true, spalte - 2, 16);
            k->regler.setTooltip (T (e.hilfe));
            k->verbindung = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                prozessor.zustand, e.id, k->regler);
            // Doppelklick setzt den Regler auf die Grundeinstellung zurück
            k->regler.setDoubleClickReturnValue (true, parameter->convertFrom0to1 (parameter->getDefaultValue()));
            k->addAndMakeVisible (k->name);
            k->addAndMakeVisible (k->regler);
            addAndMakeVisible (k);
            plaetze.push_back ({ e.sektion, e.reihe, e.spalte, k });
        }
    }

    // Klangauswahl (Presets)
    int nr = 1;
    for (const auto& preset : allePresets())
        klangwahl.addItem (T (preset.name), nr++);
    klangwahl.setSelectedId (prozessor.aktuellesPreset + 1, juce::dontSendNotification);
    klangwahl.setTooltip (T ("Fertige Klänge zum Lernen. Danach an den Reglern drehen und hören, was passiert!"));
    klangwahl.onChange = [this] { prozessor.ladePreset (klangwahl.getSelectedId() - 1); };
    addAndMakeVisible (klangwahl);

    // Bildschirmtastatur
    tastatur.setAvailableRange (24, 96);
    tastatur.setWantsKeyboardFocus (false);
    tastatur.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xfff4efe2));
    tastatur.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff121214));
    tastatur.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff8f897d));
    tastatur.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Farbe::akzent.withAlpha (0.75f));
    tastatur.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Farbe::akzent.withAlpha (0.2f));
    tastatur.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.4f));
    tastatur.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, Farbe::platte);
    tastatur.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, Farbe::schrift);
    addAndMakeVisible (tastatur);

    const int breite = 2 * holz + 2 * innen + gesamtSpalten() * spalte + (anzahlSektionen - 1) * luecke;
    const int hoehe  = kopfH + 6 + sektionH + 10 + tastaturH + 14;
    setSize (breite, hoehe);
}

RR1Editor::~RR1Editor()
{
    setLookAndFeel (nullptr);
}

juce::Rectangle<int> RR1Editor::sektionsBereich (int sektion) const
{
    int x = holz + innen;
    for (int i = 0; i < sektion; ++i)
        x += sektionen[i].spalten * spalte + luecke;
    return { x, kopfH + 6, sektionen[sektion].spalten * spalte, sektionH };
}

juce::Rectangle<int> RR1Editor::reihenBereich (int sektion, int reihe) const
{
    auto r = sektionsBereich (sektion);
    r.removeFromTop (titelH + untertitelH);
    if (reihe == 1)
        r.removeFromTop (reiheH + untertitelH);
    return r.withHeight (reiheH);
}

void RR1Editor::resized()
{
    for (const auto& p : plaetze)
    {
        auto reihe = reihenBereich (p.sektion, p.reihe);
        p.element->setBounds (reihe.getX() + juce::roundToInt (p.spalte * spalte), reihe.getY(), spalte, reiheH);
    }

    klangwahl.setBounds (getWidth() / 2 - 60, 17, 240, 28);

    auto tastenBereich = juce::Rectangle<int> (holz + innen, kopfH + 6 + sektionH + 10,
                                               getWidth() - 2 * (holz + innen), tastaturH);
    tastatur.setBounds (tastenBereich);
    tastatur.setKeyWidth ((float) tastenBereich.getWidth() / 43.0f);   // 43 weiße Tasten (C0–C6, Ableton-Zählung)
}

void RR1Editor::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto gesamt = getLocalBounds().toFloat();

    // Frontplatte und Kopfleiste
    g.fillAll (Farbe::platte);
    g.setColour (Farbe::kopf);
    g.fillRect (0.0f, 0.0f, gesamt.getWidth(), (float) kopfH);
    g.setColour (Farbe::linie);
    g.drawHorizontalLine (kopfH, (float) holz, gesamt.getWidth() - (float) holz);

    // Name des Instruments
    g.setColour (Farbe::schrift);
    g.setFont (FontOptions (34.0f, Font::bold).withKerningFactor (0.06f));
    g.drawText ("RR-1", holz + innen, 8, 120, 44, Justification::centredLeft);
    g.setFont (FontOptions (11.0f, Font::bold).withKerningFactor (0.18f));
    g.setColour (Farbe::gedimmt);
    g.drawText (T ("ANALOGER LERNSYNTHESIZER"), holz + innen + 112, 20, 260, 14, Justification::centredLeft);
    g.drawText (T ("MONOPHON · 2 OSZILLATOREN · 24-dB-FILTER"), holz + innen + 112, 34, 360, 14, Justification::centredLeft);

    g.setColour (Farbe::gedimmt);
    g.drawText (T ("KLANG"), getWidth() / 2 - 120, 24, 52, 14, Justification::centredRight);

    g.setFont (FontOptions (10.5f));
    g.drawText (T ("Maus über Regler = Erklärung · Doppelklick = Grundwert"),
                getWidth() - holz - innen - 330, 24, 330, 14, Justification::centredRight);

    // Sektionen
    for (int i = 0; i < anzahlSektionen; ++i)
    {
        const auto& s = sektionen[i];
        auto r = sektionsBereich (i);

        g.setColour (Farbe::schrift);
        g.setFont (FontOptions (13.0f, Font::bold).withKerningFactor (0.14f));
        g.drawText (T (s.titel), r.getX(), r.getY(), r.getWidth(), titelH - 4, Justification::centred);
        g.setColour (Farbe::schrift.withAlpha (0.55f));
        g.fillRect ((float) r.getX() + 4.0f, (float) (r.getY() + titelH - 3), (float) r.getWidth() - 8.0f, 1.2f);

        g.setColour (Farbe::gedimmt);
        g.setFont (FontOptions (9.5f, Font::bold).withKerningFactor (0.12f));
        const int u1 = r.getY() + titelH;
        const int u2 = u1 + untertitelH + reiheH;
        g.drawText (T (s.unter1), r.getX(), u1, r.getWidth(), untertitelH, Justification::centred);
        g.drawText (T (s.unter2), r.getX(), u2, r.getWidth(), untertitelH, Justification::centred);
        g.setColour (Farbe::linie);
        g.drawHorizontalLine (u2 - 2, (float) r.getX() + 10.0f, (float) r.getRight() - 10.0f);

        // Trennlinie und Signalpfeil zur nächsten Sektion
        if (i < anzahlSektionen - 1)
        {
            const float x = (float) r.getRight() + luecke * 0.5f;
            g.setColour (Farbe::linie);
            g.drawVerticalLine ((int) x, (float) r.getY() + 2.0f, (float) r.getBottom() - 4.0f);

            if (i >= 2)   // Oszillatoren → Mixer → Filter → Lautstärke
            {
                Path pfeil;
                const float y = (float) r.getY() + 9.0f;
                pfeil.addTriangle (x - 4.0f, y - 5.0f, x - 4.0f, y + 5.0f, x + 5.0f, y);
                g.setColour (Farbe::akzent);
                g.fillPath (pfeil);
            }
        }
    }

    // Holzseitenteile
    Look::maleHolz (g, { 0.0f, 0.0f, (float) holz, gesamt.getHeight() }, 7);
    Look::maleHolz (g, { gesamt.getWidth() - (float) holz, 0.0f, (float) holz, gesamt.getHeight() }, 11);
}
