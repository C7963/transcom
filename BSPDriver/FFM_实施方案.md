# FFM 实施方案（MZ121，独立于 RTSA）

## 结论

FFM 不能复用或改名 RTSA。它是独立的 FPGA 工作模式：宽带频谱走 `read3`，窄带 IQ/音频走 `read6`。实施必须分阶段完成，每阶段独立编译和硬件验证，避免影响已验证的 PSCAN RMS 链路。

## 旧 C# 的确认结果

- 运行时入口：`ReceiveDataService/DataService.cs` 的 `OrderType.FFM`。
- 参数结构：`Utilities/SocketDataModel.cs` 的 `FFMParameter`。
- FPGA FFM 参数区：`Utilities/sweepLogic_ffm.cs`，基地址 `0x10011000`。
- 宽带原始 IQ：`PCIE_FFMData` 使用 `Xillybus_Read3_32`，每次读取 `4096 * 8 = 32768` 字节。
- 窄带 40 路交织 IQ：`PCIE_MultiFFMData2` 使用 `Xillybus_Read6_32`。单频点 FFM 仅取第 0 路用于 IQ 和 AM/FM/CW 音频。
- `read2/read7/read8` 的 MultiFFM 类存在，但在本服务的单频点 FFM 实际运行路径没有调用。
- 校准项目 `ReceiverCalibration` 是校准参考，不是 MZ121 运行时配置来源；其中含有旧硬件的 ADC/时钟值，不能直接复制。

## 阶段 1：移除 RTSA，保持 PSCAN 正常

目标：不保留 RTSA，但不丢失现有 `InitDevice()` 的触发源寄存器初始化。

### 需要修改

1. `CommonManager.cpp`
   - 将 `Device_Data_RTSA::getInstance()->set_trigger_source(0)` 替换为直接写寄存器：

     ```cpp
     pcie_mem_->SendData(0x1001000C, 0);
     ```

   - `CloseDevice()` 不再调用 `Device_Data_RTSA::Device_CloseDevice()`。
   - 删除 `SetWorkMode()` 中的 RTSA 分支。

2. `Global.h`
   - 删除 `WorkMode::RTSA`。

3. `Device_Data.h/.cpp`
   - 删除 `RTSA_Impl` 和 `Device_Data_RTSA`。
   - 暂时保留 `Device_Data_Multi`，它不能被假定为 RTSA 专属。

4. 工程文件
   - 从 `TransComApi.vcxproj/.filters` 与 `BSPDriverConsole.vcxproj` 移除：
     `RTSA.cpp/.h`、`RTSACLi.cpp/.h`、`DataRead.cpp/.h`。
   - 删除上述 RTSA 文件。

### 验收

- DLL、控制台均编译通过。
- PSCAN 的 `InitDevice()`、配置、读取一帧、连续启动停止均正常。
- 不再有任何 `RTSA`、`RTSACLi`、`Device_Data_RTSA` 引用。

## 阶段 2：FFM 宽带频谱控制台验证

目标：先得到与旧 C# 同样的 `read3 -> 2048 FFT -> 1601 点` 宽带频谱。不接 Qt，不做 IQ、音频、ITU。

### 新增文件

| 文件 | 职责 |
| --- | --- |
| `FfmDefs.h` | `FfmConfig`、`SpectrumFrame`、RF 模式枚举 |
| `FfmHardwareProfile.h/.cpp` | MZ121 采样率、Span/IFBW 抽取档位 |
| `FfmConfigurator.h/.cpp` | 工作模式、ADC NCO、FFM 寄存器块、射频命令 |
| `FfmDataSource.h/.cpp` | `read3` 的打开、读取、关闭 |
| `FfmSpectrumProcessor.h/.cpp` | IQ 解包、窗函数、FFT、1601 点与校准 |
| `FfmController.h/.cpp` | 采集线程、最新频谱缓存、启动停止 |
| `FfmApi.h/.cpp` | DLL C 接口 |
| `FfmSmokeTest.cpp` | 独立硬件验证程序 |

### MZ121 固定配置

```text
ADC 采样率      1.2288 GHz
FFM 内部 FS     614.4 MHz
ADC NCO 偏移    307.2 MHz
RF 分界         10 MHz（旧 C# App.config）
中心频率量化    10 kHz（旧 C# MZ121 Global.RFRBW）
```

