if [ "$#" -ne 1 ]; then
    echo "Usage: $0 device"
    echo "e.g: $0 /dev/ttyUSB0"
    exit 1
fi

~/.arduino15/packages/esp8266/tools/mkspiffs/3.1.0-gcc10.3-e5f9fec/mkspiffs \
-c ./data \
  --size 2072576 \
  --page 256 \
  --block 8192 -- \
  spiffs.bin

~/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/tools/esptool/esptool.py \
  --chip esp8266 \
  --port $1 \
  --baud 460800 \
  --before default_reset \
  --after hard_reset \
  write_flash 0x200000 spiffs.bin
