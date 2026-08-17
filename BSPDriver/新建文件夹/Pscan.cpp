#include "Pscan.h"
// Legacy Sweep path retained for rollback, but disabled for the RMS PSCAN path.
// #include "SweepLogic.h"
#include "CommonManager.h"
#include "Global.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <windows.h>
#include "Device_MEM32.h"

#pragma unmanaged

// BSPDriver diagnostic log
static std::string GetDllDir()
{
    char path[MAX_PATH] = { 0 };
    HMODULE hMod = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetDllDir, &hMod))
    {
        GetModuleFileNameA(hMod, path, MAX_PATH);
    }
    std::string s(path);
    auto pos = s.find_last_of("\\/");
    return (pos != std::string::npos) ? s.substr(0, pos) : ".";
}

static void BSLog(const char* msg)
{
    std::string logPath = GetDllDir() + "\\bsdriver_diag.log";
    std::ofstream log(logPath, std::ios::app);
    if (log.is_open())
    {
        log << msg << std::endl;
    }
}

static std::vector<uint8_t> BuildRmsPscanBody(const PSCANCONFIG::PscanConfig& config)
{
    std::vector<uint8_t> body(65, 0);
    const char taskId[] = "QT_PSCAN_RMS";
    std::memcpy(body.data(), taskId, sizeof(taskId) - 1);

    const uint64_t startFreq = static_cast<uint64_t>(
        std::llround(config.centerFreq - config.span / 2.0));
    const uint64_t stopFreq = static_cast<uint64_t>(
        std::llround(config.centerFreq + config.span / 2.0));
    const uint32_t step = static_cast<uint32_t>(std::llround(config.step));
    const uint8_t rfAtt = 0;
    const uint8_t agc = 0;
    const uint8_t ifAtt = 0;
    const uint8_t rfMode = 3;        // NORM
    const uint16_t detectorMode = 0; // Crop the RMS frame to PSCAN point count.

    std::memcpy(body.data() + 39, &startFreq, sizeof(startFreq));
    std::memcpy(body.data() + 47, &stopFreq, sizeof(stopFreq));
    std::memcpy(body.data() + 55, &step, sizeof(step));
    body[59] = rfAtt;
    body[60] = agc;
    body[61] = ifAtt;
    body[62] = rfMode;
    std::memcpy(body.data() + 63, &detectorMode, sizeof(detectorMode));
    return body;
}

static bool WritePscanRegister(Device::Device_MEM32* mem, uint32_t address,
    uint32_t value, const char* name)
{
    const bool ok = mem != nullptr && mem->SendData(address, value);
    std::ostringstream oss;
    oss << "[PSCAN_MODE] " << name
        << " address=0x" << std::hex << std::uppercase << std::setw(8)
        << std::setfill('0') << address
        << " value=0x" << std::setw(8) << value
        << std::dec << " result=" << (ok ? "OK" : "FAILED");
    BSLog(oss.str().c_str());
    return ok;
}

static bool ConfigurePscanHardwareMode()
{
    auto* mem = Device::Device_MEM32::getInstance();
    bool ok = true;

    // Match TransComReceiver LocalServerInit.ChangeViewMenu(PSCAN).
    ok = WritePscanRegister(mem, 0x00005010, 0x00000001, "logic work mode") && ok;
    ok = WritePscanRegister(mem, 0x00005000, 0x00000004, "ADC channel") && ok;
    ok = WritePscanRegister(mem, 0x000A0000, 0x00000001, "PSCAN mode") && ok;
    ok = WritePscanRegister(mem, 0x000C2310, 0x00000063, "ADC0 PSCAN DDC mode") && ok;
    ok = WritePscanRegister(mem, 0x000C2330, 0x00000043, "ADC0 complex output") && ok;
    ok = WritePscanRegister(mem, 0x000C2336, 0x00000000, "ADC0 DDC NCO byte0") && ok;
    ok = WritePscanRegister(mem, 0x000C2337, 0x00000000, "ADC0 DDC NCO byte1") && ok;
    ok = WritePscanRegister(mem, 0x000C2338, 0x00000000, "ADC0 DDC NCO byte2") && ok;
    ok = WritePscanRegister(mem, 0x000C2339, 0x00000000, "ADC0 DDC NCO byte3") && ok;
    ok = WritePscanRegister(mem, 0x000C233A, 0x00000080, "ADC0 DDC NCO byte4") && ok;
    ok = WritePscanRegister(mem, 0x000C233B, 0x00000025, "ADC0 DDC NCO byte5") && ok;
    BSLog(ok
        ? "[PSCAN_MODE] C# PSCAN ADC/DDC mode sequence complete"
        : "[PSCAN_MODE] C# PSCAN ADC/DDC mode sequence FAILED");
    return ok;
}

