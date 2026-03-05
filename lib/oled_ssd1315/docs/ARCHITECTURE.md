# Архитектура библиотеки OLED SSD1315 v3.0

## Обзор

Библиотека построена по принципам **Clean Architecture** с чётким разделением на слои.

```text
┌─────────────────────────────────────────────────┐
│              OledSsd1315 (Facade)               │  ← Публичный API
│                   + pImpl                       │
├─────────────────────────────────────────────────┤
│                    domain/                      │  ← Чистая бизнес-логика
│              Gfx, Ssd1315Driver                 │
├─────────────────────────────────────────────────┤
│                    ports/                       │  ← Интерфейсы (абстракции)
│                     II2c                        │
├─────────────────────────────────────────────────┤
│                   adapters/                     │  ← Реализации платформ
│       WireI2cAdapter, Stm32HalI2cAdapter        │
└─────────────────────────────────────────────────┘
```

---

## Принципы архитектуры

### 1. Dependency Inversion Principle (DIP)

- **Domain** зависит от **Ports** (интерфейсов)
- **Adapters** реализуют **Ports**
- Domain не знает о конкретных платформах

### 2. pImpl Idiom

HAL-типы (`I2C_HandleTypeDef`, `TwoWire`) скрыты от публичного API:

```cpp
// OledSsd1315.hpp (публичный)
class OledSsd1315 {
private:
    detail::OledSsd1315Impl* pImpl_;  // Скрытая реализация
};

// OledSsd1315Impl.hpp (внутренний)
struct OledSsd1315Impl {
    I2C_HandleTypeDef* hi2c;  // HAL-тип только здесь
    Stm32HalI2cAdapter adapter;
    Ssd1315Driver driver;
    Gfx gfx;
    // ...
};
```

### 3. Module Boundaries

Каждый слой — "чёрный ящик" с минимальным публичным API.

---

## Структура файлов

```text
lib/oled_ssd1315/
├── include/oled/
│   ├── OledSsd1315.hpp         # Публичный API (Facade)
│   ├── OledSsd1315Impl.hpp     # pImpl реализация (internal)
│   ├── OledSsd1315Fwd.hpp      # Forward declarations
│   ├── OledConfig.hpp          # Конфигурация, макросы
│   ├── OledTypes.hpp           # Типы: OledResult, VccMode
│   │
│   ├── ports/                  # ИНТЕРФЕЙСЫ (абстракции)
│   │   └── II2c.hpp            # Абстракция I2C транспорта
│   │
│   ├── adapters/               # АДАПТЕРЫ (платформенные реализации)
│   │   ├── WireI2cAdapter.hpp  # Arduino Wire
│   │   ├── Stm32HalI2cAdapter.hpp  # STM32 HAL
│   │   └── PlatformDelay.hpp   # Кросс-платформенные задержки
│   │
│   └── domain/                 # DOMAIN (чистая логика)
│       ├── Gfx.hpp             # Графика, примитивы, текст
│       ├── Ssd1315Driver.hpp   # Драйвер контроллера
│       └── Ssd1315Commands.hpp # Константы команд
│
├── src/
│   ├── OledSsd1315.cpp         # Реализация Facade
│   ├── driver/Ssd1315Driver.cpp
│   ├── gfx/Gfx.cpp
│   └── transport/WireI2cAdapter.cpp
│
├── tests/                      # UNIT-ТЕСТЫ
│   ├── CMakeLists.txt          # Сборка тестов
│   ├── mocks/MockI2c.hpp       # Mock I2C адаптер
│   ├── test_gfx.cpp            # Тесты графики
│   └── test_driver.cpp         # Тесты драйвера
│
├── examples/
│   └── stm32h743_test/         # Пример для STM32H743
│
├── .clang-format               # Автоформатирование
├── .clang-tidy                 # Статический анализ
└── library.json
```

---

## Слои архитектуры

### 1. Facade — `OledSsd1315`

**Файл:** `include/oled/OledSsd1315.hpp`

Единая точка входа. Скрывает внутреннюю сложность через pImpl.

