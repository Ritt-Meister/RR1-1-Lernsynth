// =============================================================================
//  PluginEditor.h – die Bedienoberfläche
//
//  Aufbau von links nach rechts (= Weg des Signals):
//  STEUERUNG · MODULATION · OSZILLATOREN · MIXER · FILTER · LAUTSTÄRKE
//  Darunter eine Bildschirmtastatur zum Ausprobieren mit der Maus.
//  Fährt man mit der Maus über einen Regler, erscheint eine Erklärung.
// =============================================================================
#pragma once
#include "PluginProcessor.h"
#include "Look.h"

class RR1Editor : public juce::AudioProcessorEditor
{
public:
    explicit RR1Editor (RR1Processor&);
    ~RR1Editor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Ein Drehknopf mit Namen darüber und Wert darunter
    struct Knopf : public juce::Component
    {
        juce::Slider regler;
        juce::Label name;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> verbindung;
        ~Knopf() override { regler.setLookAndFeel (nullptr); }
        void resized() override;
    };

    // Ein Kippschalter mit Namen
    struct Schalter : public juce::Component
    {
        juce::ToggleButton knopf;
        juce::Label name;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> verbindung;
        void resized() override;
    };

    struct Platz { int sektion, reihe; float spalte; juce::Component* element; };

    juce::Rectangle<int> sektionsBereich (int sektion) const;
    juce::Rectangle<int> reihenBereich (int sektion, int reihe) const;

    RR1Processor& prozessor;
    rr1::Look look;

    juce::OwnedArray<Knopf> knoepfe;
    juce::OwnedArray<Schalter> schalter;
    std::vector<Platz> plaetze;

    juce::ComboBox klangwahl;
    juce::MidiKeyboardComponent tastatur;
    juce::TooltipWindow hinweise { this, 600 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RR1Editor)
};
