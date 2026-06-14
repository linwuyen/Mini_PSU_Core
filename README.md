# Mini_PSU_Core

小型可程式電源控制韌體 prototype 的公開作品集 repository。

此 repo 用於記錄小型 programmable power supply controller 的架構與開發方向。它適合作為 power-electronics firmware portfolio project，但不是已驗證的 production release。

## 1. Repository 角色

- **用途：** Mini PSU control-firmware architecture 與 development 的 public portfolio repository。
- **狀態：** public showcase and active prototype。
- **範圍：** 非機密 power-control firmware structure、documented interfaces、reproducible examples。
- **正式產品狀態：** prototype / portfolio code；硬體使用前必須獨立 review、calibration、protection validation。

## 2. Project goal

目標是實作一個 digital-control programmable power supply，包含：

- regulated output voltage and current
- fast transient response target
- over-voltage protection，OVP
- over-current protection，OCP
- over-temperature protection，OTP
- host communication for configuration and monitoring

## 3. Hardware platform

目前 README 歷史描述的 intended platform：

- **Control board：** Mini PSU Control Board V1.0 或 compatible hardware。
- **MCU family：** TI C2000 F28388D / F28377D class high-resolution PWM MCU。

## 4. Power topology

目標拓撲方向：

- synchronous buck
- bidirectional buck-boost candidate path
- digital dual-loop control：outer voltage loop + inner current loop
- HRPWM-based switching control

## 5. Firmware architecture

預期 firmware architecture 為 foreground/background + interrupt-driven real-time control：

```text
High-speed ISR, 100 kHz+ target
  - ADC conversion handling
  - 2P2Z / PID control calculation
  - HRPWM duty update
  - fast protection checks

Background / scheduled tasks
  - system state machine
  - LED/status reporting
  - thermal monitoring and fan control
  - host communication
  - parameter storage
```

## 6. Build guide

1. 安裝 Code Composer Studio v12 或更新版本。
2. 儘量將 C2000Ware 安裝在標準 TI path。
3. 透過 CCS import project。
4. 選擇目標 build configuration：
   - `RAM`：debug 用
   - `FLASH`：standalone firmware image 用
5. build 並確認產生 `.out` file。

## 7. Current status

目前文件化狀態：

- baseline clock / hardware configuration work exists
- HRPWM 與 ADC baseline direction 已記錄
- CCS project 與 build workflow 是此 repo 的核心用途之一
- closed-loop control 與 physical-board validation 仍屬 future work

## 8. Safety notes

這是 power-electronics firmware。未 review 下列項目前，不要直接上實體硬體：

- ADC scaling and calibration
- PWM polarity and dead-time
- current-sense polarity
- fault-trip configuration
- startup and shutdown state machine
- OVP / OCP / OTP limits
- load and supply boundaries

## 9. Working rules

1. public content 必須保持 non-confidential。
2. 未驗證硬體行為要明確標示。
3. 不要把 prototype code 描述成 production-ready。
4. generated build output 不進 version control。
5. 每個 hardware result 都要記錄 board version、toolchain、test conditions。
