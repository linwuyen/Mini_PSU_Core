# Mini PSU Core v0.1-A Walkthrough

我們已經成功完成了 **v0.1-A: ePWM Bring-Up** 的實作與本機編譯驗證。所有的變更範疇均嚴格限制在 ePWM 與 Buffer 啟用的必要邏輯，無任何未授權的 ADC, ISR, 或閉迴路控制代碼。

## Changes Made

### 1. 腳位與資源重分配 (SysConfig)
修改了 [led_ex2_blinky_sysconfig_cpu1.syscfg](file:///d:/GITHUB/Mini_PSU_Core/sysconfig_cpu1/led_ex2_blinky_sysconfig_cpu1.syscfg)：
- **`CPU1_LED`**：重分配至 `GPIO31`，釋放 `GPIO0`。
- **`CPU2_LED`**：重分配至 `GPIO34`，釋放 `GPIO1`。
- **`PWM_BUFFER_EN`**：新增一組 GPIO，綁定至 `GPIO99`（Output 模式，且配置 `writeInitialValue = true` 及 `initialValue = 0`，以在上電時自動將其拉低使能板載 74LVXC3245TTR 緩衝晶片）。
- **`myEPWM1`**：新增 `EPWM1` 外設實例：
  - `EPWM1A` 綁定至 `GPIO0`。
  - `EPWM1B` 綁定至 `GPIO1`。
  - 計數模式設定為遞增模式 (`EPWM_COUNTER_MODE_UP`)。
  - 除頻器 `CLKDIV = /1`, `HSPCLKDIV = /1`（時基頻率為系統時脈 200 MHz）。
  - 週期暫存器 `TBPRD = 1999`（每週期 2000 個時脈，頻率為固定的 100 kHz）。
  - 比較暫存器 `CMPA = 1000`（達成 50% 占空比）。
  - 動作限定器（Action Qualifier）：當計數器為 0 時拉高輸出，當計數器等於 CMPA 時拉低輸出。

### 2. 時基時鐘啟用 (CPU1 Main)
修改了 [cpu1_main.c](file:///d:/GITHUB/Mini_PSU_Core/sysconfig_cpu1/cpu1_main.c)：
- 在 `Board_init();` 之後新增呼叫 `SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC)`，這是 C2000 F28388D 啟用 ePWM Time-Base Clock 的標準驅動庫調用。

## Verification Results

### Headless CCS Build
我們在本地端透過 CCS 12.8.1 的無介面編譯工具 (`eclipsec.exe`) 進行了編譯驗證：
- **專案名稱**：`empty_sysconfig_cpu1`
- **組態**：`FLASH`
- **結果**：**0 errors, 1 warning (與 SysConfig 生成相關的隱式聲明警告，非錯誤)**，成功生成 `empty_sysconfig_cpu1.out` 韌體鏡像。

```text
Finished building target: "empty_sysconfig_cpu1.out"
 
**** Build Finished ****

================================================================================
CCS headless build complete! 0 out of 1 projects have errors.
```

## Next Steps for User (Manual Verification)
請在實體硬體上進行以下步驟量測波形：
1. **硬體設置**：
   - 確保板上已插上 **JP4**（使用 GPIO99 作為 buffer enable）。
   - 確保未插上 **JP5**（防止兩個 jumper 同時插入）。
2. **示波器接法**：
   - Scope GND → 板卡 Ground。
   - Scope CH1 → 排針 J5 上的 `EPWM1A` (GPIO0)。
3. **預期觀察指標**：
   - 穩定的方波。
   - 頻率 (Frequency) ≈ 100 kHz (週期 10 us)。
   - 占空比 (Duty Cycle) ≈ 50%。
