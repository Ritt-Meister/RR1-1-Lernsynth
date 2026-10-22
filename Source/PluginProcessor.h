// =============================================================================
//  PluginProcessor.h – die Verbindung zwischen Ableton und dem Synthesizer
//
//  Ableton ruft regelmäßig processBlock() auf und übergibt dabei die
//  MIDI-Noten und einen leeren Audiopuffer. Der Prozessor füllt den Puffer
//  mit Klang. Außerdem speichert er die Reglerstellungen mit dem Projekt.
// =============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "DSP/MonoSynth.h"
#include "Parameter.h"
#include <map>
#include <string>
#include <string_view>

class RR1Processor : public juce::AudioProcessor
{
public:
    RR1Processor();
    ~RR1Processor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RR-1 Lernsynth"; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& ziel) override;
    void setStateInformation (const void* daten, int groesse) override;

    // Für die Oberfläche
    juce::AudioProcessorValueTreeState zustand;
    juce::MidiKeyboardState tastaturZustand;
    int aktuellesPreset = 0;
    void ladePreset (int nummer);

private:
    rr1::SynthParameter leseParameter() const;
    void verarbeiteMidi (const juce::MidiMessage& m);

    rr1::MonoSynth synth;

    // Schneller Zugriff auf die Reglerwerte (ohne Speicheranforderung im Audio-Thread)
    std::map<std::string, std::atomic<float>*, std::less<>> wert;
    float hole (const char* id) const { return wert.find (std::string_view (id))->second->load(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RR1Processor)
};
