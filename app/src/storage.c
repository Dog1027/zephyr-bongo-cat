#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/kvss/nvs.h>
#include "storage.h"

LOG_MODULE_REGISTER(storage, LOG_LEVEL_INF);

#define NVS_PARTITION        storage_partition
#define NVS_SECTOR_COUNT     3
#define NVS_ID_HEART_COUNT   1

static struct nvs_fs fs;
static bool mounted;

int storage_init(void)
{
    struct flash_pages_info info;
    int ret;

    fs.flash_device = PARTITION_DEVICE(NVS_PARTITION);
    if (!device_is_ready(fs.flash_device)) {
        LOG_ERR("Flash device not ready");
        return -ENODEV;
    }

    fs.offset = PARTITION_OFFSET(NVS_PARTITION);
    ret = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (ret < 0) {
        LOG_ERR("Failed to get flash page info (%d)", ret);
        return ret;
    }
    fs.sector_size = info.size;
    fs.sector_count = NVS_SECTOR_COUNT;

    ret = nvs_mount(&fs);
    if (ret < 0) {
        LOG_ERR("Failed to mount NVS (%d)", ret);
        return ret;
    }

    mounted = true;
    return 0;
}

int storage_load_count(uint32_t *count)
{
    if (!mounted) {
        return -ENODEV;
    }

    ssize_t ret = nvs_read(&fs, NVS_ID_HEART_COUNT, count, sizeof(*count));
    if (ret != sizeof(*count)) {
        return ret < 0 ? (int)ret : -EIO;
    }
    return 0;
}

int storage_save_count(uint32_t count)
{
    if (!mounted) {
        return -ENODEV;
    }

    /* nvs_write() skips the flash write if the value is unchanged. */
    ssize_t ret = nvs_write(&fs, NVS_ID_HEART_COUNT, &count, sizeof(count));
    return ret < 0 ? (int)ret : 0;
}
