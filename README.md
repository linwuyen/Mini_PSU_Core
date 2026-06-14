# Mini_PSU_Core

本專案為 Mini PSU (微型可程式化電源供應器) 控制板的核心韌體專案。

## 專案目標 (Project Goal)
實現高效能、高精度的可程式化數位控制電源。提供精確的電壓、電流調節，快速的瞬態響應，以及完善的過壓 (OVP)、過流 (OCP) 與過溫 (OTP) 等硬體保護與軟體防護機制。

## 硬體平台 (Hardware Platform)
- **硬體板型**: Mini PSU Control Board V1.0 及相容硬體電路。
- **MCU (微控制器)**: Texas Instruments C2000 F28388D / F28377D 系列高解析度 PWM 微控制器。

## 電源拓撲 (Power Topology)
- **主拓撲**: 同步降壓拓撲 (Synchronous Buck Topology) / 雙向升降壓 (Buck-Boost)。
- **控制方式**: 數位雙環控制（外環電壓環 + 內環電流環），支援高頻高解析度 HRPWM 驅動。

## 韌體架構 (Firmware Architecture)
本系統基於前后台與時間片輪詢架構設計，確保即時性控制與通訊的平衡：
1. **即時控制環路 (高速中斷，100kHz+)**：
   - 由 ADC 轉換完成中斷觸發。
   - 執行 2P2Z 或 PID 演算法更新 HRPWM 占空比。
   - 快速過流與過壓保護檢測。
2. **背景任務 (主循環/定時任務，10Hz - 1000Hz)**：
   - 系統狀態機切換與指示燈控制。
   - 溫度監控與散熱風扇 PID 調節。
   - 與上位機通訊（Modbus RTU / 自定義協定）。
   - 參數保存 (EEPROM / Flash 模擬 EEPROM)。

## 編譯指南 (Build Guide)
1. 安裝 **Code Composer Studio (CCS) v12** 或更新版本。
2. 下載並安裝 **C2000Ware SDK** (建議使用預設路徑 `C:\ti\c2000\C2000Ware_x_xx_xx_xx`)。
3. 啟動 CCS 並導入本專案目錄。
4. 選擇編譯配置：
   - `FLASH`: 生成生產發行版韌體，將代碼燒錄於晶片 Flash。
   - `RAM`: 用於硬體仿真調試。
5. 點擊 `Build Project` 按鈕進行編譯，產生 `.out` 映像檔。

## 目前狀態 (Current Status)
- [x] 完成基本系統時鐘與 HwConfig 初始化設定。
- [x] 完成基本 HRPWM 與 ADC 配置基線。
- [x] 整合基本 CCS 專案設定與自動編譯腳本。
- [ ] 待進行閉環控制演算法整合與物理板實機調試。

## 待辦事項 (TODO)
- [ ] 實作數位電壓電流 PID 閉環控制演算法。
- [ ] 整合上位機 Modbus 通訊通訊協定。
- [ ] 進行硬體過載與瞬態響應極限測試。
- [ ] 撰寫詳細的硬體校準與補償指南。


---

## Repository Classification

- **Purpose:** public portfolio repository for Mini PSU control-firmware architecture and development.
- **Status:** public showcase and active prototype.
- **Scope:** non-confidential power-control firmware structure, documented interfaces, and reproducible examples.
- **Production status:** prototype/portfolio code; hardware use requires independent review, calibration, and protection validation.
