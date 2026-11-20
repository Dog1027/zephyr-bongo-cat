# Zephyr Bongo Cat

在 ESP32-C3 SuperMini + 128×64 OLED 上跑的 Bongo Cat：按左／右鍵貓咪就拍桌、冒出愛心，
畫面左上角累計拍擊次數。一段時間沒操作會關閉螢幕並 power off，按任一鍵喚醒。

以 Zephyr + LVGL 實作，使用 [just](https://just.systems) 管理 build / flash。

## 硬體接線

| 元件             | 腳位                       |
| ---------------- | -------------------------- |
| OLED SH1106 (I2C, 0x3C) | SDA = GPIO3, SCL = GPIO4 |
| 左鍵             | GPIO1（接 GND，內部上拉）  |
| 右鍵             | GPIO2（接 GND，內部上拉）  |

設定見 `boards/esp32c3_supermini.overlay`。

## 程式結構

| 檔案           | 說明                                              |
| -------------- | ------------------------------------------------- |
| `src/main.c`   | 主迴圈：LVGL、閒置計時與 power off               |
| `src/button.c` | Input callback → message queue                   |
| `src/cat.c`    | 貓咪狀態機（idle / prep / 左拍 / 右拍 / 雙拍）   |
| `src/heart.c`  | 愛心動畫與計數                                    |
| `src/bitmap.c` | 所有 I1 格式圖檔                                  |

## App 設定（Kconfig）

| 選項                              | 預設 | 說明                          |
| --------------------------------- | ---- | ----------------------------- |
| `CONFIG_APP_INACTIVITY_SLEEP_MS`  | 5000 | 無操作多久後 power off（ms） |
| `CONFIG_APP_HEARTBEAT_INTERVAL_MS`| 1000 | debug heartbeat log 間隔（ms）|

---

以下為原模板說明。

## 前置需求

- 一個 west workspace（預設 `~/Projects/zephyrproject`，含 `.venv`）
  - 其他路徑：設定環境變數 `ZEPHYR_WORKSPACE`
- Zephyr SDK
- `just`、`uv`

## 目錄結構

```
.
├── CMakeLists.txt         # 新增原始碼加到 target_sources
├── Kconfig                # App 自訂 Kconfig 選項（CONFIG_APP_*）
├── prj.conf               # 通用 Kconfig 設定
├── VERSION                # App 版本，產生 <zephyr/app_version.h>
├── boards/                # 板子專屬設定，Zephyr 自動套用
│   ├── <board>.overlay    #   例：esp32_devkitc_procpu.overlay
│   └── <board>.conf       #   例：esp32_devkitc_procpu.conf
├── include/               # App 標頭檔
├── src/main.c
├── zephyr.just            # 通用 recipes（不含板子資訊）
└── justfile               # 本專案的板子 / port / flash 參數
```

> 注意：只要 `boards/<board>.overlay` 存在，Zephyr 就**不會**讀 `app.overlay`。

## 使用

```sh
just build        # 建置（輸出到 ./build）
just rebuild      # pristine 重建
just config       # menuconfig
just flash        # 燒錄
just monitor      # 序列埠監看
just run          # build + flash + monitor
just clean
```

額外參數可直接傳給 west build，例如 `just build -- -DEXTRA_CONF_FILE=debug.conf`。

## 從模板開新專案

1. 複製這個目錄
2. 修改 `justfile`：`board`、`port`、`baud`、`flash_args`、`monitor`
3. 在 `boards/` 放對應板子的 `.overlay` / `.conf`
   （檔名＝board target 把 `/` 換成 `_`，SoC 名稱若與板名重複可省略，
   如 `esp32_devkitc/esp32/procpu` → `esp32_devkitc_procpu`）
4. 修改 `VERSION`
