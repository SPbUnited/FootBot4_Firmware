#include "buzzer.hpp"

#include "drivers/system_clock/system_clock.hpp"

namespace drivers::buzzer
{
namespace
{
constexpr uint32_t timer_tick_hz = 1000000;
constexpr uint32_t rest_tick_hz = 1000;
constexpr uint32_t minimum_frequency_hz = 20;
constexpr uint32_t maximum_frequency_hz = 4000;
constexpr uint32_t timer_irq_priority = 15;
constexpr uint32_t buzzer_pin = GPIO_PIN_13;
constexpr size_t maximum_melody_notes = 64;
constexpr uint16_t maximum_tempo_bpm = 1000;
constexpr uint16_t minimum_tempo_bpm = 1;
constexpr uint32_t maximum_duration_ms = 2147483;

bool convert_note(const MelodyNote &note, uint16_t tempo_bpm, uint32_t &frequency_hz,
                  uint32_t &duration_ms)
{
    constexpr uint16_t octave_four_hz[] = {262, 277, 294, 311, 330, 349,
                                           370, 392, 415, 440, 466, 494};
    uint32_t quarter_ms = 60000 / tempo_bpm;
    bool dotted = false;
    Length base_length = note.length;
    switch (note.length)
    {
        case Length::WholeDotted:
            base_length = Length::Whole;
            dotted = true;
            break;
        case Length::HalfDotted:
            base_length = Length::Half;
            dotted = true;
            break;
        case Length::QuarterDotted:
            base_length = Length::Quarter;
            dotted = true;
            break;
        case Length::EighthDotted:
            base_length = Length::Eighth;
            dotted = true;
            break;
        case Length::SixteenthDotted:
            base_length = Length::Sixteenth;
            dotted = true;
            break;
        case Length::ThirtySecondDotted:
            base_length = Length::ThirtySecond;
            dotted = true;
            break;
        default:
            break;
    }

    switch (base_length)
    {
        case Length::Whole:
            duration_ms = quarter_ms * 4;
            break;
        case Length::Half:
            duration_ms = quarter_ms * 2;
            break;
        case Length::Quarter:
            duration_ms = quarter_ms;
            break;
        case Length::Eighth:
            duration_ms = quarter_ms / 2;
            break;
        case Length::Sixteenth:
            duration_ms = quarter_ms / 4;
            break;
        case Length::ThirtySecond:
            duration_ms = quarter_ms / 8;
            break;
        default:
            return false;
    }
    if (dotted)
    {
        duration_ms = (duration_ms * 3 + 1) / 2;
    }
    if (duration_ms == 0 || duration_ms > maximum_duration_ms)
    {
        return false;
    }

    if (note.pitch == Pitch::Rest)
    {
        frequency_hz = 0;
        return true;
    }

    uint8_t pitch = static_cast<uint8_t>(note.pitch);
    if (pitch >= 12 || note.octave < 1 || note.octave > 7)
    {
        return false;
    }
    frequency_hz = octave_four_hz[pitch];
    if (note.octave > 4)
    {
        frequency_hz <<= note.octave - 4;
    }
    else
    {
        frequency_hz >>= 4 - note.octave;
    }
    return frequency_hz >= minimum_frequency_hz && frequency_hz <= maximum_frequency_hz;
}
}  // namespace

Buzzer *Buzzer::instance_ = nullptr;

bool Buzzer::init()
{
    if (instance_ != nullptr && instance_ != this)
    {
        return false;
    }
    instance_ = this;

    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef gpio_config{};
    gpio_config.Pin = buzzer_pin;
    gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_config.Pull = GPIO_PULLDOWN;
    gpio_config.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &gpio_config);
    HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);

    __HAL_RCC_TIM6_CLK_ENABLE();
    uint32_t timer_clock_hz = HAL_RCC_GetPCLK1Freq();
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
    {
        timer_clock_hz *= 2;
    }
    if (timer_clock_hz < timer_tick_hz || timer_clock_hz % timer_tick_hz != 0)
    {
        instance_ = nullptr;
        return false;
    }

    TIM6->CR1 = 0;
    TIM6->PSC = (timer_clock_hz / timer_tick_hz) - 1;
    TIM6->ARR = (timer_tick_hz / rest_tick_hz) - 1;
    TIM6->EGR = TIM_EGR_UG;
    TIM6->SR = 0;
    TIM6->DIER = TIM_DIER_UIE;
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, timer_irq_priority, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    initialized_ = true;
    return true;
}

