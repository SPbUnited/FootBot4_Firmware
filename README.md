# Прошивка для FootBot 4
Программатор JLink. Чтобы поменять на ST-Link, надо изменить параметр в platformio.ini

## Важные моменты
Так как main - это cpp файл, то для корректной работы прерываний требуется extern "C". https://community.platformio.org/t/hal-delay-infinite-loop-due-to-return-0-from-hal-gettick/45129
## Стек: 
1. Platformio
2. HAL
3. letter-shell + extensions

## Стуктура исходного кода

https://web.archive.org/web/20231210061404/https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs

## Дерево технологий

![](docs/tech_tree.excalidraw.png)

## Заметки

### Компиляция letter-shell

Для компиляции letter-shell потребовались некоторые танцы с бубном. Во первых, для экспорта функций необходимо модифицировать файл линковки `.ld`.

Иначе комипилятор ругается на:

```
.pio/build/disco_f429zi/lib117/libletter-shell.a(shell.o): In function `shellInit':
shell.c:(.text.shellInit+0x7c): undefined reference to `_shell_command_start'
shell.c:(.text.shellInit+0x80): undefined reference to `_shell_command_end'
collect2: error: ld returned 1 exit status
```

Конкретно, интересующий файл лежал по пути `~/.platformio/packages/tool-ldscripts-ststm32/stm32f4/STM32F429ZITX_FLASH.ld`. Его надо скопировать в наш проект:

```bash
cp ~/.platformio/packages/tool-ldscripts-ststm32/stm32f4/STM32F429ZITX_FLASH.ld .
```

И в нем надо добавить строки:

```diff
  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
+   _shell_command_start = .;
+   KEEP (*(shellCommand))
+   _shell_command_end = .;
    . = ALIGN(4);
  } >FLASH
```

После чего в `platformio.ini` надо указать этот кастомный файл:

```diff
...

+board_build.ldscript = STM32F429ZITX_FLASH.ld
```

### Компиляция log

При компиляции библиотек platformio, судя по всему, смотрит только на файлы из папки `src`. Файлы логгера находились в папке `extensions/log`, поэтому функции логгера не компилировались. Чтобы они смогли автоматически компилироваться нужные расширения были перенесены в директорию `src/extensions/`. Так они автоматически подхватываются и компилируются.

### Include path

Чтобы была возможность однокнопочного подключения заголовочных файлов, необходимо их пути добавить в флаги компиляции. Кроме как в `platformio.ini`, это можно сделать и в файле `library.json` в поле `build`, например вот так:

```json
{
    "name": "letter-shell",
    "version": "3.2.4",
    "keywords": "shell, embedded, command line, cli, console",
    "description": "Letter shell for embedded systems",
    "repository": {
        "type": "git",
        "url": "https://github.com/NevermindZZT/letter-shell.git"
    },
    "frameworks": "stm32cube",
    "platforms": "ststm32",
    "build": {
        "libArchive": false,
        "flags": [
            "-Isrc/",
            "-Isrc/extensions/cpp_support",
            "-Isrc/extensions/log"
        ]
    }
}
```

## UART с Raspberry Pi

Код: `src/devices/uart_control`. Парная сторона — `src/protocol/uart.py` в [fb4-runner](https://github.com/sigizmund993/fb4-runner); форматы должны совпадать (размер пакета проверяет `static_assert`).

Кадр в обе стороны: `[0xA5][payload][XOR от 0xA5 и payload]`.

| Направление | Формат (Python struct) | Поля | Кадр |
|---|---|---|---|
| RPi → STM | `<5eB??B` | vel_x, vel_y, w, dribbler, kicker (fp16), kicker_mode, angle_mode, beep, robot_id | 16 байт |
| STM → RPi | `<Be??` | robot_id, напряжение кикера (fp16), ball_deep, ball_front | 7 байт |

Если формат меняется, прошивку, раннер и teleop надо обновлять вместе: кадры другой длины не распознаются, и робот стоит.

Если 200 мс не приходит ни одного корректного кадра, робот останавливается (скорости, дриблер, кикер, писк).

### beep

Пищалка на PF13 пассивная: звучит, только пока пин переключается. Его переключает прерывание TIM7 на 2 кГц (`src/drivers/buzzer`), главный цикл только включает и выключает звук. Пищит, пока приходят кадры с `beep = 1`.

### robot_id

Обе стороны шлют свой id в каждом кадре. Правила, чтобы стороны не перебивали друг друга:

- STM принимает id от RPi сразу и сохраняет в EEPROM.
- Исключение — id только что поменяли кнопками. Тогда STM игнорирует id от RPi, пока RPi не пришлёт этот же новый id (RPi берёт id STM, если он отличается дольше 1 с). После этого STM сохраняет id в EEPROM сама, зажимать SELECT не обязательно.
- `robot_id = 0xFF` (и всё больше 15) — у отправителя нет id, STM оставляет свой. Так шлют teleop в режиме UART и аппаратные тесты.
- Перед записью в EEPROM робот получает стоп: стирание сектора flash останавливает главный цикл на 1–2 с.

Ограничения:

- Если поменять id кнопками и сохранить с перезагрузкой, пока раннер на RPi не запущен, после подключения победит id RPi: отметка «поменяли кнопками» живёт только в RAM.
- Звёздочка `*` на экране («id изменён») после автосохранения не гаснет до перезагрузки.
