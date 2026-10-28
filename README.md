# Jamo — Vocal Tuning VST3

Jamo is an original JUCE/C++ vocal-tuning project scaffold. It builds a VST3 plugin and Standalone app, and requests AU on macOS. It includes a YIN-inspired monophonic pitch detector, scale quantization, MIDI-note target selection, host-automatable parameters, state serialization, and a dark custom editor.

## Important status / honest limitations

This repository is a **development starting point**, not a release-certified Auto-Tune replacement. The pitch detector and scale mapper are implemented, but the current processor uses JUCE's pitch shifter as a first-pass correction stage. The current implementation is not yet production-grade: the detector is CPU-heavy, the dry/wet control is currently a wet-level control rather than a true parallel blend, formant shift and humanize are UI/parameter scaffolding only, and MIDI target handling uses the latest Note-On in the current block. Latency and artefacts must be measured and improved before professional release. Do not market this build as production-ready until those items are fixed and tested.

## Requirements

- CMake 3.22+
- A C++17 compiler (Visual Studio 2022 on Windows, Xcode on macOS, GCC/Clang on Linux)
- Internet connection on the first configure: CMake FetchContent downloads JUCE 8.0.10
- Platform audio development libraries (Linux: ALSA/JACK, X11 and FreeType packages)

## Build on Windows

Install **Visual Studio 2022** with “Desktop development with C++”, and install CMake. In PowerShell, from the repository folder:

```powershell
cmake -S . -B build -DJAMO_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Look for the generated plugin under a path similar to:
`build/Jamo_artefacts/Release/VST3/Jamo.vst3`

The exact path may vary by generator and platform. For Visual Studio, the VST3 bundle is a directory ending in `.vst3`, not always a single `.dll` file.

## Build on macOS

Install Xcode command-line tools and CMake, then:

```bash
cmake -S . -B build -DJAMO_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

VST3 and Standalone targets are requested. The AU target is also requested by CMake and is only relevant on macOS. Check the generated artefacts directory for `Jamo.vst3`, `Jamo.component`, and the Standalone app.

## Build on Linux

Install CMake, a C++ compiler and JUCE Linux dependencies, then use the same configure/build commands. VST3 is supported by JUCE on Linux; FL Studio Linux support depends on your FL Studio environment.

## Install in FL Studio (Windows)

1. Build the project or download the `.vst3` artifact produced by a successful build.
2. Copy the complete `Jamo.vst3` bundle/folder to:
   `%LOCALAPPDATA%\Programs\Common\VST3\Jamo.vst3`
   or `C:\Program Files\Common Files\VST3\Jamo.vst3`.
3. Open FL Studio → **Options → Manage plugins**.
4. Add the folder if necessary, click **Find installed plugins**, then search for “Jamo”.
5. Insert Jamo on a vocal mixer track. Feed the vocal into the track and choose the key/scale.

## GitHub: upload the project and get a build

1. Sign in to GitHub and create a new repository named `Jamo` (choose Public or Private).
2. On the repository page, choose **Add file → Upload files**.
3. Upload the project files/folders from this ZIP (extract it first). GitHub's web uploader may not handle every nested/hidden file smoothly; if so use GitHub Desktop.
4. Commit the files to the `main` branch.
5. Open **Actions**. Approve workflow execution if GitHub asks, then open the “Build Jamo” workflow run.
6. If it succeeds, open the completed run and scroll to **Artifacts** at the bottom. Download `Jamo-...-VST3`; it contains the generated VST3 folder (and Standalone output when produced).
7. For a reliable workflow, use GitHub Desktop: **File → Add local repository** (or “Add existing repository”), select the extracted `Jamo` folder, commit, then **Publish repository**.

### Optional: upload the compiled plugin as a GitHub Actions artifact

Add this step after the Build step in `.github/workflows/build.yml` (the exact artifact path can vary by OS/generator):

```yaml
      - name: Upload plugin artefacts
        uses: actions/upload-artifact@v4
        with:
          name: Jamo-${{ runner.os }}
          path: |
            build/**/Jamo_artefacts/**/Jamo.vst3
            build/**/Jamo_artefacts/**/Jamo.component
```

For Windows, the `.vst3` is a directory bundle. If the glob does not capture it as expected, zip the bundle first and upload the ZIP. A GitHub Actions artifact is a build output, not a signed installer.

## Included features

- VST3 / Standalone project targets; AU requested for macOS
- Mono/stereo bus-layout checks
- Host parameters for Retune Speed, Humanize, Flex Tune, Formant Shift, Mix, Input/Output Gain, Bypass, key, scale, mode, MIDI mode
- Scale masks: Chromatic, Major, Minor, Pentatonic, Blues, Dorian, Mixolydian
- Rolling-buffer pitch estimate and note display
- Plugin state saved/restored through JUCE ValueTree
- Basic test executable and GitHub Actions build workflow

## Next engineering milestones

1. Run the CI build and fix platform-specific compiler issues.
2. Replace the current detector with a faster, confidence-calibrated YIN/MPM implementation and multi-rate analysis.
3. Implement a genuine parallel dry/wet path, parameter smoothing and safe per-sample/block pitch-ratio transitions.
4. Improve MIDI event tracking, note-off behavior, voice-leading and missing-MIDI fallback.
5. Implement independent formant processing (e.g. LPC envelope warp / cepstral envelope preservation).
6. Add preset files and a tested preset browser.
7. Add audio test fixtures, latency/CPU benchmarks and listening tests at 44.1–192 kHz.
8. Test the built plugin in FL Studio on the actual target OS.

## License

Choose a license before publishing. JUCE has its own licensing terms; review them for your intended distribution.
