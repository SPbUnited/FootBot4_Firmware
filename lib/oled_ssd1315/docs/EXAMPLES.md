# Примеры использования — OLED SSD1315

Практические примеры для различных задач.

---

## Содержание

- [Базовый пример](#базовый-пример)
- [Кириллица и UTF-8](#кириллица-и-utf-8)
- [Графические примитивы](#графические-примитивы)
- [Анимация](#анимация)
- [Отображение датчиков](#отображение-датчиков)
- [Меню](#простое-меню)
- [Экран загрузки](#экран-загрузки)

---

## Базовый пример

```cpp
#include <Wire.h>
#include <oled/OledSsd1315.hpp>

OledSsd1315 display(Wire);

void setup() {
    Wire.begin();
    
    OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;
    
    display.begin(cfg);
    display.clear();
    display.print("Hello World!");
    display.flush();
}

void loop() {}
```

---

## Кириллица и UTF-8

Библиотека полностью поддерживает русский язык.

```cpp
void setup() {
    Wire.begin();
    
    OledConfig cfg;
    display.begin(cfg);
    
    display.clear();
    
    // Русский текст
    display.setCursor(0, 0);
    display.print("Привет мир!");
    
    // Смешанный текст
    display.setCursor(0, 10);
    display.print("Hello Мир!");
    
    // Кириллица в printf
    display.setCursor(0, 20);
    display.printf("Темп: %d°C", 25);
    
    // Буквы Ё и ё
    display.setCursor(0, 30);
    display.print("Ёлка, ёжик");
    
    display.flush();
}
```

**Поддерживаемые символы:**
- ASCII: 32–126 (латиница, цифры, знаки)
- Кириллица: А–Я, а–я, Ё, ё

---

## Графические примитивы

### Точки и линии

```cpp
void drawPrimitives() {
    display.clear();
    
    // Точки
    for (int i = 0; i < 128; i += 4) {
        display.pixel(i, 0, true);
    }
    
    // Горизонтальная линия
    display.line(0, 10, 127, 10, true);
    
    // Вертикальная линия
    display.line(64, 10, 64, 63, true);
    
    // Диагональ
    display.line(0, 20, 127, 63, true);
    
    display.flush();
}
```

### Прямоугольники

```cpp
void drawRects() {
    display.clear();
    
    // Контуры
    display.rect(0, 0, 40, 30, true);
    display.rect(5, 5, 30, 20, true);
    
    // Залитые
    display.rectFill(50, 0, 20, 20, true);
    display.rectFill(80, 0, 40, 30, true);
    
    // Инвертированный текст внутри
    display.setCursor(82, 10);
    display.setTextColor(false);  // Чёрный текст
    display.print("OK");
    display.setTextColor(true);   // Вернуть белый
    
    display.flush();
}
```

### Рамка экрана

```cpp
void drawBorder() {
    display.clear();
    
    // Рамка по периметру
    display.rect(0, 0, 128, 64, true);
    
    // Заголовок
    display.rectFill(0, 0, 128, 12, true);
    display.setCursor(2, 2);
    display.setTextColor(false);
    display.print("MENU");
    display.setTextColor(true);
    
    // Контент
    display.setCursor(4, 16);
    display.print("Item 1");
    display.setCursor(4, 26);
    display.print("Item 2");
    
    display.flush();
}
```

---

## Анимация

### Бегущая строка

```cpp
void marquee() {
    const char* text = "  OLED SSD1315 Library  ";
    int textWidth = strlen(text) * 6;  // 5px + 1px gap
    int offset = 0;
    
    while (true) {
        display.clear();
        display.setCursor(-offset, 28);
        display.print(text);
        display.flush();
        
        offset++;
        if (offset > textWidth) offset = 0;
        
        delay(50);
    }
}
```

### Мигающий курсор

```cpp
void blinkingCursor() {
    bool visible = true;
    
    display.clear();
    display.print("Input: ");
    display.flush();
    
    int cursorX = 42;  // После "Input: "
    
    while (true) {
        // Курсор
        display.rectFill(cursorX, 0, 6, 8, visible);
        display.flush();
        
        visible = !visible;
        delay(500);
    }
}
```

### Простой прогресс-бар

```cpp
void progressBar(int percent) {
    display.clear();
    
    // Рамка прогресс-бара
    display.rect(10, 25, 108, 14, true);
    
    // Заполнение
    int fillWidth = (percent * 104) / 100;
    display.rectFill(12, 27, fillWidth, 10, true);
    
    // Процент
    display.setCursor(50, 45);
    display.printf("%d%%", percent);
    
    display.flush();
}

void loop() {
    for (int i = 0; i <= 100; i++) {
        progressBar(i);
        delay(50);
    }
}
```

---

## Отображение датчиков

### Температура и влажность

```cpp
void showSensorData(float temp, float humidity) {
    display.clear();
    
    // Заголовок
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Датчики:");
    
    // Температура (крупно)
    display.setCursor(0, 16);
    display.setTextSize(2);
    display.printf("%.1f", temp);
    display.setTextSize(1);
    display.print(" C");
    
    // Влажность
    display.setCursor(0, 40);
    display.setTextSize(2);
    display.printf("%.0f", humidity);
    display.setTextSize(1);
    display.print(" %");
    
    // Иконки (простые)
    display.setCursor(100, 16);
    display.print("T");
    display.setCursor(100, 40);
    display.print("H");
    
    display.flush();
}
```

### График значений

```cpp
#define GRAPH_POINTS 100
int graphData[GRAPH_POINTS] = {0};
int graphIndex = 0;

void addGraphPoint(int value) {
    graphData[graphIndex] = value;
    graphIndex = (graphIndex + 1) % GRAPH_POINTS;
}

void drawGraph() {
    display.clear();
    
    // Оси
    display.line(10, 0, 10, 50, true);   // Y
    display.line(10, 50, 127, 50, true); // X
    
    // Данные
    for (int i = 0; i < GRAPH_POINTS - 1; i++) {
        int idx1 = (graphIndex + i) % GRAPH_POINTS;
        int idx2 = (graphIndex + i + 1) % GRAPH_POINTS;
        
        int y1 = 50 - (graphData[idx1] * 45 / 100);
        int y2 = 50 - (graphData[idx2] * 45 / 100);
        
        display.line(12 + i, y1, 13 + i, y2, true);
    }
    
    display.flush();
}
```

---

## Простое меню

```cpp
const char* menuItems[] = {
    "Настройки",
    "Яркость",
    "Контраст", 
    "Сброс",
    "Выход"
};
const int menuCount = 5;
int menuSelected = 0;

void drawMenu() {
    display.clear();
    
    for (int i = 0; i < menuCount; i++) {
        int y = i * 12;
        
        if (i == menuSelected) {
            // Выделенный пункт
            display.rectFill(0, y, 128, 11, true);
            display.setCursor(4, y + 2);
            display.setTextColor(false);
            display.print(menuItems[i]);
            display.setTextColor(true);
        } else {
            display.setCursor(4, y + 2);
            display.print(menuItems[i]);
        }
    }
    
    display.flush();
}

void menuUp() {
    if (menuSelected > 0) menuSelected--;
    drawMenu();
}

void menuDown() {
    if (menuSelected < menuCount - 1) menuSelected++;
    drawMenu();
}
```

---

## Экран загрузки

```cpp
void splashScreen() {
    display.clear();
    
    // Логотип (простой)
    display.rectFill(44, 10, 40, 30, true);
    display.setCursor(52, 18);
    display.setTextColor(false);
    display.setTextSize(2);
    display.print("SK");
    display.setTextColor(true);
    display.setTextSize(1);
    
    // Название
    display.setCursor(20, 48);
    display.print("OLED SSD1315 v1.1");
    
    display.flush();
    delay(2000);
}
```

---

## Управление питанием

### Режим сна

```cpp
void enterSleep() {
    display.setPower(false);  // Выключить дисплей
    // Дисплей потребляет ~10µA в режиме сна
}

void wakeUp() {
    display.setPower(true);   // Включить дисплей
    display.flush();          // Восстановить изображение
}
```

### Автояркость

```cpp
void adjustBrightness(int lightLevel) {
    // lightLevel: 0-1023 (от датчика освещённости)
    uint8_t contrast = map(lightLevel, 0, 1023, 0, 255);
    display.setContrast(contrast);
}
```

---

## См. также

- [API Reference](API.md)
- [Архитектура](ARCHITECTURE.md)
