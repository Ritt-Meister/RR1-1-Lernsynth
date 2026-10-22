// =============================================================================
//  Look.h – das Aussehen: schwarze Frontplatte, Drehknöpfe mit Skala,
//  Kippschalter und Holzseitenteile im Stil der 70er-Jahre-Synthesizer.
//  Hier wird nur gemalt, mit dem Klang hat diese Datei nichts zu tun.
// =============================================================================
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace rr1
{

namespace Farbe
{
    const juce::Colour platte   { 0xff18181a };   // Frontplatte
    const juce::Colour kopf     { 0xff111113 };   // Kopfleiste
    const juce::Colour schrift  { 0xffece6d6 };   // Beschriftung (cremeweiß)
    const juce::Colour gedimmt  { 0xff8f897d };   // Nebenbeschriftung
    const juce::Colour linie    { 0xff3a3a3e };   // Trennlinien
    const juce::Colour akzent   { 0xffe8a33a };   // Bernstein (Lämpchen, Werte)
}

class Look : public juce::LookAndFeel_V4
{
public:
    Look()
    {
        setColour (juce::Slider::textBoxTextColourId,       Farbe::akzent.withAlpha (0.9f));
        setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId,  Farbe::akzent.withAlpha (0.3f));
        setColour (juce::Label::textColourId,               Farbe::schrift);
        setColour (juce::ComboBox::backgroundColourId,      juce::Colour (0xff222225));
        setColour (juce::ComboBox::textColourId,            Farbe::schrift);
        setColour (juce::ComboBox::outlineColourId,         Farbe::linie);
        setColour (juce::ComboBox::arrowColourId,           Farbe::akzent);
        setColour (juce::PopupMenu::backgroundColourId,     juce::Colour (0xff222225));
        setColour (juce::PopupMenu::textColourId,           Farbe::schrift);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Farbe::akzent.withAlpha (0.35f));
        setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xfff3ecd9));
        setColour (juce::TooltipWindow::textColourId,       juce::Colour (0xff1a1a1a));
        setColour (juce::TooltipWindow::outlineColourId,    juce::Colour (0xff8f897d));
    }

    // ------------------------------------------------------- Drehknopf -----
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float winkelStart, float winkelEnde, juce::Slider& s) override
    {
        using namespace juce;
        auto bereich = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
        const auto mitte = bereich.getCentre();
        const float r = jmin (bereich.getWidth(), bereich.getHeight()) * 0.5f - 7.0f;

        // Skala: Rasterknöpfe bekommen einen Strich pro Stellung, sonst 0 … 10
        const double spanne = s.getMaximum() - s.getMinimum();
        const bool gerastert = s.getInterval() >= 1.0 && spanne <= 24.0;
        const int striche = gerastert ? (int) spanne + 1 : 11;

        for (int i = 0; i < striche; ++i)
        {
            const float a = winkelStart + (winkelEnde - winkelStart) * (float) i / (float) (striche - 1);
            const bool gross = gerastert || i % 5 == 0;
            g.setColour (Farbe::schrift.withAlpha (gross ? 0.85f : 0.5f));
            g.drawLine ({ mitte.getPointOnCircumference (r + 2.0f, a),
                          mitte.getPointOnCircumference (r + (gross ? 6.5f : 5.0f), a) },
                        gross ? 1.4f : 1.0f);
        }

        // Rock (äußerer Ring) mit Riffelung
        g.setGradientFill (ColourGradient (Colour (0xff3b3b3f), mitte.x, mitte.y - r,
                                           Colour (0xff0b0b0c), mitte.x, mitte.y + r, false));
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (mitte));
        g.setColour (Colours::white.withAlpha (0.07f));
        for (int i = 0; i < 32; ++i)
        {
            const float a = MathConstants<float>::twoPi * (float) i / 32.0f;
            g.drawLine ({ mitte.getPointOnCircumference (r * 0.80f, a),
                          mitte.getPointOnCircumference (r * 0.98f, a) }, 1.0f);
        }

        // Kappe
        const float rk = r * 0.72f;
        g.setGradientFill (ColourGradient (Colour (0xff36363a), mitte.x - rk * 0.5f, mitte.y - rk * 0.7f,
                                           Colour (0xff0d0d0e), mitte.x + rk * 0.4f, mitte.y + rk, true));
        g.fillEllipse (Rectangle<float> (rk * 2, rk * 2).withCentre (mitte));
        g.setColour (Colour (0xff55555a));
        g.drawEllipse (Rectangle<float> (rk * 2, rk * 2).withCentre (mitte), 0.8f);

        // Zeiger
        const float a = winkelStart + pos * (winkelEnde - winkelStart);
        g.setColour (Farbe::schrift);
        g.drawLine ({ mitte.getPointOnCircumference (r * 0.18f, a),
                      mitte.getPointOnCircumference (r * 0.94f, a) }, 2.4f);
    }

    // ------------------------------------------------------- Kippschalter --
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool) override
    {
        using namespace juce;
        auto r = b.getLocalBounds().toFloat();
        const bool an = b.getToggleState();

        auto koerper = Rectangle<float> (26.0f, 40.0f).withCentre (r.getCentre().translated (0.0f, -6.0f));
        g.setColour (Colour (0xff0b0b0c));
        g.fillRoundedRectangle (koerper, 4.0f);
        g.setColour (Farbe::linie.brighter (0.3f));
        g.drawRoundedRectangle (koerper, 4.0f, 1.0f);

        // Wippe: die gedrückte Hälfte ist dunkler
        auto oben  = koerper.reduced (3.0f).removeFromTop (koerper.getHeight() * 0.5f - 3.0f);
        auto unten = koerper.reduced (3.0f).removeFromBottom (koerper.getHeight() * 0.5f - 3.0f);
        g.setColour (an ? Colour (0xff3c3c40) : Colour (0xff1c1c1e));
        g.fillRoundedRectangle (oben, 2.5f);
        g.setColour (an ? Colour (0xff1c1c1e) : Colour (0xff3c3c40));
        g.fillRoundedRectangle (unten, 2.5f);

        // Lämpchen
        auto lampe = Rectangle<float> (7.0f, 7.0f).withCentre ({ koerper.getCentreX(), koerper.getY() - 7.0f });
        g.setColour (an ? Farbe::akzent : Colour (0xff3a2a14));
        g.fillEllipse (lampe);
        if (an)
        {
            g.setColour (Farbe::akzent.withAlpha (0.25f));
            g.fillEllipse (lampe.expanded (3.0f));
        }

        g.setColour (Farbe::akzent.withAlpha (0.9f));
        g.setFont (FontOptions (11.0f));
        g.drawText (an ? String::fromUTF8 ("An") : String::fromUTF8 ("Aus"),
                    r.removeFromBottom (16.0f), Justification::centred);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (13.0f, juce::Font::bold); }
    juce::Font getPopupMenuFont() override                { return juce::FontOptions (13.0f); }
    juce::Font getSliderPopupFont (juce::Slider&) override { return juce::FontOptions (12.0f); }

    juce::Label* createSliderTextBox (juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (s);
        l->setFont (juce::FontOptions (11.0f));
        return l;
    }

    // ------------------------------------------------------- Holz ----------
    static void maleHolz (juce::Graphics& g, juce::Rectangle<float> r, int saat)
    {
        using namespace juce;
        ColourGradient verlauf (Colour (0xff6e3e1d), r.getX(), 0.0f, Colour (0xff4a2711), r.getRight(), 0.0f, false);
        verlauf.addColour (0.45, Colour (0xff8c5630));
        g.setGradientFill (verlauf);
        g.fillRect (r);

        Random zufall (saat);
        for (int i = 0; i < 22; ++i)   // Maserung
        {
            const float x0 = r.getX() + zufall.nextFloat() * r.getWidth();
            const float f1 = 0.008f + zufall.nextFloat() * 0.01f, f2 = 0.03f + zufall.nextFloat() * 0.03f;
            Path p;
            p.startNewSubPath (x0, r.getY());
            for (float yy = r.getY(); yy <= r.getBottom(); yy += 6.0f)
                p.lineTo (x0 + std::sin (yy * f1 + (float) i) * 2.5f + std::sin (yy * f2 + (float) i * 1.7f) * 1.0f, yy);
            g.setColour (Colour (0xff2a1408).withAlpha (0.12f + 0.25f * zufall.nextFloat()));
            g.strokePath (p, PathStrokeType (0.5f + zufall.nextFloat() * 1.3f));
        }

        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.10f), r.getX(), 0.0f,
                                           Colours::transparentWhite, r.getX() + 6.0f, 0.0f, false));
        g.fillRect (r);
        g.setColour (Colours::black.withAlpha (0.45f));
        g.drawRect (r, 1.0f);
    }
};

} // namespace rr1
