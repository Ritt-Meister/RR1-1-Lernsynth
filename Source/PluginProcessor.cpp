#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace rr1;

RR1Processor::RR1Processor()
    : AudioProcessor (BusesProperties().withOutput ("Ausgang", juce::AudioChannelSet::stereo(), true)),
      zustand (*this, nullptr, "RR1", erstelleParameter())
{
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            wert[rp->getParameterID().toStdString()] = zustand.getRawParameterValue (rp->getParameterID());
}

bool RR1Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto aus = layouts.getMainOutputChannelSet();
    return aus == juce::AudioChannelSet::mono() || aus == juce::AudioChannelSet::stereo();
}

void RR1Processor::prepareToPlay (double sampleRate, int)
{
    synth.setParameter (leseParameter());
    synth.prepare (sampleRate);
}

// Liest alle Regler und packt sie in die Struktur für den Synthesizer
SynthParameter RR1Processor::leseParameter() const
{
    SynthParameter p;
    p.osz1Lage      = (int) hole (ID::osz1Lage);
    p.osz1Form      = (int) hole (ID::osz1Form);
    p.osz2Lage      = (int) hole (ID::osz2Lage);
    p.osz2Form      = (int) hole (ID::osz2Form);
    p.osz2Halbton   = hole (ID::osz2Halbton);
    p.osz2Fein      = hole (ID::osz2Fein);
    p.pegelOsz1     = hole (ID::pegelOsz1);
    p.pegelOsz2     = hole (ID::pegelOsz2);
    p.pegelRauschen = hole (ID::pegelRauschen);
    p.uebersteuern  = hole (ID::uebersteuern);
    p.cutoff        = hole (ID::cutoff);
    p.resonanz      = hole (ID::resonanz);
    p.filterHuellkurve = hole (ID::filterMenge);
    p.keytrack      = hole (ID::keytrack) * 0.5;      // 0 / 0,5 / 1
    p.fA = hole (ID::fA);  p.fD = hole (ID::fD);  p.fS = hole (ID::fS);  p.fR = hole (ID::fR);
    p.aA = hole (ID::aA);  p.aD = hole (ID::aD);  p.aS = hole (ID::aS);  p.aR = hole (ID::aR);
    p.lautstaerkeDb = hole (ID::lautstaerke);
    p.anschlag      = hole (ID::anschlag);
    p.lfoTempo      = hole (ID::lfoTempo);
    p.lfoForm       = (int) hole (ID::lfoForm);
    p.lfoTonhoehe   = hole (ID::lfoTonhoehe);
    p.lfoFilter     = hole (ID::lfoFilter);
    p.glide         = hole (ID::glide);
    p.legato        = hole (ID::legato) > 0.5f;
    p.drift         = hole (ID::drift);
    return p;
}

void RR1Processor::verarbeiteMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        synth.noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        synth.noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())
        synth.setPitchBend ((m.getPitchWheelValue() - 8192) / 8192.0f);
    else if (m.isControllerOfType (1))                       // Modulationsrad
        synth.setModRad (m.getControllerValue() / 127.0f);
    else if (m.isAllNotesOff() || m.isAllSoundOff())
        synth.allesAus();
}

void RR1Processor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals keineDenormals;
    puffer.clear();

    // Noten von der Bildschirmtastatur hinzufügen
    tastaturZustand.processNextMidiBuffer (midi, 0, puffer.getNumSamples(), true);

    synth.setParameter (leseParameter());

    // Zwischen den MIDI-Ereignissen rechnen – so sitzen Noten sample-genau
    auto* links = puffer.getWritePointer (0);
    int position = 0;
    for (const auto meta : midi)
    {
        const int zeitpunkt = juce::jlimit (0, puffer.getNumSamples(), meta.samplePosition);
        if (zeitpunkt > position)
            synth.render (links + position, zeitpunkt - position);
        position = zeitpunkt;
        verarbeiteMidi (meta.getMessage());
    }
    if (position < puffer.getNumSamples())
        synth.render (links + position, puffer.getNumSamples() - position);

    // Monosignal auf alle Kanäle kopieren
    for (int kanal = 1; kanal < puffer.getNumChannels(); ++kanal)
        puffer.copyFrom (kanal, 0, puffer, 0, 0, puffer.getNumSamples());
}

void RR1Processor::ladePreset (int nummer)
{
    rr1::ladePreset (zustand, nummer);
    aktuellesPreset = nummer;
}

void RR1Processor::getStateInformation (juce::MemoryBlock& ziel)
{
    if (auto xml = zustand.copyState().createXml())
        copyXmlToBinary (*xml, ziel);
}

void RR1Processor::setStateInformation (const void* daten, int groesse)
{
    if (auto xml = getXmlFromBinary (daten, groesse))
        if (xml->hasTagName (zustand.state.getType()))
            zustand.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* RR1Processor::createEditor()
{
    return new RR1Editor (*this);
}

// Diese Funktion ruft Ableton auf, um das Plugin zu erzeugen
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RR1Processor();
}
