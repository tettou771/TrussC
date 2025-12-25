#include "tcApp.h"

void tcApp::setup() {
    setWindowTitle("ChipSound Example");
    createSounds();
}

void tcApp::createSounds() {
    float x = margin;
    float y = 55;  // Space for title and label

    // =========================================================================
    // Section 1: Simple Notes (5 types)
    // =========================================================================
    vector<pair<string, Wave>> waveTypes = {
        {"Sin", Wave::Sin},
        {"Square", Wave::Square},
        {"Triangle", Wave::Triangle},
        {"Sawtooth", Wave::Sawtooth},
        {"Noise", Wave::Noise},
        {"Pink", Wave::PinkNoise}
    };

    for (const auto& [name, wave] : waveTypes) {
        ChipSoundNote note;
        note.wave = wave;
        note.hz = 440;
        note.duration = 0.3f;
        note.volume = 0.4f;

        SoundButton btn;
        btn.label = name;
        btn.sound = note.build();
        btn.x = x;
        btn.y = y;
        btn.w = buttonWidth;
        btn.h = buttonHeight;
        simpleButtons.push_back(btn);

        x += buttonWidth + margin;
    }

    // =========================================================================
    // Section 2: Chords (8 types)
    // =========================================================================
    x = margin;
    y += buttonHeight + margin + 20;

    // Helper: Create chord bundle
    auto makeChord = [](vector<float> frequencies, Wave wave = Wave::Square) {
        ChipSoundBundle bundle;
        for (float hz : frequencies) {
            ChipSoundNote note;
            note.wave = wave;
            note.hz = hz;
            note.duration = 0.4f;
            note.volume = 0.3f;
            bundle.add(note, 0.0f);
        }
        return bundle.build();
    };

    // C Major: C4(261.63), E4(329.63), G4(392.00)
    {
        SoundButton btn;
        btn.label = "C Major";
        btn.sound = makeChord({261.63f, 329.63f, 392.00f});
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // A Minor: A3(220), C4(261.63), E4(329.63)
    {
        SoundButton btn;
        btn.label = "A Minor";
        btn.sound = makeChord({220.0f, 261.63f, 329.63f});
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Power Chord: C + G
    {
        SoundButton btn;
        btn.label = "Power";
        btn.sound = makeChord({261.63f, 392.00f});
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Octave: 440 + 880
    {
        SoundButton btn;
        btn.label = "Octave";
        btn.sound = makeChord({440.0f, 880.0f});
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Second row of chords
    x = margin;
    y += buttonHeight + margin;

    // Fifth: 440 + 660
    {
        SoundButton btn;
        btn.label = "Fifth";
        btn.sound = makeChord({440.0f, 660.0f});
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Dissonance: half-step apart
    {
        SoundButton btn;
        btn.label = "Dissonant";
        btn.sound = makeChord({440.0f, 466.16f});  // A4 + A#4
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Thick chord: 3 notes + octave
    {
        SoundButton btn;
        btn.label = "Thick";
        btn.sound = makeChord({261.63f, 329.63f, 392.00f, 523.25f});  // C Major + C5
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Mixed waves chord
    {
        ChipSoundBundle bundle;
        ChipSoundNote n1{Wave::Sin, 261.63f, 0.4f, 0.3f};
        ChipSoundNote n2{Wave::Square, 329.63f, 0.4f, 0.25f};
        ChipSoundNote n3{Wave::Triangle, 392.00f, 0.4f, 0.3f};
        bundle.add(n1, 0.0f);
        bundle.add(n2, 0.0f);
        bundle.add(n3, 0.0f);

        SoundButton btn;
        btn.label = "Mixed";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        chordButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // =========================================================================
    // Section 3: Effects (4 types)
    // =========================================================================
    x = margin;
    y += buttonHeight + margin + 20;

    // Detune: slight detuning for beating/chorus effect
    {
        ChipSoundBundle bundle;
        ChipSoundNote n1{Wave::Square, 440.0f, 0.3f, 0.3f};
        ChipSoundNote n2{Wave::Square, 443.0f, 0.3f, 0.3f};  // Slight detune
        n1.attack = 0.01f; n1.decay = 0.05f; n1.sustain = 0.6f; n1.release = 0.1f;
        n2.attack = 0.01f; n2.decay = 0.05f; n2.sustain = 0.6f; n2.release = 0.1f;
        bundle.add(n1, 0.0f);
        bundle.add(n2, 0.0f);

        SoundButton btn;
        btn.label = "Detune";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Arpeggio: rapid sequence
    {
        ChipSoundBundle bundle;
        float times[] = {0.0f, 0.05f, 0.1f, 0.15f};
        float freqs[] = {261.63f, 329.63f, 392.00f, 523.25f};
        for (int i = 0; i < 4; i++) {
            ChipSoundNote n{Wave::Square, freqs[i], 0.15f, 0.35f};
            n.attack = 0.005f; n.decay = 0.02f; n.sustain = 0.5f; n.release = 0.08f;
            bundle.add(n, times[i]);
        }

        SoundButton btn;
        btn.label = "Arpeggio";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Rising pitch
    {
        ChipSoundBundle bundle;
        for (int i = 0; i < 8; i++) {
            float hz = 200.0f * pow(2.0f, i / 8.0f);  // 200 -> 400 Hz
            ChipSoundNote n{Wave::Square, hz, 0.08f, 0.35f};
            n.attack = 0.005f; n.decay = 0.01f; n.sustain = 0.8f; n.release = 0.02f;
            bundle.add(n, i * 0.06f);
        }

        SoundButton btn;
        btn.label = "Rise";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Falling pitch
    {
        ChipSoundBundle bundle;
        for (int i = 0; i < 8; i++) {
            float hz = 800.0f * pow(0.5f, i / 8.0f);  // 800 -> 400 Hz
            ChipSoundNote n{Wave::Square, hz, 0.08f, 0.35f};
            n.attack = 0.005f; n.decay = 0.01f; n.sustain = 0.8f; n.release = 0.02f;
            bundle.add(n, i * 0.06f);
        }

        SoundButton btn;
        btn.label = "Fall";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Second row of effects (noise-based)
    x = margin;
    y += buttonHeight + margin;

    // Hit - quick noise burst attack sound
    {
        ChipSoundBundle bundle;
        // Sharp noise burst
        ChipSoundNote hit{Wave::Noise, 0, 0.08f, 0.5f};
        hit.attack = 0.001f; hit.decay = 0.02f; hit.sustain = 0.3f; hit.release = 0.05f;
        bundle.add(hit, 0.0f);
        // Add pitch drop for impact
        for (int i = 0; i < 4; i++) {
            float hz = 200.0f * pow(0.7f, (float)i);
            ChipSoundNote n{Wave::Square, hz, 0.03f, 0.3f};
            n.attack = 0.001f; n.decay = 0.01f; n.sustain = 0.5f; n.release = 0.02f;
            bundle.add(n, i * 0.015f);
        }

        SoundButton btn;
        btn.label = "Hit";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Explosion - longer noise decay
    {
        ChipSoundBundle bundle;
        // Main explosion noise
        ChipSoundNote boom{Wave::Noise, 0, 0.3f, 0.6f};
        boom.attack = 0.005f; boom.decay = 0.1f; boom.sustain = 0.4f; boom.release = 0.15f;
        bundle.add(boom, 0.0f);
        // Low rumble
        ChipSoundNote rumble{Wave::Square, 60.0f, 0.25f, 0.3f};
        rumble.attack = 0.01f; rumble.decay = 0.08f; rumble.sustain = 0.3f; rumble.release = 0.1f;
        bundle.add(rumble, 0.0f);

        SoundButton btn;
        btn.label = "Explosion";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Laser - sharp attack with pitch sweep
    {
        ChipSoundBundle bundle;
        for (int i = 0; i < 10; i++) {
            float hz = 1200.0f * pow(0.85f, (float)i);  // 1200 -> ~300 Hz
            ChipSoundNote n{Wave::Square, hz, 0.025f, 0.35f};
            n.attack = 0.001f; n.decay = 0.005f; n.sustain = 0.8f; n.release = 0.01f;
            bundle.add(n, i * 0.02f);
        }
        // Add noise tail
        ChipSoundNote tail{Wave::Noise, 0, 0.05f, 0.15f};
        tail.attack = 0.01f; tail.decay = 0.02f; tail.sustain = 0.2f; tail.release = 0.02f;
        bundle.add(tail, 0.15f);

        SoundButton btn;
        btn.label = "Laser";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // Jump - rising then falling
    {
        ChipSoundBundle bundle;
        // Quick rise
        for (int i = 0; i < 5; i++) {
            float hz = 150.0f * pow(1.3f, (float)i);  // Rising
            ChipSoundNote n{Wave::Square, hz, 0.03f, 0.3f};
            n.attack = 0.002f; n.decay = 0.01f; n.sustain = 0.7f; n.release = 0.01f;
            bundle.add(n, i * 0.025f);
        }
        // Then fall
        for (int i = 0; i < 5; i++) {
            float hz = 150.0f * pow(1.3f, 4.0f - i);  // Falling from peak
            ChipSoundNote n{Wave::Square, hz, 0.03f, 0.3f};
            n.attack = 0.002f; n.decay = 0.01f; n.sustain = 0.7f; n.release = 0.01f;
            bundle.add(n, 0.125f + i * 0.025f);
        }

        SoundButton btn;
        btn.label = "Jump";
        btn.sound = bundle.build();
        btn.x = x; btn.y = y; btn.w = buttonWidth; btn.h = buttonHeight;
        effectButtons.push_back(btn);
        x += buttonWidth + margin;
    }

    // =========================================================================
    // Section 4: Melodies (2 loops)
    // =========================================================================
    x = margin;
    y += buttonHeight + margin + 20;

    // Fanfare melody
    {
        ChipSoundBundle bundle;
        // Simple fanfare: C-E-G-C (octave up)
        float notes[] = {261.63f, 329.63f, 392.00f, 523.25f, 523.25f};
        float times[] = {0.0f, 0.15f, 0.3f, 0.45f, 0.6f};
        float durs[]  = {0.12f, 0.12f, 0.12f, 0.12f, 0.25f};

        for (int i = 0; i < 5; i++) {
            ChipSoundNote n{Wave::Square, notes[i], durs[i], 0.35f};
            n.attack = 0.01f; n.decay = 0.02f; n.sustain = 0.7f; n.release = 0.03f;
            bundle.add(n, times[i]);
        }
        // Add silence at end for loop spacing
        bundle.add({Wave::Silent, 0, 0.3f}, 0.85f);

        SoundButton btn;
        btn.label = "Fanfare (Loop)";
        btn.sound = bundle.build();
        btn.sound.setLoop(true);
        btn.isLoop = true;
        btn.x = x; btn.y = y; btn.w = buttonWidth * 1.4f; btn.h = buttonHeight;
        melodyButtons.push_back(btn);
        x += buttonWidth * 1.4f + margin;
    }

    // 8-bit BGM pattern (exactly 1.0 second loop = 4 beats at 0.25s each)
    {
        ChipSoundBundle bundle;
        float beatLen = 0.25f;  // Quarter note
        float noteLen = 0.2f;   // Slightly shorter for gap

        // Bass line
        float bassNotes[] = {130.81f, 130.81f, 146.83f, 146.83f};  // C3, C3, D3, D3
        for (int i = 0; i < 4; i++) {
            ChipSoundNote n{Wave::Square, bassNotes[i], noteLen, 0.25f};
            n.attack = 0.01f; n.decay = 0.05f; n.sustain = 0.5f; n.release = 0.04f;
            bundle.add(n, i * beatLen);
        }

        // Melody on top
        float melNotes[] = {523.25f, 587.33f, 659.25f, 587.33f};  // C5, D5, E5, D5
        for (int i = 0; i < 4; i++) {
            ChipSoundNote n{Wave::Triangle, melNotes[i], noteLen, 0.3f};
            n.attack = 0.01f; n.decay = 0.03f; n.sustain = 0.6f; n.release = 0.04f;
            bundle.add(n, i * beatLen);
        }

        // Pad to exactly 1.0 second for clean loop
        bundle.add({Wave::Silent, 0, 0.01f}, 0.99f);

        SoundButton btn;
        btn.label = "8bit BGM (Loop)";
        btn.sound = bundle.build();
        btn.sound.setLoop(true);
        btn.isLoop = true;
        btn.x = x; btn.y = y; btn.w = buttonWidth * 1.4f; btn.h = buttonHeight;
        melodyButtons.push_back(btn);
    }
}

void tcApp::drawButton(const SoundButton& btn, bool highlight) {
    // Background
    if (highlight) {
        setColor(0.4f, 0.7f, 0.4f);  // Bright green when playing
    } else {
        setColor(0.2f, 0.3f, 0.4f);  // Dark blue-gray
    }
    drawRect(btn.x, btn.y, btn.w, btn.h);

    // Border
    setColor(0.5f, 0.6f, 0.7f);
    noFill();
    drawRect(btn.x, btn.y, btn.w, btn.h);
    fill();  // Restore fill for next draws

    // Label
    setColor(1.0f);
    float textX = btn.x + 10;
    float textY = btn.y + btn.h / 2 - 5;
    drawBitmapString(btn.label, textX, textY);
}

void tcApp::draw() {
    clear(0.1f);
    float currentTime = getElapsedTime();

    // Title
    setColor(1.0f);
    drawBitmapString("=== ChipSound Example ===", margin, 25);

    // Section 1: Simple Notes
    setColor(0.8f, 0.8f, 0.4f);
    drawBitmapString("Simple Notes", margin, simpleButtons[0].y - 15);
    for (auto& btn : simpleButtons) {
        bool playing = btn.sound.isPlaying() || currentTime < btn.playEndTime;
        drawButton(btn, playing);
    }

    // Section 2: Chords
    setColor(0.8f, 0.8f, 0.4f);
    drawBitmapString("Chords", margin, chordButtons[0].y - 15);
    for (auto& btn : chordButtons) {
        bool playing = btn.sound.isPlaying() || currentTime < btn.playEndTime;
        drawButton(btn, playing);
    }

    // Section 3: Effects
    setColor(0.8f, 0.8f, 0.4f);
    drawBitmapString("Effects", margin, effectButtons[0].y - 15);
    for (auto& btn : effectButtons) {
        bool playing = btn.sound.isPlaying() || currentTime < btn.playEndTime;
        drawButton(btn, playing);
    }

    // Section 4: Melodies
    setColor(0.8f, 0.8f, 0.4f);
    drawBitmapString("Melodies (click to toggle loop)", margin, melodyButtons[0].y - 15);
    for (auto& btn : melodyButtons) {
        drawButton(btn, btn.sound.isPlaying());
    }

    // Instructions
    setColor(0.5f);
    int h = getWindowHeight();
    drawBitmapString("Click buttons to play sounds. Melodies toggle on/off.", margin, h - 25);
}

void tcApp::mousePressed(Vec2 pos, int button) {
    auto checkButtons = [&](vector<SoundButton>& buttons) {
        for (auto& btn : buttons) {
            if (pos.x >= btn.x && pos.x <= btn.x + btn.w &&
                pos.y >= btn.y && pos.y <= btn.y + btn.h) {

                if (btn.isLoop) {
                    // Toggle for melodies
                    if (btn.sound.isPlaying()) {
                        btn.sound.stop();
                    } else {
                        btn.sound.play();
                    }
                } else {
                    // One-shot for others
                    btn.sound.stop();
                    btn.sound.play();
                    btn.playEndTime = getElapsedTime() + btn.sound.getDuration();
                }
                return true;
            }
        }
        return false;
    };

    if (checkButtons(simpleButtons)) return;
    if (checkButtons(chordButtons)) return;
    if (checkButtons(effectButtons)) return;
    if (checkButtons(melodyButtons)) return;
}