bool Buzzer::play(const MelodyNote *melody, size_t count, uint16_t tempo_bpm)
{
    if (initialized_ && !playing_ && melody != nullptr && count > 0 &&
        count <= maximum_melody_notes && tempo_bpm >= minimum_tempo_bpm &&
        tempo_bpm <= maximum_tempo_bpm)
    {
        for (size_t note = 0; note < count; ++note)
        {
            if (!convert_note(melody[note], tempo_bpm, tones_[note].frequency_hz,
                              tones_[note].duration_ms))
            {
                return false;
            }
        }

        uint32_t interrupt_state = __get_PRIMASK();
        __disable_irq();
        sequence_ = tones_;
        count_ = count;
        index_ = 0;
        note_started_us_ = drivers::system_clock::micros();
        playing_ = true;
        start_note(note_started_us_);
        TIM6->CNT = 0;
        TIM6->SR = 0;
        TIM6->CR1 |= TIM_CR1_CEN;
        if (interrupt_state == 0)
        {
            __enable_irq();
        }
        return true;
    }
    return false;
}

void Buzzer::stop()
{
    uint32_t interrupt_state = __get_PRIMASK();
    __disable_irq();
    playing_ = false;
    sequence_ = nullptr;
    count_ = 0;
    index_ = 0;
    TIM6->CR1 &= ~TIM_CR1_CEN;
    HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
    if (interrupt_state == 0)
    {
        __enable_irq();
    }
}

bool Buzzer::is_playing() const
{
    return playing_;
}

void Buzzer::start_note(uint32_t now_us)
{
    note_started_us_ = now_us;
    uint32_t frequency = sequence_[index_].frequency_hz;
    if (frequency == 0)
    {
        HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
        TIM6->ARR = (timer_tick_hz / rest_tick_hz) - 1;
        return;
    }

    uint32_t half_period_us = timer_tick_hz / (frequency * 2);
    TIM6->ARR = half_period_us - 1;
    HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
}

void Buzzer::timer_irq_handler()
{
    if (!playing_)
    {
        TIM6->CR1 &= ~TIM_CR1_CEN;
        HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
        return;
    }

    uint32_t now_us = drivers::system_clock::micros();
    uint32_t duration_us = sequence_[index_].duration_ms * 1000;
    if (now_us - note_started_us_ >= duration_us)
    {
        ++index_;
        if (index_ == count_)
        {
            playing_ = false;
            sequence_ = nullptr;
            count_ = 0;
            index_ = 0;
            TIM6->CR1 &= ~TIM_CR1_CEN;
            HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
            return;
        }
        start_note(now_us);
        return;
    }

    if (sequence_[index_].frequency_hz == 0)
    {
        HAL_GPIO_WritePin(GPIOF, buzzer_pin, GPIO_PIN_RESET);
        return;
    }
    HAL_GPIO_TogglePin(GPIOF, buzzer_pin);
    uint32_t remaining_us = duration_us - (now_us - note_started_us_);
    uint32_t half_period_us = timer_tick_hz / (sequence_[index_].frequency_hz * 2);
    if (remaining_us < half_period_us)
    {
        TIM6->ARR = remaining_us > 1 ? remaining_us - 1 : 1;
    }
}

void timer_irq()
{
    if (Buzzer::instance_ != nullptr)
    {
        Buzzer::instance_->timer_irq_handler();
    }
}

}  // namespace drivers::buzzer

extern "C" void TIM6_DAC_IRQHandler(void)
{
    TIM6->SR = 0;
    drivers::buzzer::timer_irq();
}