static bool WritePscanRfModeRegister(Device::Device_MEM32* mem, uint32_t address,
    uint32_t value, const char* name)
{
    const bool ok = mem != nullptr && mem->SendData(address, value);
    std::ostringstream oss;
    oss << "[PSCAN_RF_MODE] " << name
        << " address=0x" << std::hex << std::uppercase << std::setw(8)
        << std::setfill('0') << address
        << " value=0x" << std::setw(8) << value
        << std::dec << " result=" << (ok ? "OK" : "FAILED");
    BSLog(oss.str().c_str());
    return ok;
}
static bool ConfigurePscanCSharpFrontend()
{
    auto* mem = Device::Device_MEM32::getInstance();
    bool ok = true;

    // Match LocalServerInit.ChangeViewMenu(PSCAN): after SpectrumSweep.Config(),
    // C# applies IF gain, RF gain, then RF work mode and enables the output.
    // PSCAN defaults are IF=0 dB, RF=0 dB and NORM.
    ok = WritePscanRfModeRegister(mem, 0x000A0001, 0x00000004,
        "IF gain command") && ok;
    ok = WritePscanRfModeRegister(mem, 0x000A0002, 0x00000000,
        "IF gain value 0 dB") && ok;

    ok = WritePscanRfModeRegister(mem, 0x000A0001, 0x00000002,
        "RF gain command") && ok;
    ok = WritePscanRfModeRegister(mem, 0x000A0002, 0x00000000,
        "RF gain value 0 dB") && ok;
    ok = WritePscanRfModeRegister(mem, 0x000A0001, 0x80000100,
        "RF gain output enable") && ok;

    ok = WritePscanRfModeRegister(mem, 0x000A0001, 0x00000008,
        "RF mode command") && ok;
    ok = WritePscanRfModeRegister(mem, 0x000A0002, 0x00000001,
        "RF NORM value") && ok;
    ok = WritePscanRfModeRegister(mem, 0x000A0001, 0x80000100,
        "RF mode output enable") && ok;

    BSLog(ok
        ? "[PSCAN_RF_MODE] C# PSCAN front-end sequence complete IF=0 RF=0 Mode=NORM"
        : "[PSCAN_RF_MODE] C# PSCAN front-end sequence FAILED");
    return ok;
}

static bool ConfigurePscanRfCenter(const PSCANCONFIG::PscanConfig& config)
{
    auto& common = Common::CommonManager::Instance();
    const uint64_t centerFreq = static_cast<uint64_t>(config.centerFreq);
    const double loFreq = centerFreq > 1.5e9
        ? static_cast<double>(centerFreq) - 307.2e6
        : static_cast<double>(centerFreq) + 307.2e6;
    const double expectedDdcNco = centerFreq > 1.5e9
        ? Common::CommonManager::ADCSampleClock / 4.0
        : Common::CommonManager::ADCSampleClock * 0.75;

    {
        std::ostringstream oss;
        oss << "[PSCAN_RF_CENTER] begin"
            << " CF=" << config.centerFreq
            << " LO=" << loFreq
            << " expectedDdcNco=" << expectedDdcNco;
        BSLog(oss.str().c_str());
    }

    try
    {
        if (!common.rfControl_)
        {
            BSLog("[PSCAN_RF_CENTER] RF control unavailable");
            return false;
        }

        // RFControl dispatches to the active RF card. For RF12 this writes
        // A0201/A0202/A0200 and programs both ADC DDC NCOs.
        const bool rfResult = common.rfControl_->SetCenterFreq(centerFreq);
        {
            std::ostringstream oss;
            oss << "[PSCAN_RF_CENTER] RF mixer LO/DDC call completed"
                << " rfType=" << static_cast<int>(RFCONTROL::RFControl::RF_SELECT)
                << " requestedCenter=" << centerFreq
                << " requestedLO=" << loFreq
                << " result=" << (rfResult ? "OK" : "FAILED");
            BSLog(oss.str().c_str());
        }

        if (common.adcconfig_)
        {
            std::ostringstream oss;
            oss << "[PSCAN_ADC_NCO]"
                << " trackedFrequency=" << common.adcconfig_->get_last_nco_frequency()
                << " initResult=" << common.adcconfig_->get_last_init_result()
                << " ncoWriteState=" << (common.adcconfig_->has_nco_write_attempt() ? "called" : "not_called")
                << " ch0Result=" << common.adcconfig_->get_last_nco_ch0_result()
                << " ch1Result=" << common.adcconfig_->get_last_nco_ch1_result();
            BSLog(oss.str().c_str());
        }


        common.UpdateErrorValue(centerFreq);
        BSLog("[PSCAN_RF_CENTER] frequency error calibration updated");

        // RF12::SetCenterFreq already programs the ADC DDC NCO and filter.
        // MZ121 follows the C# path: InitDevice initializes the ADC DDC and
        // the RF card only receives the RF LO update at runtime. Repeating
        // the common filter transaction here can fail on the remote hardware.
        if (RFCONTROL::RFControl::RF_SELECT == RFCONTROL::RFType::RF12)
        {
            BSLog("[PSCAN_RF_CENTER] ADC filter skipped for RF12 (already configured by RF12)");
        }
        else if (RFCONTROL::RFControl::RF_SELECT == RFCONTROL::RFType::MZ121)
        {
            BSLog("[PSCAN_RF_CENTER] ADC filter skipped for MZ121 (C# compatible InitDevice path)");
        }
        else
        {
            BSLog("[PSCAN_RF_CENTER] ADC filter update begin");
            common.SetADCFilter(centerFreq);
            BSLog("[PSCAN_RF_CENTER] ADC filter update completed");
        }
        BSLog("[PSCAN_RF_CENTER] result=OK");
        return true;
    }
    catch (const std::exception& ex)
    {
        BSLog((std::string("[PSCAN_RF_CENTER] result=FAILED exception=") + ex.what()).c_str());
        return false;
    }
    catch (...)
    {
        BSLog("[PSCAN_RF_CENTER] result=FAILED unknown exception");
        return false;
    }
}

