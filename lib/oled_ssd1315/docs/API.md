# API Reference — OLED SSD1315 v3.0

## Namespace

Все типы находятся в namespace `oled`:

```cpp
#include <oled/OledSsd1315.hpp>

oled::OledSsd1315* display;
oled::OledConfig cfg;
oled::OledResult result;
```

---

## Типы

### OledResult

```cpp
enum class OledResult {
    Ok,             // Успех
    Disabled,       // Библиотека отключена флагом
    I2cError,       // Ошибка I2C (NACK, timeout)
    NotInitialized, // begin() не вызван или неуспешен
    InvalidArg,     // Неверный аргумент
    Busy,           // Занят (DMA в процессе)
    Timeout,        // Таймаут операции
    Unsupported     // Операция не поддерживается
};
```

### VccMode

```cpp
enum class VccMode {
    InternalChargePump,  // Встроенный charge pump (большинство модулей)
    ExternalVcc          // Внешнее питание VCC
};
```

### OledConfig

```cpp
struct OledConfig {
    uint8_t  i2cAddr7 = 0x3C;      // 7-битный I2C адрес
    uint16_t width    = 128;       // Ширина в пикселях
    uint16_t height   = 64;        // Высота (32 или 64)
    uint32_t i2cFreq  = 400000;    // Частота I2C (для Arduino)
    VccMode  vccMode  = VccMode::InternalChargePump;
    bool     flip180  = false;     // Поворот на 180°
    ResetGpioCallback resetCallback = nullptr;  // Callback для reset
};
```

### ResetGpioCallback

```cpp
using ResetGpioCallback = void (*)(bool high);
```

Callback для управления пином RST:
- `high = true` — RST HIGH
- `high = false` — RST LOW

---

## Класс OledSsd1315

### Конструкторы

#### Arduino

```cpp
explicit OledSsd1315(TwoWire& wire);
```

**Параметры:**
- `wire` — ссылка на объект Wire (I2C)

**Пример:**
```cpp
#include <Wire.h>
oled::OledSsd1315* display = new oled::OledSsd1315(Wire);
```

#### STM32 HAL

```cpp
explicit OledSsd1315(I2C_HandleTypeDef* hi2c);
```

**Параметры:**
- `hi2c` — указатель на HAL I2C handle

**Пример:**
```cpp
extern I2C_HandleTypeDef hi2c1;
oled::OledSsd1315* display = new oled::OledSsd1315(&hi2c1);
```

### Деструктор

```cpp
~OledSsd1315();
```

Освобождает внутренние ресурсы (pImpl).

---

## Инициализация

### begin

```cpp
OledResult begin(const OledConfig& cfg);
```

Инициализирует дисплей с заданной конфигурацией.

**Параметры:**
- `cfg` — структура конфигурации

**Возвращает:**
- `OledResult::Ok` — успех
- `OledResult::InvalidArg` — неверные параметры
- `OledResult::I2cError` — ошибка I2C

**Пример:**
```cpp
oled::OledConfig cfg;
cfg.i2cAddr7 = 0x3C;
cfg.width = 128;
cfg.height = 64;

if (display->begin(cfg) != oled::OledResult::Ok) {
    // Ошибка
}
```

### isReady

```cpp
bool isReady() const;
```

Проверяет готовность дисплея к работе.

### resetState

```cpp
void resetState();
```

Сбрасывает внутреннее состояние (без деинициализации I2C).

---

## Управление дисплеем

### setPower

```cpp
OledResult setPower(bool on);
```

Включает/выключает дисплей (с корректным управлением charge pump).

### setContrast

```cpp
OledResult setContrast(uint8_t value);
```

Устанавливает контраст (0-255).

### invert

```cpp
OledResult invert(bool on);
```

Инвертирует цвета дисплея.

---

## Буфер

### clear

```cpp
void clear();
```

Очищает буфер (заполняет нулями).

### fill

```cpp
void fill(bool color);
```

Заполняет буфер цветом:
- `true` — белый (все пиксели включены)
- `false` — чёрный

