# RR-1 Lernsynth

Ein einstimmiger, analog-artiger Lernsynthesizer als AU-/VST3-Plugin für den Mac
(z. B. für Ableton Live 12). Der Code ist ausführlich auf Deutsch kommentiert.

## Aufbau

| Datei | Inhalt |
| --- | --- |
| `Source/DSP/Oszillator.h` | Oszillatoren (Dreieck, Sägezahn, Rechteck, Puls) und Rauschen |
| `Source/DSP/Kaskadenfilter.h` | 24-dB-Tiefpass mit Resonanz und Sättigung |
| `Source/DSP/Huellkurve.h` | ADSR-Hüllkurve |
| `Source/DSP/LFO.h` | LFO (Dreieck, Rechteck, Zufall) |
| `Source/DSP/MonoSynth.h` | Verschaltung aller Bausteine, Notenverwaltung, Glide |
| `Source/Parameter.h` | Alle Regler mit Bereichen und Grundwerten |
| `Source/Presets.h` | Fertige Klänge |
| `Source/PluginProcessor.*` | Verbindung zu Ableton (MIDI, Audio, Speichern) |
| `Source/PluginEditor.*`, `Source/Look.h` | Oberfläche |
| `build.yml` | GitHub-Bauprogramm (gehört nach `.github/workflows/`) |

## Bauen

- **GitHub:** Bei jeder Änderung baut GitHub Actions das Plugin automatisch.
  Das Ergebnis liegt unter *Actions → letzter Lauf → Artifacts*.
- **Lokal (Mac mit Xcode):**
  `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release`

## Installation (Mac)

```
cp -R "RR-1 Lernsynth.component" ~/Library/Audio/Plug-Ins/Components/
cp -R "RR-1 Lernsynth.vst3"      ~/Library/Audio/Plug-Ins/VST3/
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/"RR-1 Lernsynth.component"
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"RR-1 Lernsynth.vst3"
```

## Lizenz

Verwendet das JUCE-Framework (AGPLv3). Für den privaten Gebrauch frei.