static const char* GetRfTypeName(RFCONTROL::RFType type)
{
    switch (type)
    {
    case RFCONTROL::RFType::RF12: return "RF12";
    case RFCONTROL::RFType::CM18: return "CM18";
    case RFCONTROL::RFType::RPU44: return "RPU44";
    case RFCONTROL::RFType::MZ116: return "MZ116";
    case RFCONTROL::RFType::MZ121: return "MZ121";
    case RFCONTROL::RFType::MZ121B: return "MZ121B";
    default: return "Unknown";
    }
}

static void LogPscanRfState(const char* stage, const PSCANCONFIG::PscanConfig& config)
{
    uint32_t rfAtt = 0;
    uint32_t ifAtt = 0;
    bool attRead = false;
    try
    {
        auto& common = Common::CommonManager::Instance();
        rfAtt = common.GetRFATT();
        ifAtt = common.GetIFATT();
        attRead = true;
    }
    catch (...)
    {
    }

    std::ostringstream oss;
    oss << "[PSCAN_RF_STATE]"
        << " stage=" << stage
        << " rfType=" << GetRfTypeName(RFCONTROL::RFControl::RF_SELECT)
        << " rfTypeValue=" << static_cast<int>(RFCONTROL::RFControl::RF_SELECT)
        << " rfAtt=" << rfAtt
        << " ifAtt=" << ifAtt
        << " attRead=" << (attRead ? "OK" : "FAILED")
        << " refLevel=" << config.refLevel
        << " CF=" << config.centerFreq
        << " Span=" << config.span
        << " RBW=" << config.rbw
        << " Step=" << config.step;
    BSLog(oss.str().c_str());
}
namespace PSCAN
{
    Pscan::Pscan()
    {
        BSLog("[PSCAN_CTOR] begin");
        // Legacy Sweep/SpectrumSweep construction retained for rollback:
        // sweep_ = SWEEPCONFIG::Sweep::getInstance();
        // spectrumSweep_ = std::make_unique<SWEEPCONFIG::SpectrumSweep>(sweep_);
        pscanOrder_ = std::make_unique<DATASERVICE::PscanOrder>();
        BSLog("[PSCAN_CTOR] stage=PscanOrder RMS path ready");
        BSLog("[PSCAN_CTOR] stage=CallbackPscanDataSink begin");
        dataSink_ = std::make_shared<CallbackPscanDataSink>();
        BSLog("[PSCAN_CTOR] stage=CallbackPscanDataSink done");
        BSLog("[PSCAN_CTOR] success");
    }
    Pscan::~Pscan()
    {
        Stop();
    }

    Pscan& Pscan::GetInstance()
    {
        static Pscan instance;
        return instance;
    }