### flush

```cpp
OledResult flush();
```

Отправляет буфер на дисплей (blocking).

---

## Графические примитивы

### pixel

```cpp
void pixel(int x, int y, bool color);
```

Устанавливает пиксель.

**Параметры:**
- `x, y` — координаты (0-based)
- `color` — true = белый, false = чёрный

### line

```cpp
void line(int x0, int y0, int x1, int y1, bool color);
```

Рисует линию (алгоритм Брезенхэма).

### rect

```cpp
void rect(int x, int y, int w, int h, bool color);
```

Рисует прямоугольник (контур).

### rectFill

```cpp
void rectFill(int x, int y, int w, int h, bool color);
```

Рисует залитый прямоугольник.

---

## Текст

### setCursor

```cpp
void setCursor(int x, int y);
```

Устанавливает позицию курсора для текста.

### setTextSize

```cpp
void setTextSize(uint8_t scale);
```

Устанавливает масштаб текста (1, 2, 3...).

### setTextColor

```cpp
void setTextColor(bool color);
```

Устанавливает цвет текста.

### print

```cpp
void print(const char* str);
```

Выводит строку (поддерживает UTF-8 и кириллицу).

### printf

```cpp
void printf(const char* fmt, ...);
```

Форматированный вывод (как стандартный printf).

**Пример:**
```cpp
display->printf("Темп: %d°C", 25);
display->printf("Напр: %.1fV", 3.3f);
```

---

## Диагностика

### getLastResult

```cpp
OledResult getLastResult() const;
```

Возвращает результат последней операции.

### getLastError

```cpp
const char* getLastError() const;
```

Возвращает текстовое описание последней ошибки.

### scanAddress

```cpp
uint8_t scanAddress(uint8_t startAddr = 0x3C, uint8_t endAddr = 0x3D);
```

Сканирует I2C шину в диапазоне адресов.

**Возвращает:**
- Найденный адрес (7-bit)
- 0 если устройство не найдено

---

## STM32 HAL специфичные

### flushDMA

```cpp
OledResult flushDMA();
```

Начинает non-blocking DMA передачу буфера.

**Требования:**
- Настроенный DMA для I2C TX в CubeMX
- Callback `onDmaComplete()` в ISR

### isDMAComplete

```cpp
bool isDMAComplete() const;
```

Проверяет завершение DMA передачи.

### onDmaComplete

```cpp
void onDmaComplete();
```

Callback для вызова из `HAL_I2C_MasterTxCpltCallback`.

**Пример:**
```cpp
// В stm32xx_it.c
extern oled::OledSsd1315* display;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    display->onDmaComplete();
}
```

### i2cBusRecovery

```cpp
static bool i2cBusRecovery(void* gpioPort, uint16_t sclPin, uint16_t sdaPin);
```

Восстанавливает зависшую I2C шину.

**Параметры:**
- `gpioPort` — GPIO порт (GPIOB и т.д.)
- `sclPin` — GPIO_PIN_x для SCL
- `sdaPin` — GPIO_PIN_x для SDA

**Пример:**
```cpp
bool recovered = oled::OledSsd1315::i2cBusRecovery(GPIOB, GPIO_PIN_6, GPIO_PIN_7);
if (recovered) {
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_Init(&hi2c1);
}
```

---

## Константы

Определены в `OledConfig.hpp`:

| Константа | Значение | Описание |
|-----------|----------|----------|
| `OLED_MAX_BUFFER_SIZE` | 1024 | Макс. размер буфера (128×64) |
| `OLED_I2C_CHUNK_SIZE` | 128/32 | Размер I2C пакета |
| `OLED_PRINTF_BUFFER_SIZE` | 64 | Буфер для printf |

---

## Флаги компиляции

| Флаг | Описание |
|------|----------|
| `OLED_SSD1315_ENABLE=1` | Включить библиотеку |
| `OLED_PLATFORM_STM32HAL=1` | Использовать STM32 HAL |
| `OLED_PLATFORM_ARDUINO=1` | Явно указать Arduino |
