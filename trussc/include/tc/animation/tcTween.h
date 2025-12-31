#pragma once

#include "tcEasing.h"
#include "../events/tcEvent.h"
#include "../events/tcEventListener.h"
#include <memory>

namespace trussc {

// Tween class for animating values with easing
// Works with any type that supports lerp (float, Vec2, Vec3, Vec4, Color, etc.)
template<typename T>
class Tween {
public:
    // Completion event - fired when animation finishes
    // Stored as unique_ptr to allow move semantics
    std::unique_ptr<Event<void>> complete;

    // Default constructor
    Tween() : complete(std::make_unique<Event<void>>()) {}

    // Move constructor
    Tween(Tween&& other) noexcept = default;
    Tween& operator=(Tween&& other) noexcept = default;

    // No copy
    Tween(const Tween&) = delete;
    Tween& operator=(const Tween&) = delete;

    // Full constructor
    Tween(T start, T end, float duration,
          EaseType type = EaseType::Cubic,
          EaseMode mode = EaseMode::InOut)
        : complete(std::make_unique<Event<void>>())
        , start_(start)
        , end_(end)
        , duration_(duration)
        , easeType_(type)
        , easeTypeOut_(type)
        , mode_(mode) {}

    // ----- Chainable setters -----

    Tween& from(T value) {
        start_ = value;
        return *this;
    }

    Tween& to(T value) {
        end_ = value;
        return *this;
    }

    Tween& duration(float seconds) {
        duration_ = seconds;
        return *this;
    }

    Tween& ease(EaseType type, EaseMode mode = EaseMode::InOut) {
        easeType_ = type;
        easeTypeOut_ = type;
        mode_ = mode;
        asymmetric_ = false;
        return *this;
    }

    // Asymmetric ease: different types for in and out
    Tween& ease(EaseType inType, EaseType outType) {
        easeType_ = inType;
        easeTypeOut_ = outType;
        mode_ = EaseMode::InOut;
        asymmetric_ = true;
        return *this;
    }

    // ----- Control -----

    void start() {
        elapsed_ = 0.0f;
        playing_ = true;
        completed_ = false;
    }

    void pause() {
        playing_ = false;
    }

    void resume() {
        if (!completed_) {
            playing_ = true;
        }
    }

    void reset() {
        elapsed_ = 0.0f;
        playing_ = false;
        completed_ = false;
    }

    // Finish immediately (jump to end)
    void finish() {
        elapsed_ = duration_;
        playing_ = false;
        if (!completed_) {
            completed_ = true;
            if (complete) complete->notify();
        }
    }

    // ----- Update -----

    void update(float deltaTime) {
        if (!playing_ || completed_) return;

        elapsed_ += deltaTime;

        if (elapsed_ >= duration_) {
            elapsed_ = duration_;
            playing_ = false;
            completed_ = true;
            if (complete) complete->notify();
        }
    }

    // ----- Getters -----

    T getValue() const {
        float t = getProgress();
        float easedT = applyEasing(t);
        return lerpValue(start_, end_, easedT);
    }

    float getProgress() const {
        if (duration_ <= 0.0f) return 1.0f;
        return std::min(elapsed_ / duration_, 1.0f);
    }

    float getElapsed() const { return elapsed_; }
    float getDuration() const { return duration_; }

    bool isPlaying() const { return playing_; }
    bool isComplete() const { return completed_; }

    T getStart() const { return start_; }
    T getEnd() const { return end_; }

private:
    T start_{};
    T end_{};
    float duration_ = 1.0f;
    float elapsed_ = 0.0f;
    EaseType easeType_ = EaseType::Cubic;
    EaseType easeTypeOut_ = EaseType::Cubic;
    EaseMode mode_ = EaseMode::InOut;
    bool playing_ = false;
    bool completed_ = false;
    bool asymmetric_ = false;

    float applyEasing(float t) const {
        if (asymmetric_) {
            return easeInOut(t, easeType_, easeTypeOut_);
        }
        return trussc::ease(t, easeType_, mode_);
    }

    // Generic lerp - works with float
    template<typename U = T>
    static typename std::enable_if<std::is_same<U, float>::value, U>::type
    lerpValue(U a, U b, float t) {
        return a + (b - a) * t;
    }

    // Generic lerp - works with types that have a lerp method (Vec2, Vec3, etc.)
    template<typename U = T>
    static typename std::enable_if<!std::is_same<U, float>::value, U>::type
    lerpValue(const U& a, const U& b, float t) {
        return a.lerp(b, t);
    }
};

} // namespace trussc
