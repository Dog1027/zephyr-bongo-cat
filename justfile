set allow-duplicate-variables
set allow-duplicate-recipes

import "zephyr.just"

# --- Board-specific settings (edit per project) ---
board := "esp32c3_supermini/esp32c3"
baud  := "921600"
# First connected Espressif USB-Serial-JTAG device (any serial ID). With several
# boards plugged in, replace this with the exact /host/dev/serial/by-id/... path.
# If none is connected, the pattern is left unexpanded and flash/monitor report it.
port  := `p=(/host/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_*-if00); echo "${p[0]}"`

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