    void Pscan::InitializeState(double cf, double span, uint32_t rbw)
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        // Legacy Sweep initialization retained for rollback:
        // if (sweep_) sweep_->SetReverseSpectrumData(false);
        // const bool modeConfigured = ConfigurePscanHardwareMode();
        BSLog("[PSCAN_DATA_ORDER] PscanOrder RMS FIFO path selected");
        config_.centerFreq = cf;
        config_.span = span;
        config_.rbw = rbw;
        // InitDevice is complete here; defer PscanOrder hardware configuration
        // until the UI submits the complete parameter set.
        configured_.store(false);
        processedBuffer_.clear();
        uint64_t version = ++configVersion_;
        {
            std::ostringstream oss;
            oss << "[PSCAN_CONFIG_VERSION] InitDeferred"
                << " version=" << version
                << " CF=" << config_.centerFreq
                << " Span=" << config_.span
                << " RBW=" << config_.rbw
                << " Step=" << config_.step
                << " PointCount=" << config_.GetPointCount()
                << " pscanOrderConfig=deferred_until_SetConfig";
            BSLog(oss.str().c_str());
        }
    }

    bool Pscan::SetPara(const PSCANCONFIG::PscanConfig& config)
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        if (isRunning_.load())
        {
            BSLog("[PSCAN_ORDER_CONFIG] rejected while acquisition is running");
            return false;
        }
        config_ = config;
        config_.traceModels.clear();
        config_.traceModels.emplace_back(
            PSCANCONFIG::TracesType::Trace1,
            PSCANCONFIG::DetectorType::RMS);
        config_.UpdateDetectorBitmask();
        uint64_t version = ++configVersion_;
        {
            std::ostringstream oss;
            oss << "[PSCAN_CONFIG_VERSION] SetParaBegin"
                << " version=" << version
                << " CF=" << config_.centerFreq
                << " Span=" << config_.span
                << " RBW=" << config_.rbw
                << " Step=" << config_.step
                << " RefLevel=" << config_.refLevel
                << " Unit=" << static_cast<int>(config_.unitType)
                << " DetectorMask=" << config_.detectorBitmask
                << " PointCount=" << config_.GetPointCount();
            BSLog(oss.str().c_str());
        }
        // Legacy SpectrumSweep configuration retained for rollback:
        // const bool configApplied = spectrumSweep_ && spectrumSweep_->Config(config_, "SetPara");
        const bool configApplied = ConfigureRmsOrder("SetPara");
        configured_.store(configApplied);
        processedBuffer_.resize(configApplied && pscanOrder_
            ? pscanOrder_->PscanSpectrumNum()
            : config_.GetPointCount());
        // Reset trace accumulation on parameter change
        traceAccumulator_.clear();
        traceFrameCount_.clear();
        cachedData_.clear();
        hasCachedData_ = false;
        {
            std::ostringstream oss;
            oss << "[PSCAN_CONFIG_VERSION] SetParaEnd"
                << " version=" << version
                << " CF=" << config_.centerFreq
                << " Span=" << config_.span
                << " RBW=" << config_.rbw
                << " Step=" << config_.step
                << " PointCount=" << config_.GetPointCount();
            BSLog(oss.str().c_str());
        }
        return configured_.load();
    }

    bool Pscan::EnsureConfigured(const char* reason)
    {
        if (configured_.load())
        {
            return true;
        }

        std::lock_guard<std::mutex> lock(dataMutex_);
        if (configured_.load())
        {
            return true;
        }

        // Legacy SpectrumSweep fallback retained for rollback:
        // if (!spectrumSweep_ || !spectrumSweep_->Config(config_, reason)) return false;
        const bool configured = ConfigureRmsOrder(reason);
        configured_.store(configured);
        return configured;
    }

    bool Pscan::ConfigureRmsOrder(const char* reason)
    {
        if (!std::isfinite(config_.centerFreq) || !std::isfinite(config_.span) ||
            !std::isfinite(config_.step) || config_.span < 0.0 || config_.step <= 0.0)
        {
            BSLog("[PSCAN_ORDER_CONFIG] invalid numeric parameters");
            return false;
        }

        const double startFreq = config_.centerFreq - config_.span / 2.0;
        const double stopFreq = config_.centerFreq + config_.span / 2.0;
        if (startFreq < 9000.0 || stopFreq > 18000000000.0 || stopFreq < startFreq)
        {
            BSLog("[PSCAN_ORDER_CONFIG] frequency range rejected");
            return false;
        }

        // Destroy the previous order first so its Xilly read handle is closed
        // before a new configuration opens or uses the same FIFO.
        pscanOrder_.reset();
        auto order = std::make_unique<DATASERVICE::PscanOrder>();
        DATASERVICE::PscanOrderContext context;
        context.RFModule = "mz121";
        context.ADC0_Address = 0x000C2000;
        context.Pscan_BW = 614.4e6;
        context.Fs_ADC = static_cast<double>(Common::CommonManager::ADCSampleClock);
        context.Rf_Sub = 180.0e6;
        context.RFRBW = 1000;
        context.PscanErrorValue = 0.0; // Match the verified console path exactly.
        context.RFModeError = 0.0;
        context.Lown = 0.0;
        context.Lowd = 0.0;

        const std::vector<uint8_t> body = BuildRmsPscanBody(config_);
        if (!order->Handle(body, context))
        {
            std::ostringstream oss;
            oss << "[PSCAN_ORDER_CONFIG] failed reason=" << (reason ? reason : "unknown");
            BSLog(oss.str().c_str());
            return false;
        }

        pscanOrder_ = std::move(order);
        std::ostringstream oss;
        oss << "[PSCAN_ORDER_CONFIG] success"
            << " reason=" << (reason ? reason : "unknown")
            << " detector=RMS"
            << " points=" << pscanOrder_->PscanSpectrumNum();
        BSLog(oss.str().c_str());
        return true;
    }

    void Pscan::SetDataSource(std::shared_ptr<IPscanDataSource> dataSource)
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        dataSource_ = dataSource;
    }

    void Pscan::SetDataSink(std::shared_ptr<IPscanDataSink> dataSink)
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        dataSink_ = dataSink;
    }

    void Pscan::Start()
    {
        if (isRunning_.load()) return;
        if (!EnsureConfigured("StartFallback")) return;

        isStopRequested_ = false;
        isRunning_ = true;
        runMode_.store(PSCANCONFIG::RunMode::RunContinue);
        LogPscanRfState("Start", config_);

        // Clear trace accumulation for fresh start
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            traceAccumulator_.clear();
            traceFrameCount_.clear();
            cachedData_.clear();
            hasCachedData_ = false;
        }

        // Item 2: Start producer + consumer threads
        producerThread_ = std::thread(&Pscan::ProducerThread, this);
        consumerThread_ = std::thread(&Pscan::ConsumerThread, this);
    }

    void Pscan::Stop()
    {
        if (!isRunning_.load()) return;

        isStopRequested_ = true;
        isRunning_ = false;
        runMode_.store(PSCANCONFIG::RunMode::Stop);

        cv_.notify_all();
        queueCV_.notify_all();

        if (producerThread_.joinable()) producerThread_.join();
        if (consumerThread_.joinable()) consumerThread_.join();

        // Clear queue
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            while (!frameQueue_.empty()) frameQueue_.pop();
        }
    }

    void Pscan::RunSingle()
    {
        if (isRunning_.load()) Stop();
        if (!EnsureConfigured("RunSingleFallback")) return;

        runMode_.store(PSCANCONFIG::RunMode::RunSingle);
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            cachedData_.clear();
            hasCachedData_ = false;
        }

        // Single mode: PscanOrder reads one already-configured RMS frame.
        // Legacy Sweep trigger retained for rollback:
        // if (sweep_) sweep_->SetTriggerOnNextRead(true);

        std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> dataPairs;
        if (GetSpectrumData(dataPairs))
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            cachedData_ = dataPairs;
            hasCachedData_ = true;
            UploadToUI(dataPairs);
        }

        runMode_.store(PSCANCONFIG::RunMode::Stop);
    }

    void Pscan::SetRunMode(PSCANCONFIG::RunMode mode)
    {
        if (mode == PSCANCONFIG::RunMode::RunContinue) Start();
        else if (mode == PSCANCONFIG::RunMode::RunSingle) RunSingle();
        else if (mode == PSCANCONFIG::RunMode::Stop) Stop();
    }

    PSCANCONFIG::RunMode Pscan::GetRunMode() const
    {
        return runMode_.load();
    }

    // =========================================================================
    // Item 2: Producer thread - reads raw dB data from hardware, pushes to queue
    // =========================================================================
    void Pscan::ProducerThread()
    {
        // PscanOrder::Handle already configured the continuous RMS FIFO.
        // Legacy Sweep trigger retained for rollback:
        // if (sweep_) sweep_->SetTriggerOnNextRead(false);

        while (!isStopRequested_.load())
        {
            PSCANCONFIG::RunMode currentMode = runMode_.load();
            if (currentMode == PSCANCONFIG::RunMode::Stop)
            {
                std::unique_lock<std::mutex> lock(dataMutex_);
                cv_.wait_for(lock, std::chrono::milliseconds(100),
                    [this]() { return isStopRequested_.load() || runMode_.load() != PSCANCONFIG::RunMode::Stop; });
                continue;
            }

            // Read the single detector stream configured by SpectrumSweepFft.
            FrameData frame;
            std::vector<double> dbData;
            if (ReadRmsFrame(dbData) && !dbData.empty())
            {
                frame.rawFrames.emplace_back(
                    PSCANCONFIG::DetectorType::RMS,
                    std::move(dbData));
            }

            if (frame.rawFrames.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            // Push to queue (drop oldest if full - C# ConcurrentQueue behavior)
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                while ((int)frameQueue_.size() >= MAX_QUEUE_SIZE)
                    frameQueue_.pop();
                frameQueue_.push(std::move(frame));
            }
            queueCV_.notify_one();
        }
    }

    // =========================================================================
    // Item 2+6: Consumer thread - processes, trace accumulation, uploads
    // =========================================================================
    void Pscan::ConsumerThread()
    {
        while (!isStopRequested_.load())
        {
            FrameData frame;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                queueCV_.wait_for(lock, std::chrono::milliseconds(100),
                    [this]() { return isStopRequested_.load() || !frameQueue_.empty(); });

                if (isStopRequested_.load()) break;
                if (frameQueue_.empty()) continue;

                frame = std::move(frameQueue_.front());
                frameQueue_.pop();
            }

            // Process: raw dB �?frequency/amplitude pairs
            std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>> dataPairs;
            for (size_t fi = 0; fi < frame.rawFrames.size(); ++fi)
            {
                PSCANCONFIG::DetectorType detType = frame.rawFrames[fi].first;
                std::vector<double>& dbData = frame.rawFrames[fi].second;

                auto freqAmpData = ProcessData(dbData, detType);

                // Item 6: Trace accumulation (MaxHold/MinHold/Average/View)
                AccumulateTrace(freqAmpData, detType);

                dataPairs.emplace_back(detType, std::move(freqAmpData));
            }

            // Upload to UI
            if (!dataPairs.empty())
            {
                // Print 10 frames of Qt spectrum data to log
                static int g_frameLogCount = 0;
                if (g_frameLogCount < 10)
                {
                    std::ostringstream oss;
                    oss << "=== QT_SPECTRUM_FRAME[" << g_frameLogCount << "] ===" << std::endl;
                    for (size_t di = 0; di < dataPairs.size(); ++di)
                    {
                        const auto& det = dataPairs[di].first;
                        const auto& data = dataPairs[di].second;
                        oss << "  Detector=" << (int)det << " Points=" << data.size() << std::endl;

                        double minAmp = data.empty() ? 0 : data[0].amplitude;
                        double maxAmp = minAmp;
                        double peakFreq = 0;
                        for (const auto& d : data)
                        {
                            if (d.amplitude < minAmp) minAmp = d.amplitude;
                            if (d.amplitude > maxAmp) { maxAmp = d.amplitude; peakFreq = d.frequency; }
                        }
                        oss << "  FreqRange=[" << (data.empty() ? 0 : data.front().frequency)
                            << " -> " << (data.empty() ? 0 : data.back().frequency) << "] Hz"
                            << " AmpRange=[" << minAmp << ", " << maxAmp << "]"
                            << " PeakFreq=" << peakFreq << " Hz" << std::endl;

                        // Print all points
                        for (size_t i = 0; i < data.size(); ++i)
                        {
                            oss << "  [" << i << "] freq=" << data[i].frequency
                                << " amp=" << data[i].amplitude << std::endl;
                        }
                    }
                    BSLog(oss.str().c_str());
                    g_frameLogCount++;
                }

                {
                    std::lock_guard<std::mutex> lock(dataMutex_);
                    cachedData_ = dataPairs;
                    hasCachedData_ = true;
                }
                UploadToUI(dataPairs);
            }
        }
    }

    // =========================================================================
    // Item 6: Cross-frame trace accumulation
    // Aligned with C# SpectrumDataHandle
    // =========================================================================
    void Pscan::AccumulateTrace(
        std::vector<PSCANCONFIG::FreqAmpData>& currentFrame,
        PSCANCONFIG::DetectorType detectorType)
    {
        if (currentFrame.empty()) return;

        // Find the trace mode for this detector type
        PSCANCONFIG::TraceMode mode = PSCANCONFIG::TraceMode::ClearWrite;
        {
            std::lock_guard<std::mutex> lock(dataMutex_);
            for (const auto& trace : config_.traceModels)
            {
                if (trace.isEnabled && trace.detectorType == detectorType)
                {
                    mode = trace.traceMode;
                    break;
                }
            }
        }

        if (mode == PSCANCONFIG::TraceMode::ClearWrite || mode == PSCANCONFIG::TraceMode::View)
        {
            // ClearWrite: replace accumulator with current frame
            // View: keep previous frame (don't update)
            if (mode == PSCANCONFIG::TraceMode::ClearWrite)
            {
                traceAccumulator_[detectorType] = currentFrame;
                traceFrameCount_[detectorType] = 1;
            }
            // View mode: return accumulated (previous) data
            auto it = traceAccumulator_.find(detectorType);
            if (it != traceAccumulator_.end() && !it->second.empty())
            {
                currentFrame = it->second;
            }
            return;
        }

        auto& accum = traceAccumulator_[detectorType];
        auto& count = traceFrameCount_[detectorType];

        if (accum.empty())
        {
            // First frame: initialize accumulator
            accum = currentFrame;
            count = 1;
            return;
        }

        size_t n = (std::min)(accum.size(), currentFrame.size());

        switch (mode)
        {
        case PSCANCONFIG::TraceMode::MaxHold:
            for (size_t i = 0; i < n; ++i)
            {
                if (currentFrame[i].amplitude > accum[i].amplitude)
                    accum[i].amplitude = currentFrame[i].amplitude;
            }
            break;

        case PSCANCONFIG::TraceMode::MinHold:
            for (size_t i = 0; i < n; ++i)
            {
                if (currentFrame[i].amplitude < accum[i].amplitude)
                    accum[i].amplitude = currentFrame[i].amplitude;
            }
            break;

        case PSCANCONFIG::TraceMode::Average:
            count++;
            for (size_t i = 0; i < n; ++i)
            {
                // Running average: avg = avg + (new - avg) / count
                accum[i].amplitude += (currentFrame[i].amplitude - accum[i].amplitude) / (double)count;
            }
            break;

        default:
            break;
        }

        // Return accumulated data as current frame
        currentFrame = accum;
    }

    // =========================================================================
    // GetSpectrumData - synchronous interface (used by RunSingle)
    // =========================================================================
    bool Pscan::GetSpectrumData(
        std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& outData)
    {
        {
            std::lock_guard<std::mutex> lock(dataMutex_);

            // Continuous acquisition publishes the newest completed frame here.
            // Keep it available between UI timer ticks instead of consuming it once.
            if (hasCachedData_ && !cachedData_.empty())
            {
                outData = cachedData_;
                return true;
            }

            // The producer thread owns hardware reads while continuous mode runs.
            if (runMode_.load() == PSCANCONFIG::RunMode::RunContinue)
            {
                return false;
            }

        }

        outData.clear();
        std::vector<double> dbData;
        if (ReadRmsFrame(dbData) && !dbData.empty())
        {
            auto freqAmpData = ProcessData(dbData, PSCANCONFIG::DetectorType::RMS);
            AccumulateTrace(freqAmpData, PSCANCONFIG::DetectorType::RMS);
            outData.emplace_back(PSCANCONFIG::DetectorType::RMS, std::move(freqAmpData));
        }

        return !outData.empty();
    }

    bool Pscan::ReadRmsFrame(std::vector<double>& buffer)
    {
        if (!pscanOrder_) return false;
        const std::vector<uint8_t> amplitudeData = pscanOrder_->ReadOneSpectrumFrame();
        if (amplitudeData.empty() || amplitudeData.size() % sizeof(int16_t) != 0)
            return false;

        const size_t pointCount = amplitudeData.size() / sizeof(int16_t);
        buffer.resize(pointCount);
        for (size_t i = 0; i < pointCount; ++i)
        {
            int16_t amplitudeTenthsDb = 0;
            std::memcpy(&amplitudeTenthsDb,
                amplitudeData.data() + i * sizeof(int16_t),
                sizeof(amplitudeTenthsDb));
            // Keep the same amplitude value printed by the verified console test.
            buffer[i] = amplitudeTenthsDb / 10.0;
        }
        return true;
    }

