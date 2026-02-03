# Прошивка для FootBot 4
Программатор JLink. Чтобы поменять на ST-Link, надо изменить параметр в platformio.ini

## Важные моменты
Так как main - это cpp файл, то для корректной работы прерываний требуется extern "C". https://community.platformio.org/t/hal-delay-infinite-loop-due-to-return-0-from-hal-gettick/45129
## Стек: 
1. Platformio
2. HAL