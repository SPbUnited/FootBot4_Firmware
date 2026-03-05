"""
PlatformIO Build Script для OLED SSD1315

Автоматически определяет платформу и исключает неиспользуемые адаптеры.
"""

Import("env")

# Получаем framework из окружения
framework = env.get("PIOFRAMEWORK", [])

# Определяем платформу на основе framework
if "arduino" in framework:
    # Arduino framework - исключаем STM32 HAL адаптер
    env.Append(CPPDEFINES=[("OLED_USE_ARDUINO", 1)])
    print("OLED SSD1315: Using Arduino Wire adapter")

elif "stm32cube" in framework:
    # STM32Cube framework (HAL) - исключаем Arduino адаптер
    env.Append(CPPDEFINES=[
        ("OLED_PLATFORM_STM32HAL", 1),
        ("OLED_USE_STM32HAL", 1)
    ])
    # Исключаем WireI2cAdapter из компиляции
    env.Replace(SRC_FILTER=["+<*>", "-<transport/WireI2cAdapter.cpp>"])
    print("OLED SSD1315: Using STM32 HAL I2C adapter")

elif "espidf" in framework:
    # ESP-IDF framework (будущее)
    env.Append(CPPDEFINES=[
        ("OLED_PLATFORM_ESPIDF", 1),
        ("OLED_USE_ESPIDF", 1)
    ])
    print("OLED SSD1315: Using ESP-IDF I2C adapter (future)")

else:
    print("OLED SSD1315: Unknown framework, platform auto-detection in headers")
