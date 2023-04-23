all: compile upload

compile:
	arduino-cli compile -b arduino:avr:nano --libraries ./libraries --output-dir ./build

upload:
	arduino-cli upload -b arduino:avr:nano:cpu=atmega328old -p /dev/ttyUSB0 --input-dir ./build -v

clean:
	rm -rf ./build
