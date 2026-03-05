# Changelog

Все значимые изменения в библиотеке OLED SSD1315.

Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.0.0/).

## [3.0.0] - 2025-01-10

### ⚠️ Breaking Changes

- **Архитектура полностью переработана** — Clean Architecture с разделением на слои
- **pImpl pattern** — HAL-типы скрыты от публичного API
- **Namespace обязателен** — используйте `oled::OledSsd1315` вместо `OledSsd1315`
- **Указатели вместо объектов** — рекомендуется `new oled::OledSsd1315(...)` для STM32

### Добавлено

- **Архитектурные слои:**
  - `ports/` — интерфейсы (II2c)
  - `adapters/` — платформенные реализации (WireI2cAdapter, Stm32HalI2cAdapter)
  - `domain/` — чистая бизнес-логика (Gfx, Ssd1315Driver)
  
- **pImpl Idiom:**
  - `OledSsd1315Impl.hpp` — детали реализации скрыты
  - `OledSsd1315Fwd.hpp` — forward declarations для минимизации зависимостей
  - HAL-типы не утекают в публичные заголовки

- **Code Quality:**
  - `.clang-format` — автоматическое форматирование (Google-based, 4 spaces)
  - `.clang-tidy` — статический анализ (bugprone, modernize, performance)

- **Unit Testing:**
  - `tests/` — структура для unit-тестов
  - `tests/mocks/MockI2c.hpp` — mock-реализация II2c
  - `tests/test_gfx.cpp` — тесты графического слоя
  - `tests/test_driver.cpp` — тесты драйвера
  - `tests/CMakeLists.txt` — сборка тестов с CMake + ASan/UBSan

- **Примеры:**
  - `examples/stm32h743_test/` — тестовый проект для STM32H743

- **Именованные константы:**
  - `kI2cRecoveryClockPulses` — количество clock pulses для recovery
  - `kI2cBitBangDelayLoops` — задержки для bit-banging
  - `kI2cDataCommandPrefix` — префикс команды данных (0x40)

### Изменено

- **Структура include:**
  ```text
  include/oled/
  ├── ports/II2c.hpp              (было: internal/)
  ├── adapters/WireI2cAdapter.hpp (было: internal/)
  ├── adapters/Stm32HalI2cAdapter.hpp
  ├── domain/Gfx.hpp              (было: internal/)
  ├── domain/Ssd1315Driver.hpp    (было: internal/)
  └── domain/Ssd1315Commands.hpp
  ```

- **Убраны using declarations из заголовков** — соответствие стандартам C++

### Удалено

- `include/oled/internal/` — заменено на `ports/`, `adapters/`, `domain/`
- Глобальные `using oled::*` из публичных заголовков

---

## [2.1.2] - 2025-01-09

### Исправлено

- **WireI2cAdapter guards** — добавлена проверка `OLED_USE_ARDUINO`
- **Build script** — добавлен `srcFilter` для исключения файлов

---

## [2.1.1] - 2025-01-09

### Исправлено

- `scanAddress()` — исправлена работа на Arduino
- DMA callback — добавлен `onDmaComplete()`
- `writeCommands()` — команды одним I2C пакетом
- Символ `~` — исправлен глиф

### Добавлено

- `II2c::probe()` — проверка устройства на шине
- `.editorconfig` — настройки стиля

---

## [2.1.0] - 2025-01-09

### Добавлено

- Диагностика: `getLastResult()`, `getLastError()`
- I2C сканер: `scanAddress()`
- DMA поддержка: `flushDMA()`, `isDMAComplete()`
- I2C Bus Recovery: `i2cBusRecovery()`

---

## [2.0.0] - 2025-01-09

### Добавлено

- Поддержка STM32 HAL I2C
- Platform-agnostic архитектура
- `Stm32HalI2cAdapter`
- `PlatformDelay.hpp`
- Скрипт `platformio_build.py`

---

## [1.1.0] - 2025-01-09

### Изменено

- Убрано динамическое выделение памяти
- Добавлены `init()` методы
- Перенос internal headers

---

## [1.0.0] - 2024-01-09

### Добавлено

- Первый релиз
- SSD1315/SSD1306 поддержка
- I2C через Arduino Wire
- Framebuffer с графикой
- UTF-8 текст с кириллицей
