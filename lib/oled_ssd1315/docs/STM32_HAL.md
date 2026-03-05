# STM32 HAL Guide — OLED SSD1315 v3.0

## Обзор

Библиотека полностью поддерживает STM32 с framework `stm32cube` (HAL/LL).

## Требования

- PlatformIO с `platform = ststm32`
- `framework = stm32cube`
- Настроенный I2C (CubeMX или вручную)

## Настройка PlatformIO

```ini
[env:stm32h743]
platform = ststm32
board = nucleo_h743zi  ; или другая плата
framework = stm32cube

build_flags = 
    -DOLED_SSD1315_ENABLE=1
    -DOLED_PLATFORM_STM32HAL=1

; Для локальной библиотеки:
lib_extra_dirs = 
    ${PROJECT_DIR}/../../..

; Или из реестра:
lib_deps = 
    oled_ssd1315
```

---

## Инициализация I2C

### Вариант 1: CubeMX

1. **Connectivity → I2C1 → I2C**
2. **Parameter Settings:**
   - Speed Mode: Fast Mode (400 kHz)
   - Clock Speed: 400000
3. **GPIO Settings:** автоматически (Open-Drain + Pull-up)
4. **Generate Code**

### Вариант 2: Вручную

```cpp
I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    // GPIO: PB6 (SCL), PB7 (SDA)
    GPIO_InitTypeDef gpio = {};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gpio);
    
    // I2C
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00C0EAFF;  // 400 kHz @ 120 MHz APB1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);
}
```

---

## Минимальный пример

```cpp
#include "stm32h7xx_hal.h"
#include <oled/OledSsd1315.hpp>

I2C_HandleTypeDef hi2c1;
oled::OledSsd1315* display = nullptr;

int main() {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    
    HAL_Delay(100);
    
    // Создаём дисплей
    display = new oled::OledSsd1315(&hi2c1);
    
    // Конфигурация
    oled::OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;
    cfg.width = 128;
    cfg.height = 64;
    cfg.vccMode = oled::VccMode::InternalChargePump;
    
    // Инициализация
    if (display->begin(cfg) != oled::OledResult::Ok) {
        Error_Handler();
    }
    
    // Вывод
    display->clear();
    display->setCursor(0, 0);
    display->print("Привет STM32!");
    display->flush();
    
    while (1) {
        HAL_Delay(1000);
    }
}

extern "C" void SysTick_Handler() {
    HAL_IncTick();
}
```

---

## DMA передача (non-blocking)

### Настройка в CubeMX

1. **I2C1 → DMA Settings → Add**
2. **DMA Request:** I2C1_TX
3. **Mode:** Normal
4. **Priority:** Medium
5. **Memory Increment:** Enabled

### Использование

```cpp
// Начать DMA передачу
oled::OledResult res = display->flushDMA();

if (res == oled::OledResult::Ok) {
    // Передача начата, можно делать другую работу
    while (!display->isDMAComplete()) {
        processOtherTasks();
    }
}
```

### Callback

В `stm32xx_it.c`:

```cpp
extern oled::OledSsd1315* display;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (display) {
        display->onDmaComplete();
    }
}
```

---

## I2C Bus Recovery

Если SDA зависла в LOW:

```cpp
bool recovered = oled::OledSsd1315::i2cBusRecovery(
    GPIOB, GPIO_PIN_6, GPIO_PIN_7);

if (recovered) {
    HAL_I2C_DeInit(&hi2c1);
    HAL_I2C_Init(&hi2c1);
    display->begin(cfg);
}
```

---

## Диагностика

```cpp
// Сканирование адреса
uint8_t addr = display->scanAddress(0x3C, 0x3D);

// Последняя ошибка
const char* err = display->getLastError();

// Код результата
oled::OledResult res = display->getLastResult();
```

---

## Пины I2C

| Серия | I2C1 SCL | I2C1 SDA | AF |
|-------|----------|----------|-----|
| STM32F4 | PB6 | PB7 | AF4 |
| STM32H7 | PB6 | PB7 | AF4 |
| STM32F1 | PB6 | PB7 | - |
| STM32L4 | PB6 | PB7 | AF4 |

---

## Timing для разных частот

### STM32H7 @ 64 MHz APB1

| Скорость | Timing |
|----------|--------|
| 100 kHz | 0x40604E73 |
| 400 kHz | 0x00C0EAFF |

### STM32F4 @ 42 MHz APB1

| Скорость | ClockSpeed |
|----------|------------|
| 100 kHz | 100000 |
| 400 kHz | 400000 |

---

## Troubleshooting

### Дисплей не отвечает

1. Проверьте подключение (VCC, GND, SCL, SDA)
2. Проверьте подтяжки (4.7kΩ к VCC)
3. Проверьте адрес (`scanAddress()`)
4. Проверьте тактирование GPIO и I2C

### HAL_BUSY

```cpp
if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_BUSY) {
    // Попробуйте bus recovery
    oled::OledSsd1315::i2cBusRecovery(GPIOB, GPIO_PIN_6, GPIO_PIN_7);
}
```

### Ничего не отображается

1. Убедитесь что вызван `flush()`
2. Проверьте `isReady()`
3. Попробуйте `invert(true)`

---

## Ссылки

- [API Reference](API.md)
- [Архитектура](ARCHITECTURE.md)
- [Быстрый старт](QUICKSTART.md)
