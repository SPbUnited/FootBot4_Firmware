build:
	mkdir -p build
	cd build && cmake ..
	cd build && make
	cp build/release/fb4_firmware fb4_controller

clean:
	rm -rf build || true
	rm fb4_firmware || true
