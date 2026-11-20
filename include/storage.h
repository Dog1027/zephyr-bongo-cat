#pragma once

#include <stdint.h>

/* Mount the NVS storage. Returns 0 on success. */
int storage_init(void);
/* Load the saved tap count. Returns 0 on success, negative if none saved. */
int storage_load_count(uint32_t *count);
/* Save the tap count. Returns 0 on success. */
int storage_save_count(uint32_t count);
