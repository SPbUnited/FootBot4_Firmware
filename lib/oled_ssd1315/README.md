<div align="center">

# 📟 OLED SSD1315

**Лёгкая библиотека для OLED дисплеев SSD1315/SSD1306**

[![Версия](https://img.shields.io/badge/версия-3.0.0-blue.svg)](CHANGELOG.md)
[![Лицензия](https://img.shields.io/badge/лицензия-MIT-green.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-совместим-orange.svg)](https://platformio.org)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org)

*Framebuffer • Графика • UTF-8 текст • Кириллица • Arduino • STM32 HAL*

[Быстрый старт](#-быстрый-старт) •
[API](#-api-reference) •
[Примеры](docs/EXAMPLES.md) •
[Документация](docs/)

</div>

---

## 📋 Содержание

- [Особенности](#-особенности)
- [Архитектура v3.0](#-архитектура-v30)
- [Установка](#-установка)
- [Быстрый старт](#-быстрый-старт)
- [API Reference](#-api-reference)
- [Тестирование](#-тестирование)
- [Документация](#-документация)

---

## ✨ Особенности

- **📟 Multi-Platform** — Arduino Wire и STM32 HAL I2C
- **🏗️ Clean Architecture** — ports/adapters/domain разделение слоёв
- **🔒 pImpl Pattern** — HAL-типы скрыты от публичного API
- **🧪 Unit Tests** — тесты с MockI2c, CMake поддержка
- **📝 Code Quality** — `.clang-format`, `.clang-tidy` конфигурации
- **Framebuffer** — все операции в памяти, затем `flush()` на дисплей
- **Примитивы** — пиксель, линия, прямоугольник (контур и заливка)
- **Текст** — шрифт 5×7, **поддержка русского языка (UTF-8)**, масштабирование
- **Условное включение** — заглушки при выключенной библиотеке

## 🏗️ Архитектура v3.0

```text
┌─────────────────────────────────────────────────┐
│              OledSsd1315 (Facade)               │  ← Публичный API
│                   + pImpl                       │
├─────────────────────────────────────────────────┤
│                    domain/                      │  ← Чистая логика
│              Gfx, Ssd1315Driver                 │
├─────────────────────────────────────────────────┤
│                    ports/                       │  ← Интерфейсы
│                     II2c                        │
├─────────────────────────────────────────────────┤
│                   adapters/                     │  ← Реализации
│       WireI2cAdapter, Stm32HalI2cAdapter        │
└─────────────────────────────────────────────────┘
```

### Структура файлов

```text
lib/oled_ssd1315/
├── include/oled/
│   ├── OledSsd1315.hpp         # Публичный API (Facade)
│   ├── OledSsd1315Impl.hpp     # pImpl детали реализации
│   ├── OledSsd1315Fwd.hpp      # Forward declarations
│   ├── OledConfig.hpp          # Конфигурация
│   ├── OledTypes.hpp           # Типы и enum'ы
│   ├── ports/                  # Интерфейсы (DIP)
│   │   └── II2c.hpp
│   ├── adapters/               # Платформенные реализации
│   │   ├── WireI2cAdapter.hpp
│   │   ├── Stm32HalI2cAdapter.hpp
│   │   └── PlatformDelay.hpp
│   └── domain/                 # Бизнес-логика (чистая)
│       ├── Gfx.hpp
│       ├── Ssd1315Driver.hpp
│       └── Ssd1315Commands.hpp
├── src/
├── tests/                      # Unit-тесты
│   ├── CMakeLists.txt
│   ├── mocks/MockI2c.hpp
│   ├── test_gfx.cpp
│   └── test_driver.cpp
├── examples/
│   └── stm32h743_test/         # Тест для STM32H743
├── .clang-format               # Автоформатирование
├── .clang-tidy                 # Статический анализ
└── library.json
```

## 💻 Поддерживаемые платформы

| Платформа | Framework | Флаги |
|-----------|-----------|------|
| ESP32/ESP8266 | Arduino | Автоматически |
| AVR/SAM | Arduino | Автоматически |
| STM32 (Arduino) | Arduino | Автоматически |
| **STM32 (HAL)** | stm32cube | `-DOLED_PLATFORM_STM32HAL=1` |

## 🔧 Установка

### PlatformIO

```ini
[env:myboard]
lib_deps = 
    oled_ssd1315

build_flags = 
    -DOLED_SSD1315_ENABLE=1
    ; Для STM32 HAL:
    ; -DOLED_PLATFORM_STM32HAL=1
```

### Локальная установка

1. Скопируйте `oled_ssd1315` в `lib/`
2. Добавьте флаг `-DOLED_SSD1315_ENABLE=1`

## 🚀 Быстрый старт

### Arduino

```cpp
#include <Wire.h>
#include <oled/OledSsd1315.hpp>

oled::OledSsd1315* display;

void setup() {
    Wire.begin();
    
    display = new oled::OledSsd1315(Wire);
    
    oled::OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;
    cfg.width = 128;
    cfg.height = 64;
    
    if (display->begin(cfg) != oled::OledResult::Ok) {
        return;
    }
    
    display->clear();
    display->print("Hello Arduino!");
    display->flush();
}

void loop() {}
```

### STM32 HAL

```cpp
#include "stm32h7xx_hal.h"
#include <oled/OledSsd1315.hpp>

I2C_HandleTypeDef hi2c1;
oled::OledSsd1315* display;

int main() {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    
    display = new oled::OledSsd1315(&hi2c1);
    
    oled::OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;
    
    display->begin(cfg);
    display->clear();
    display->print("Привет STM32!");
    display->flush();
    
    while (1) {}
}
```

## 📖 API Reference

### Результаты операций

```cpp
enum class OledResult {
    Ok,             // Успех
    Disabled,       // Библиотека отключена
    I2cError,       // Ошибка I2C
    NotInitialized, // Не инициализирован
    InvalidArg,     // Неверный аргумент
    Busy,           // Занят (DMA)
    Timeout         // Таймаут
};
```

### Основные методы

| Метод | Описание |
|-------|----------|
| `begin(cfg)` | Инициализация дисплея |
| `isReady()` | Проверка готовности |
| `clear()` | Очистить буфер |
| `flush()` | Отправить на дисплей |
| `print(str)` | Вывод строки |
| `printf(fmt, ...)` | Форматированный вывод |

### Примитивы

| Метод | Описание |
|-------|----------|
| `pixel(x, y, color)` | Пиксель |
| `line(x0, y0, x1, y1, color)` | Линия |
| `rect(x, y, w, h, color)` | Прямоугольник |
| `rectFill(x, y, w, h, color)` | Залитый прямоугольник |

### STM32 HAL специфичные

| Метод | Описание |
|-------|----------|
| `flushDMA()` | Non-blocking передача |
| `isDMAComplete()` | Проверка завершения DMA |
| `i2cBusRecovery()` | Восстановление I2C шины |

## 🧪 Тестирование

```bash
cd tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest --output-on-failure
```

Тесты включают:
- `test_gfx` — графический слой
- `test_driver` — драйвер SSD1315 с MockI2c

## 📚 Документация

- [Архитектура](docs/ARCHITECTURE.md)
- [API Reference](docs/API.md)
- [STM32 HAL Guide](docs/STM32_HAL.md)
- [Быстрый старт](docs/QUICKSTART.md)
- [Команды SSD1315](docs/SSD1315_COMMANDS.md)

## 📜 Лицензия

MIT License. См. [LICENSE](LICENSE).

---

<div align="center">

**Сделано с ❤️ для embedded-сообщества**

</div>
