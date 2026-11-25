set allow-duplicate-variables
set allow-duplicate-recipes

import "zephyr.just"

# --- Board-specific settings (edit per project) ---
board := "esp32c3_supermini/esp32c3"
baud  := "921600"
port  := "/host/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_9C:9E:6E:C2:FD:58-if00"

flash_args := "--esp-device " + port + " --esp-baud-rate " + baud

# Open serial monitor
monitor:
    cd {{build_dir}} && {{west}} espressif monitor -p {{port}}

# Experimental light sleep build (see pm.conf), in its own build dir so the
# normal `just build` is unaffected. Flash it with `just flash-pm`.
build-pm *args:
    just build_dir={{topdir}}/build-pm build -- -DEXTRA_CONF_FILE={{app_dir}}/pm.conf -DEXTRA_DTC_OVERLAY_FILE={{app_dir}}/pm.overlay {{args}}

flash-pm:
    just build_dir={{topdir}}/build-pm flash
