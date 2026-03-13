run:
	.pio/build/native/program

build: pio_run
	g++ -g -o .pio/build/native/program .pio/build/native/lib117/letter-shell/extensions/log/log.o .pio/build/native/lib117/letter-shell/extensions/shell_enhance/shell_cmd_group.o .pio/build/native/lib117/letter-shell/shell.o .pio/build/native/lib117/letter-shell/shell_cmd_list.o .pio/build/native/lib117/letter-shell/shell_companion.o .pio/build/native/lib117/letter-shell/shell_ext.o .pio/build/native/src/devices/bldc/bldc.o .pio/build/native/src/devices/chassis/chassis.o .pio/build/native/src/devices/chassis/chassis.shell.o .pio/build/native/src/devices/device_manager.o .pio/build/native/src/devices/logger/logger.o .pio/build/native/src/devices/oled/oled.o .pio/build/native/src/devices/oled/oled.shell.o .pio/build/native/src/devices/shell/shell.o .pio/build/native/src/drivers/can/can.o .pio/build/native/src/drivers/driver_manager.o .pio/build/native/src/drivers/i2c/i2c.o .pio/build/native/src/drivers/led/led.o .pio/build/native/src/drivers/system_clock/system_clock.o .pio/build/native/src/kernel/kernel.o .pio/build/native/src/main.o -L.pio/build/native -Wl,--start-group -T /home/arsenier/Documents/ssl/FootBot4_Firmware/x86.lds -Wl,--end-group

pio_run:
	pio run -e native || true