```cpp
namespace oled {
class OledSsd1315 {
public:
    explicit OledSsd1315(I2C_HandleTypeDef* hi2c);  // STM32
    explicit OledSsd1315(TwoWire& wire);            // Arduino
    
    OledResult begin(const OledConfig& cfg);
    void clear();
    void flush();
    // ...
    
private:
    detail::OledSsd1315Impl* pImpl_;  // Скрытая реализация
};
}
```

### 2. Domain Layer

**Файлы:** `include/oled/domain/`

Чистая бизнес-логика, не зависит от платформы.

#### Gfx (Графика)

```cpp
class Gfx {
    void init(uint8_t* buffer, uint16_t w, uint16_t h);
    void clear();
    void pixel(int x, int y, bool color);
    void line(int x0, int y0, int x1, int y1, bool color);
    void print(const char* str);  // UTF-8
};
```

#### Ssd1315Driver

```cpp
class Ssd1315Driver {
    OledResult init(II2c& transport, const OledConfig& cfg);
    OledResult writeBuffer(const uint8_t* data, size_t len);
    OledResult setPower(bool on);
};
```

### 3. Ports Layer

**Файлы:** `include/oled/ports/`

Интерфейсы (абстракции) для инверсии зависимостей.

```cpp
struct II2c {
    virtual bool write(uint8_t addr7, const uint8_t* data, size_t len) = 0;
    virtual bool probe(uint8_t addr7) = 0;
    virtual ~II2c() = default;
};
```

### 4. Adapters Layer

**Файлы:** `include/oled/adapters/`

Платформенные реализации интерфейсов.

#### WireI2cAdapter (Arduino)

```cpp
class WireI2cAdapter : public II2c {
    void init(TwoWire& wire);
    bool write(uint8_t addr7, const uint8_t* data, size_t len) override;
};
```

#### Stm32HalI2cAdapter (STM32 HAL)

```cpp
class Stm32HalI2cAdapter : public II2c {
    void init(I2C_HandleTypeDef* hi2c);
    bool write(uint8_t addr7, const uint8_t* data, size_t len) override;
};
```

---

## Поток данных

### Инициализация

```text
User                OledSsd1315              pImpl              Driver
  │                      │                     │                   │
  │ begin(cfg)           │                     │                   │
  │─────────────────────>│                     │                   │
  │                      │ pImpl_->adapter.init()                  │
  │                      │────────────────────>│                   │
  │                      │                     │                   │
  │                      │ pImpl_->driver.init()                   │
  │                      │────────────────────────────────────────>│
  │                      │                     │                   │
  │<─────────────────────│                     │                   │
  │ OledResult::Ok       │                     │                   │
```

### Отрисовка

```text
User                OledSsd1315              Gfx              Driver
  │                      │                    │                  │
  │ clear()              │                    │                  │
  │─────────────────────>│ pImpl_->gfx.clear()│                  │
  │                      │───────────────────>│                  │
  │                      │                    │                  │
  │ print("Привет")      │                    │                  │
  │─────────────────────>│ pImpl_->gfx.print()│                  │
  │                      │───────────────────>│ UTF-8 decode     │
  │                      │                    │                  │
  │ flush()              │                    │                  │
  │─────────────────────>│ pImpl_->driver.writeBuffer()          │
  │                      │───────────────────────────────────────>│
```

---

## Тестирование

### MockI2c

Для unit-тестов используется mock-реализация:

```cpp
class MockI2c : public II2c {
    std::vector<Transaction> transactions_;
    
    bool write(uint8_t addr7, const uint8_t* data, size_t len) override {
        transactions_.push_back({addr7, {data, data + len}});
        return !shouldFail_;
    }
    
    void setFail(bool fail) { shouldFail_ = fail; }
    size_t transactionCount() const { return transactions_.size(); }
};
```

### Запуск тестов

```bash
cd tests
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest --output-on-failure
```

---

## Code Quality

### .clang-format

Google-based стиль с адаптациями:
- 4 пробела отступ
- 100 символов ширина
- Сортировка includes

### .clang-tidy

Включённые проверки:
- `bugprone-*` — потенциальные баги
- `modernize-*` — современный C++
- `performance-*` — оптимизации
- `readability-*` — читаемость

---

## Ссылки

- [API Reference](API.md)
- [STM32 HAL Guide](STM32_HAL.md)
- [Команды SSD1315](SSD1315_COMMANDS.md)
