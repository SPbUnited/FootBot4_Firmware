/**
 * @file main.cpp
 * @brief Пример использования библиотеки OLED SSD1315
 * 
 * Демонстрирует:
 * - Инициализацию дисплея
 * - Вывод текста
 * - Рисование примитивов
 * - Управление питанием и контрастом
 */

#include <Arduino.h>
#include <Wire.h>
#include <oled/OledSsd1315.hpp>

// Создаём объект OLED с привязкой к Wire
OledSsd1315 display(Wire);

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("OLED SSD1315 Example");
    
    // Инициализация I2C
    // Для ESP32: Wire.begin(SDA, SCL)
    // Для стандартных пинов:
    Wire.begin();
    
    // Конфигурация дисплея
    OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;  // 7-битный адрес (0x3C или 0x3D)
    cfg.width = 128;
    cfg.height = 64;      // Или 32 для 128x32 дисплеев
    cfg.vccMode = VccMode::InternalChargePump;  // Для большинства модулей
    cfg.flip180 = false;  // true если изображение вверх ногами
    // cfg.resetCallback = nullptr;  // по умолчанию, можно не указывать
    
    // Инициализация
    OledResult result = display.begin(cfg);
    
    if (result != OledResult::Ok) {
        Serial.print("OLED init failed: ");
        switch (result) {
            case OledResult::Disabled:
                Serial.println("Library disabled (OLED_SSD1315_ENABLE not set)");
                break;
            case OledResult::I2cError:
                Serial.println("I2C error - check wiring and address");
                break;
            case OledResult::InvalidArg:
                Serial.println("Invalid configuration");
                break;
            default:
                Serial.println("Unknown error");
        }
        return;
    }
    
    Serial.println("OLED initialized successfully!");
    
    // === Демонстрация возможностей ===
    
    // Очистка экрана
    display.clear();
    
    // Вывод текста
    display.setCursor(0, 0);
    display.setTextSize(1);  // Масштаб 1 = 5x7 пикселей
    display.print("SSD1315 Library");
    
    display.setCursor(0, 10);
    display.print("128x64 OLED");
    
    // Текст с масштабом 2
    display.setCursor(0, 24);
    display.setTextSize(2);
    display.print("OK!");
    
    // Рисование примитивов
    display.rect(80, 20, 40, 30, true);      // Прямоугольник (контур)
    display.rectFill(85, 25, 10, 10, true);  // Залитый прямоугольник
    display.line(0, 63, 127, 50, true);      // Линия
    
    // Отправка буфера на дисплей
    display.flush();
    
    Serial.println("Display updated!");
}

void loop() {
    static uint32_t lastUpdate = 0;
    static int counter = 0;
    
    // Обновление раз в секунду
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        counter++;
        
        // Очищаем область для счётчика
        display.rectFill(0, 54, 80, 10, false);
        
        // Выводим счётчик
        display.setCursor(0, 54);
        display.setTextSize(1);
        display.printf("Count: %d", counter);
        
        // Обновляем дисплей
        display.flush();
    }
}