#if 0
    // Legacy Sweep reader retained for rollback. The active path is ReadRmsFrame().
    bool Pscan::ReadFromSweep(std::vector<double>& buffer, PSCANCONFIG::DetectorType detectorType)
    {
        if (!sweep_) return false;
        uint32_t pointCount = config_.GetPointCount();
        if (buffer.size() < pointCount) buffer.resize(pointCount);
        bool success = false;
        switch (detectorType)
        {
        case PSCANCONFIG::DetectorType::PositivePeak: success = sweep_->GetPositivePeakData(buffer); break;
        case PSCANCONFIG::DetectorType::NegativePeak: success = sweep_->GetNegativeData(buffer); break;
        case PSCANCONFIG::DetectorType::Average: success = sweep_->GetAverageData(buffer); break;
        case PSCANCONFIG::DetectorType::Sample: success = sweep_->GetSampleData(buffer); break;
        case PSCANCONFIG::DetectorType::RMS: success = sweep_->GetRMSData(buffer); break;
        case PSCANCONFIG::DetectorType::AutoPeak:
        {
            std::vector<double> highData, lowData;
            sweep_->GetAutoPeakData(highData, lowData);
            buffer = std::move(highData);
            success = !buffer.empty();
        }
        break;
        default: break;
        }
        return success;
    }
