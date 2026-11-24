# Zephyr Bongo Cat

在 ESP32-C3 SuperMini + 128×64 OLED 上跑的 Bongo Cat：按左／右鍵貓咪就拍桌、冒出愛心，
畫面左上角累計拍擊次數。

- 左右鍵可單拍或雙拍，每一拍冒一顆愛心
- 一段時間沒操作會顯示 "Zzz"，接著關閉螢幕並進入 deep sleep
- 按任一鍵喚醒，喚醒的那一下也算一拍
- 拍擊次數存在 flash（NVS），斷電也會保留

以 Zephyr + LVGL 實作，使用 [just](https://just.systems) 管理 build / flash。

## 硬體

| 元件                    | 腳位                      |
| ----------------------- | ------------------------- |
| ESP32-C3 SuperMini      |                           |
| OLED SH1106 (I2C, 0x3C) | SDA = GPIO3, SCL = GPIO4  |
| 左鍵                    | GPIO1（接 GND，內部上拉） |
| 右鍵                    | GPIO2（接 GND，內部上拉） |

I2C 跑在 400 kHz。腳位與螢幕設定見 `boards/esp32c3_supermini.overlay`。

## 前置需求

- 一個 west workspace（預設 `~/Projects/zephyrproject`，含 `.venv`）
  - 其他路徑：設定環境變數 `ZEPHYR_WORKSPACE`
- Zephyr SDK
- `just`、`uv`

## 建置與燒錄

先把 `justfile` 裡的 `port` 改成自己板子的序列埠（`/dev/serial/by-id/...`）。

```sh
just build        # 建置（輸出到 ./build）
just rebuild      # pristine 重建
just config       # menuconfig
just flash        # 燒錄
just monitor      # 序列埠監看
just upload       # build + flash
just run          # build + flash + monitor
just clean
```

額外參數可直接傳給 west build，例如 `just build -- -DCONFIG_APP_HEART_STYLE=2`。

## 設定（Kconfig）

| 選項                               | 預設  | 說明                                   |
| ---------------------------------- | ----- | -------------------------------------- |
| `CONFIG_APP_INACTIVITY_SLEEP_MS`   | 30000 | 無操作多久後 power off（ms）           |
| `CONFIG_APP_HEART_STYLE`           | 0     | 愛心樣式 0–3（只編入選到的那組）       |
| `CONFIG_APP_HEARTBEAT_INTERVAL_MS` | 1000  | debug heartbeat log 間隔（ms）         |

可以寫在 `prj.conf`，或用 `just config` 調整。

## 程式結構

```
.
├── CMakeLists.txt
├── Kconfig                         # App 設定（CONFIG_APP_*）
├── prj.conf                        # Kconfig 設定
├── pm.conf / pm.overlay            # 實驗性 light sleep 設定
├── boards/
│   └── esp32c3_supermini.overlay   # 螢幕、I2C、按鍵
├── include/
├── src/
│   ├── main.c      # 主迴圈：LVGL、閒置計時、power off
│   ├── button.c    # Input callback → message queue、喚醒按鍵補拍
│   ├── cat.c       # 貓咪狀態機（idle / prep / 左拍 / 右拍 / 雙拍）
│   ├── heart.c     # 愛心動畫與計數
│   ├── storage.c   # 拍擊次數存取（NVS，storage_partition）
│   └── bitmap.c    # 所有 A1 格式圖檔
├── zephyr.just     # 通用 recipes（不含板子資訊）
└── justfile        # 本專案的板子 / port / flash 參數
```

> 注意：只要 `boards/<board>.overlay` 存在，Zephyr 就**不會**讀 `app.overlay`。

## 實驗性：Light sleep

```sh
just build-pm     # 輸出到 ./build-pm，不影響一般的 build
just flash-pm
```

設定在 `pm.conf` / `pm.overlay`。目前 ESP32-C3 的 light sleep 只能由 RTC timer 喚醒，
按鍵要等下一次 timer 醒來才會處理；USB Serial/JTAG console 在睡眠時可能斷線。
