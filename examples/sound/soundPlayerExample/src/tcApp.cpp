// =============================================================================
// soundPlayerExample - サウンド再生サンプル
//
// 使用音源:
//   "113 2b loose-pants 4.2 mono" by astro_denticle
//   https://freesound.org/
//   License: CC0 (Public Domain)
//   Thanks to astro_denticle for sharing this great beat loop!
// =============================================================================

#include "tcApp.h"
#include "TrussC.h"

void tcApp::setup() {
    tc::setVsync(true);

    // macOS バンドルの場合、data フォルダへのパスを設定
    // 実行ファイル: bin/xxx.app/Contents/MacOS/xxx
    // data: bin/data/
    // ../../../ = bin/ なので ../../../data/ が正しいパス
    #ifdef __APPLE__
    tc::setDataPathRoot("../../../data/");
    #endif

    // 音声ファイルパス（data フォルダ内の CC0 サンプル音源）
    musicPath = tc::getDataPath("beat_loop.wav");
    printf("Trying to load: %s\n", musicPath.c_str());

    // 音楽をロード
    if (music.load(musicPath)) {
        musicLoaded = true;
        music.setLoop(true);
        printf("Music loaded: %s (%.1f sec)\n", musicPath.c_str(), music.getDuration());
    } else {
        printf("Music not found: %s - using test tone\n", musicPath.c_str());
        music.loadTestTone(440.0f, 3.0f);  // A4 (440Hz), 3秒
        music.setLoop(true);
        musicLoaded = true;
    }

    // 効果音（テストトーンを使用）
    sfx.loadTestTone(880.0f, 0.2f);  // A5 (880Hz), 0.2秒
    sfxLoaded = true;

    printf("\n=== Controls ===\n");
    printf("SPACE: Play/Stop music\n");
    printf("P: Pause/Resume music\n");
    printf("S: Play sound effect\n");
    printf("UP/DOWN: Volume control\n");
    printf("LEFT/RIGHT: Pan control\n");
    printf("+/-: Speed control\n");
    printf("================\n\n");
}

void tcApp::draw() {
    tc::clear(30);

    float y = 50;

    // タイトル
    tc::setColor(tc::colors::white);
    tc::drawBitmapString("TrussC Sound Player Example", 50, y);
    y += 40;

    // コントロール説明
    tc::setColor(180);
    tc::drawBitmapString("Controls:", 50, y);
    y += 25;
    tc::drawBitmapString("  SPACE - Play/Stop music", 50, y);
    y += 20;
    tc::drawBitmapString("  P - Pause/Resume music", 50, y);
    y += 20;
    tc::drawBitmapString("  S - Play sound effect", 50, y);
    y += 20;
    tc::drawBitmapString("  UP/DOWN - Volume control", 50, y);
    y += 20;
    tc::drawBitmapString("  LEFT/RIGHT - Pan control", 50, y);
    y += 20;
    tc::drawBitmapString("  +/- - Speed control", 50, y);
    y += 40;

    // 音楽の状態
    tc::setColor(tc::colors::white);
    tc::drawBitmapString("=== Music ===", 50, y);
    y += 25;

    if (musicLoaded) {
        std::string status = music.isPlaying() ? "Playing" :
                            (music.isPaused() ? "Paused" : "Stopped");
        tc::setColor(music.isPlaying() ? tc::colors::lime : tc::colors::gray);
        tc::drawBitmapString("Status: " + status, 50, y);
        y += 20;

        tc::setColor(180);
        char buf[128];
        snprintf(buf, sizeof(buf), "Position: %.1f / %.1f sec",
                music.getPosition(), music.getDuration());
        tc::drawBitmapString(buf, 50, y);
        y += 20;

        snprintf(buf, sizeof(buf), "Volume: %.0f%%", music.getVolume() * 100);
        tc::drawBitmapString(buf, 50, y);
        y += 20;

        snprintf(buf, sizeof(buf), "Pan: %.1f (%s)", music.getPan(),
                music.getPan() < -0.1f ? "Left" : music.getPan() > 0.1f ? "Right" : "Center");
        tc::drawBitmapString(buf, 50, y);
        y += 20;

        snprintf(buf, sizeof(buf), "Speed: %.1fx", music.getSpeed());
        tc::drawBitmapString(buf, 50, y);
        y += 20;

        tc::drawBitmapString(std::string("Loop: ") + (music.isLoop() ? "ON" : "OFF"), 50, y);
        y += 30;

        // プログレスバー
        float progress = music.getDuration() > 0 ?
                        music.getPosition() / music.getDuration() : 0;
        tc::setColor(60);
        tc::drawRect(50, y, 300, 20);
        tc::setColor(tc::colors::dodgerBlue);
        tc::drawRect(50, y, 300 * progress, 20);
        y += 40;
    } else {
        tc::setColor(tc::colors::red);
        tc::drawBitmapString("Music file not found: " + musicPath, 50, y);
        y += 40;
    }

    // 効果音の状態
    tc::setColor(tc::colors::white);
    tc::drawBitmapString("=== Sound Effect ===", 50, y);
    y += 25;

    std::string status = sfx.isPlaying() ? "Playing" : "Ready";
    tc::setColor(sfx.isPlaying() ? tc::colors::lime : tc::colors::gray);
    tc::drawBitmapString("Status: " + status, 50, y);

    // FPS
    tc::setColor(100);
    tc::drawBitmapString("FPS: " + std::to_string((int)tc::getFrameRate()), 50, 560);
}

