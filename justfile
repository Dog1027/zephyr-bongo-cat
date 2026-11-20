set allow-duplicate-variables
set allow-duplicate-recipes

import "zephyr.just"

# --- Board-specific settings (edit per project) ---
board := "esp32c3_supermini/esp32c3"
baud  := "921600"
port  := "/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:9E:6E:C2:FD:58-if00"

flash_args := "--esp-device " + port + " --esp-baud-rate " + baud

# Open serial monitor
monitor:
    cd {{build_dir}} && ZEPHYR_BASE={{workspace}}/zephyr uv run --no-project --python {{workspace}}/.venv west espressif monitor -p {{port}}
