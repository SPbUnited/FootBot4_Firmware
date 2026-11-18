#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

namespace drivers::buzzer
{

/** Equal-tempered note names. Rest creates silence for the selected duration. */
enum class Pitch : uint8_t
{
    C,
    CSharp,
    DFlat = CSharp,
    D,
    DSharp,
    EFlat = DSharp,
    E,
    F,
    FSharp,
    GFlat = FSharp,
    G,
    GSharp,
    AFlat = GSharp,
    A,
    ASharp,
    BFlat = ASharp,
    B,
    Rest,
};

/** Supported note lengths. Dotted values last 1.5 times their base length. */
enum class Length : uint8_t
{
    Whole,
    Half,
    Quarter,
    Eighth,
    Sixteenth,
    ThirtySecond,
    WholeDotted,
    HalfDotted,
    QuarterDotted,
    EighthDotted,
    SixteenthDotted,
    ThirtySecondDotted,
};

/** One named pitch, octave and rhythmic length. Octaves use C4 = middle C. Rest ignores octave. */
struct MelodyNote
{
    Pitch pitch;
    uint8_t octave;
    Length length;
};

/** Non-blocking, single-voice buzzer connected to PF13. */
class Buzzer
{
  public:
    /** Configure PF13 and TIM6. Returns false if another instance owns TIM6. */
    bool init();

    /** Copy and start up to 64 named notes if idle. Tempo range is 1..1000 BPM. */
    bool play(const MelodyNote *melody, size_t count, uint16_t tempo_bpm = 120);

    /** Stop playback and drive PF13 low. */
    void stop();

    /** Return whether a sequence is playing. */
    bool is_playing() const;

  private:
    friend void timer_irq();

    void start_note(uint32_t now_us);
    void timer_irq_handler();

    struct Tone
    {
        uint32_t frequency_hz;
        uint32_t duration_ms;
    };

    static Buzzer *instance_;
    Tone tones_[64]{};
    const Tone *sequence_ = nullptr;
    size_t count_ = 0;
    size_t index_ = 0;
    uint32_t note_started_us_ = 0;
    volatile bool playing_ = false;
    bool initialized_ = false;
};

/** TIM6 interrupt entry point. */
void timer_irq();

}  // namespace drivers::buzzer