Span 只能从 MZ121 表中选：`480M/240M/120M/60M/30M/15M/7.5M/3M/1.5M/750k/...`。

注意：20 MHz 不是该 C# MZ121 FFM 表中的合法档位；首测使用 30 MHz 或 15 MHz。

### FFM 配置寄存器顺序

```text
0x10011000 + 511 = 0             停止 FFM
0x00005010 = 4                   选择 FFM 工作模式
0x000C0007 = 2
0x00006000 = 0
ADC0(0x000C2000) + 0x310 = 0x43
ADC0 + 0x316~0x31B               写 ADC NCO 的 48-bit FTW
0x00006008 = 0 或 1             NCO 通路选择
0x00010001/3/4/0                下发 RFATT、IFATT、RFMode、Freq 命令
0x10011000 + 0..159             写 FFM FTW 表（单频点阶段全零）
0x10011000 + 508                Span Decim
0x10011000 + 509                MultiDecim + 1
0x10011000 + 510                FFT 长度配置（2048）
0x10011000 + 511 = 1，再写 3    启动 FFM
```

低频 NCO：`centerFreq <= 10 MHz` 时用中心频率计算 FTW、`0x6008=1`；其余频率用 `307.2 MHz` 计算 FTW、`0x6008=0`。

### read3 与频谱处理

```text
read3：32768 字节
  -> Decim=0：16-bit I/Q（含两复数样点交换）
  -> Decim>0：32-bit I + 32-bit Q
  -> 取 2048 个复数样点
  -> Blackman-Harris 窗
  -> 2048 点 FFT + fftshift
  -> Skip(224) / Take(1601) / Reverse
  -> 校准，缓存最新频谱
```

频率轴：

```cpp
startFreq = centerFreq - span / 2.0;
rbw = span * 1.28 / 2048.0;
freq[i] = startFreq + i * rbw;
```

幅度需要同时保留 `rawFftDb`、`correctedDbm` 和 `displayDbuv=correctedDbm+107` 以方便与 C# 对比；Qt 的 dBm 曲线应使用 `correctedDbm`，不能把 dBµV 标为 dBm。

### 阶段验收

- `read3` 成功打开，单帧实际读取恒为 32768 字节。
- 每帧输出 1601 个有限数值，不能出现 NaN/Inf。
- 中心信号峰值接近点 800；移动信号源时峰值移动约等于 `DeltaFrequency / rbw`。
- 20 次 Start/Stop 不挂死。
- 完成后再次运行 PSCAN 仍正常。

## 阶段 3：Qt 接入 FFM 宽带频谱

新增 `PscanTestClient/FfmDriver.h/.cpp`，动态加载：

```cpp
FfmApi_Init(...)
FfmApi_SetConfig(...)
FfmApi_Start()
FfmApi_Stop()
FfmApi_GetSpectrumData(...)
```

Qt 增加 `PSCAN / FFM` 模式。切换顺序固定为：停止当前模式 -> 等待线程退出 -> 配置目标模式 -> 启动目标模式。

`SpectrumWidget` 不需要修改，FFM 直接提供 `vector<double> frequencies` 和 `vector<double> amplitudes`。

## 阶段 4：窄带 IQ、音频、ITU

仅在阶段 2、3 通过后实施。

1. 修正并验证 C++ `read6` 路径为：

   ```cpp
   R"(\\.\xillybus_read6)"
   ```

2. 从 `read6` 读取 `sampleCount * 40 * 8` 字节，解析 `I0,Q0,...,I39,Q39`，先提取第 0 路。
3. 按旧 C# 行为实现 IFBW 重采样、IQ 输出、AM/FM/CW 解调。
4. 最后移植 ITU：Level、XdB、Beta、SNR。

## 已知风险与约束

- 旧 C# 单 FFM 分支没有每次重置 `FFMSpanErrorValue`；新 C++ 必须每次配置显式查表，缺失则置零。
- 旧 C# 的 `GetFreqErrorData()` 首行直接返回 0；不要误认为频率校准已生效。
- 旧 C# 最终对频谱总是 `Reverse()`；新代码按最终实际行为实现。
- 当前 C++ `Device_Address.cpp` 的 `read6` 字符串疑似多一个反斜杠；PSCAN 不使用它，必须在阶段 4 单独验证。
