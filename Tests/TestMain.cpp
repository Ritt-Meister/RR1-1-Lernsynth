// Testprogramm: spielt jedes Preset an, prüft das Signal und speichert
// WAV-Dateien sowie ein Bild der Oberfläche. Wird für das Plugin nicht gebraucht.
#include "../Source/PluginProcessor.h"
#include "../Source/Presets.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <iostream>

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File ausgabe (argc > 1 ? argv[1] : "test_out");
    ausgabe.createDirectory();

    const double sr = 48000.0;
    const int block = 256;
    bool ok = true;

    RR1Processor proc;
    proc.setPlayConfigDetails (0, 2, sr, block);
    proc.prepareToPlay (sr, block);

    const auto& presets = rr1::allePresets();
    for (int pi = 0; pi < (int) presets.size(); ++pi)
    {
        proc.ladePreset (pi);
        proc.prepareToPlay (sr, block);

        const int laenge = (int) (sr * 4.0);
        juce::AudioBuffer<float> gesamt (2, laenge);
        gesamt.clear();

        // Melodie: (Note, Start s, Ende s) – teilweise überlappend (Legato)
        struct N { int note; double an, aus; };
        const N noten[] = { { 36, 0.0, 0.45 }, { 43, 0.4, 0.8 }, { 48, 0.75, 1.2 }, { 39, 1.25, 1.6 },
                            { 60, 1.7, 2.6 }, { 72, 2.0, 2.4 } };

        for (int pos = 0; pos < laenge; pos += block)
        {
            const int n = std::min (block, laenge - pos);
            juce::AudioBuffer<float> buf (2, n);
            juce::MidiBuffer midi;
            for (const auto& no : noten)
            {
                const int an = (int) (no.an * sr), aus = (int) (no.aus * sr);
                if (an >= pos && an < pos + n)   midi.addEvent (juce::MidiMessage::noteOn (1, no.note, 0.8f), an - pos);
                if (aus >= pos && aus < pos + n) midi.addEvent (juce::MidiMessage::noteOff (1, no.note), aus - pos);
            }
            proc.processBlock (buf, midi);
            for (int c = 0; c < 2; ++c)
                gesamt.copyFrom (c, pos, buf, c, 0, n);
        }

        float spitze = 0.0f;
        bool kaputt = false;
        for (int i = 0; i < laenge; ++i)
        {
            const float v = gesamt.getSample (0, i);
            if (! std::isfinite (v)) kaputt = true;
            spitze = std::max (spitze, std::abs (v));
        }
        const float rms = gesamt.getRMSLevel (0, 0, (int) (sr * 2.6));
        std::cout << pi << " " << presets[(size_t) pi].name << "  Spitze=" << spitze
                  << "  RMS=" << rms << (kaputt ? "  NaN/Inf!" : "") << std::endl;
        if (kaputt || spitze > 4.0f || rms < 0.005f) ok = false;

        juce::WavAudioFormat wav;
        auto datei = ausgabe.getChildFile ("preset_" + juce::String (pi) + ".wav");
        datei.deleteFile();
        std::unique_ptr<juce::OutputStream> strom (datei.createOutputStream().release());
        if (auto* w = wav.createWriterFor (strom.get(), sr, 2, 24, {}, 0))
        {
            strom.release();
            std::unique_ptr<juce::AudioFormatWriter> writer (w);
            writer->writeFromAudioSampleBuffer (gesamt, 0, laenge);
        }
    }

    // Zustand speichern und laden
    {
        juce::MemoryBlock mb;
        proc.getStateInformation (mb);
        proc.setStateInformation (mb.getData(), (int) mb.getSize());
        std::cout << "Zustand: " << mb.getSize() << " Bytes" << std::endl;
    }

    // Bild der Oberfläche
    {
        proc.ladePreset (1);
        std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditorIfNeeded());
        std::cout << "Fenster: " << ed->getWidth() << " x " << ed->getHeight() << std::endl;
        auto bild = ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f);
        juce::PNGImageFormat png;
        auto datei = ausgabe.getChildFile ("oberflaeche.png");
        datei.deleteFile();
        juce::FileOutputStream fs (datei);
        png.writeImageToStream (bild, fs);
        proc.editorBeingDeleted (ed.get());
    }

    std::cout << (ok ? "ALLES OK" : "FEHLER") << std::endl;
    return ok ? 0 : 1;
}