void tcApp::keyPressed(int key) {
    if (key == ' ') {
        // スペース: 音楽再生/停止
        if (musicLoaded) {
            if (music.isPlaying() || music.isPaused()) {
                music.stop();
                printf("Music stopped\n");
            } else {
                music.play();
                printf("Music playing\n");
            }
        }
    }
    else if (key == 'p' || key == 'P') {
        // P: 一時停止/再開
        if (musicLoaded) {
            if (music.isPaused()) {
                music.resume();
                printf("Music resumed\n");
            } else if (music.isPlaying()) {
                music.pause();
                printf("Music paused\n");
            }
        }
    }
    else if (key == 's' || key == 'S') {
        // S: 効果音再生
        if (sfxLoaded) {
            sfx.play();
            printf("SFX playing\n");
        }
    }
    else if (key == SAPP_KEYCODE_UP) {
        // 音量上げる
        float vol = music.getVolume() + 0.1f;
        if (vol > 1.0f) vol = 1.0f;
        music.setVolume(vol);
        printf("Volume: %.0f%%\n", vol * 100);
    }
    else if (key == SAPP_KEYCODE_DOWN) {
        // 音量下げる
        float vol = music.getVolume() - 0.1f;
        if (vol < 0.0f) vol = 0.0f;
        music.setVolume(vol);
        printf("Volume: %.0f%%\n", vol * 100);
    }
    else if (key == SAPP_KEYCODE_LEFT) {
        // パン左へ
        float pan = music.getPan() - 0.1f;
        music.setPan(pan);
        printf("Pan: %.1f\n", music.getPan());
    }
    else if (key == SAPP_KEYCODE_RIGHT) {
        // パン右へ
        float pan = music.getPan() + 0.1f;
        music.setPan(pan);
        printf("Pan: %.1f\n", music.getPan());
    }
    else if (key == '+' || key == '=' || key == SAPP_KEYCODE_KP_ADD) {
        // 速度上げる
        float speed = music.getSpeed() + 0.1f;
        music.setSpeed(speed);
        printf("Speed: %.1fx\n", music.getSpeed());
    }
    else if (key == '-' || key == SAPP_KEYCODE_KP_SUBTRACT) {
        // 速度下げる
        float speed = music.getSpeed() - 0.1f;
        music.setSpeed(speed);
        printf("Speed: %.1fx\n", music.getSpeed());
    }
}