#endif

    std::vector<PSCANCONFIG::FreqAmpData> Pscan::ProcessData(
        const std::vector<double>& dbData,
        PSCANCONFIG::DetectorType detectorType)
    {
        std::vector<PSCANCONFIG::FreqAmpData> result;


        if (dbData.empty()) return result;

        const uint32_t displayPoints = static_cast<uint32_t>(dbData.size());
        // Item 7: C# does NOT add CF_Offset to frequency axis
        double startFreq = config_.centerFreq - config_.span / 2.0;
        double spanStep = (config_.step > 0.0 && displayPoints > 1) ? config_.step : 0.0;

        uint32_t actualPoints = (displayPoints < (uint32_t)dbData.size()) ? displayPoints : (uint32_t)dbData.size();
        result.reserve(actualPoints);

        for (uint32_t i = 0; i < actualPoints; ++i)
        {
            double freq = startFreq + spanStep * i;
            double convertedAmp = ConvertAmplitude(dbData[i], config_.unitType);
            result.emplace_back(freq, convertedAmp);
        }
        if (!result.empty())
        {
            double maxAmp = result[0].amplitude;
            uint32_t peakIdx = 0;
            for (uint32_t i = 0; i < actualPoints; ++i)
            {
                if (result[i].amplitude > maxAmp)
                {
                    maxAmp = result[i].amplitude;
                    peakIdx = i;
                }
            }

            std::ostringstream oss;
            oss << "[PSCAN_PROCESS_FRAME]"
                << " configVersion=" << configVersion_.load()
                << " detector=" << static_cast<int>(detectorType)
                << " CF=" << config_.centerFreq
                << " Span=" << config_.span
                << " SetPoints=" << displayPoints
                << " readPoints=" << dbData.size()
                << " outputPoints=" << actualPoints
                << " firstFreq=" << result.front().frequency
                << " lastFreq=" << result.back().frequency
                << " peakIdx=" << peakIdx
                << " peakFreq=" << result[peakIdx].frequency
                << " peakOffsetHz=" << (result[peakIdx].frequency - config_.centerFreq)
                << " peakAmp=" << maxAmp
                << " peakRatio=" << (actualPoints > 1 ? (double)peakIdx / (double)(actualPoints - 1) : 0.0);
            BSLog(oss.str().c_str());
        }

        return result;
    }

    double Pscan::ConvertAmplitude(double ampDbm, PSCANCONFIG::UnitType targetUnit) const
    {
        switch (targetUnit)
        {
        case PSCANCONFIG::UnitType::dBm:  return ampDbm;
        case PSCANCONFIG::UnitType::dBmV: return ampDbm + 46.99;
        case PSCANCONFIG::UnitType::dBmuV: return ampDbm + 107.0;
        case PSCANCONFIG::UnitType::V:
        {
            double powerMw = std::pow(10.0, ampDbm / 10.0);
            return std::sqrt(powerMw / 1000.0 * 50.0) * 1000.0;
        }
        case PSCANCONFIG::UnitType::W:
            return std::pow(10.0, ampDbm / 10.0) / 1000.0;
        case PSCANCONFIG::UnitType::A:
        {
            double powerMw = std::pow(10.0, ampDbm / 10.0);
            return std::sqrt(powerMw / 1000.0 / 50.0) * 1000.0;
        }
        default: return ampDbm;
        }
    }

    void Pscan::UploadToUI(
        const std::vector<std::pair<PSCANCONFIG::DetectorType, std::vector<PSCANCONFIG::FreqAmpData>>>& dataPairs)
    {
        if (dataSink_)
        {
            dataSink_->OnDataArrived(dataPairs);
            if (!dataPairs.empty())
                dataSink_->OnSpectrogramDataArrived(dataPairs[0].second);
        }
    }
}
