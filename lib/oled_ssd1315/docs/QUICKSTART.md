# Быстрый старт OLED SSD1315 v3.0

## Установка

### PlatformIO (рекомендуется)

```ini
[env:myboard]
lib_deps = 
    oled_ssd1315

build_flags = 
    -DOLED_SSD1315_ENABLE=1
```

### Для STM32 HAL

```ini
[env:stm32]
platform = ststm32
framework = stm32cube
lib_deps = 
    oled_ssd1315

build_flags = 
    -DOLED_SSD1315_ENABLE=1
    -DOLED_PLATFORM_STM32HAL=1
```

---

## Arduino

```cpp
#include <Wire.h>
#include <oled/OledSsd1315.hpp>

oled::OledSsd1315* display = nullptr;

void setup() {
    Wire.begin();
    
    // Создаём объект дисплея
    display = new oled::OledSsd1315(Wire);
    
    // Конфигурация
    oled::OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;  // или 0x3D
    cfg.width = 128;
    cfg.height = 64;      // или 32
    cfg.vccMode = oled::VccMode::InternalChargePump;
    
    // Инициализация
    if (display->begin(cfg) != oled::OledResult::Ok) {
        Serial.println("OLED init failed!");
        return;
    }
    
    // Вывод текста
    display->clear();
    display->setCursor(0, 0);
    display->print("Hello World!");
    display->flush();
}

void loop() {
    // Обновление экрана
    static uint32_t counter = 0;
    
    display->setCursor(0, 20);
    display->printf("Count: %lu", counter++);
    display->flush();
    
    delay(100);
}
```

---

## STM32 HAL

```cpp
#include "stm32h7xx_hal.h"
#include <oled/OledSsd1315.hpp>

// I2C handle (из CubeMX или ручной инициализации)
I2C_HandleTypeDef hi2c1;

// Дисплей (указатель)
oled::OledSsd1315* display = nullptr;

int main() {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    
    HAL_Delay(100);  // Стабилизация питания
    
    // Создаём объект
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
    display->print("Привет STM32!");
    display->flush();
    
    while (1) {
        HAL_Delay(1000);
    }
}
```

---

## Графические примитивы

```cpp
// Очистка
display->clear();

// Пиксель
display->pixel(64, 32, true);

// Линия (алгоритм Брезенхэма)
display->line(0, 0, 127, 63, true);

// Прямоугольник (контур)
display->rect(10, 10, 50, 30, true);

// Залитый прямоугольник
display->rectFill(70, 10, 50, 30, true);

// Отправить на дисплей
display->flush();
```

---

## Текст

```cpp
// Позиция курсора
display->setCursor(0, 0);

// Масштаб (1, 2, 3...)
display->setTextSize(2);

// Цвет (true = белый, false = чёрный)
display->setTextColor(true);

// Вывод строки (UTF-8, кириллица поддерживается)
display->print("Привет мир!");

// Форматированный вывод
int temp = 25;
display->printf("Темп: %d°C", temp);
```

---

## Управление дисплеем

```cpp
// Вкл/выкл питание
display->setPower(false);  // Выключить
display->setPower(true);   // Включить

// Контраст (0-255)
display->setContrast(128);

// Инверсия цветов
display->invert(true);
display->invert(false);
```

---

## Диагностика

```cpp
// Проверка готовности
if (!display->isReady()) {
    // Дисплей не инициализирован
}

// Последний результат
oled::OledResult result = display->getLastResult();

// Текст ошибки
const char* error = display->getLastError();

// Сканирование I2C адреса
uint8_t addr = display->scanAddress(0x3C, 0x3D);
if (addr != 0) {
    cfg.i2cAddr7 = addr;
}
```

---

## STM32: DMA передача

```cpp
// Non-blocking передача буфера
oled::OledResult res = display->flushDMA();

// Проверка завершения
while (!display->isDMAComplete()) {
    // Другие задачи
}

// Callback в stm32xx_it.c
extern oled::OledSsd1315* display;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    display->onDmaComplete();
}
```

---

## Подключение

### I2C адреса

| 7-bit | 8-bit (write) | Примечание |
|-------|---------------|------------|
| 0x3C  | 0x78          | По умолчанию |
| 0x3D  | 0x7A          | Альтернативный |

### Схема подключения

```
OLED       MCU
----       ---
VCC   -->  3.3V
GND   -->  GND
SCL   -->  I2C Clock
SDA   -->  I2C Data
```

### Пины для STM32

| Плата | I2C1 SCL | I2C1 SDA |
|-------|----------|----------|
| STM32F4 | PB6 | PB7 |
| STM32H7 | PB6 | PB7 |
| Nucleo | PB8 | PB9 |

---

## Ссылки

- [Полный API](API.md)
- [Архитектура](ARCHITECTURE.md)
- [STM32 HAL подробно](STM32_HAL.md)
