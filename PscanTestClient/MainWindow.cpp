#include "MainWindow.h"
#include "SpectrumWidget.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QStatusBar>
#include <QThread>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

static void WriteDiagLog(const QString& msg)
{
    QFile log(QCoreApplication::applicationDirPath() + "/diagnostic.log");
    if (log.open(QIODevice::Append | QIODevice::Text))
    {
        log.write(QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toUtf8());
        log.write(" ");
        log.write(msg.toUtf8());
        log.write("\n");
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , spectrumWidget_(nullptr)
    , centerFreqSpin_(nullptr)
    , spanSpin_(nullptr)
    , rbwSpin_(nullptr)
    , stepSpin_(nullptr)
    , refLevelSpin_(nullptr)
    , unitCombo_(nullptr)
    , detectorCombo_(nullptr)
    , modeLabel_(nullptr)
    , statusLabel_(nullptr)
    , cursorLabel_(nullptr)
    , sweepCountLabel_(nullptr)
    , startButton_(nullptr)
    , singleButton_(nullptr)
    , stopButton_(nullptr)
    , refreshTimer_(nullptr)
    , initWatchdog_(nullptr)
    , sweepCount_(0)
    , continuousMode_(false)
    , driverInitialized_(false)
    , hardwareConfigDirty_(true)
    , initializationInProgress_(false)
    , pendingContinuousStart_(false)
{
    WriteDiagLog("[MW] ctor enter");
    config_.centerFreq = 921.6e6;
    config_.span = 20e6;
    config_.rbw = 1e3;
    config_.step = 200e3;
    config_.refLevel = -30.0;
    config_.unitType = PSCANCONFIG::UnitType::dBm;
    config_.traceModels.clear();
    config_.traceModels.emplace_back(
        PSCANCONFIG::TracesType::Trace1,
        PSCANCONFIG::DetectorType::RMS);
    config_.UpdateDetectorBitmask();

    WriteDiagLog("[MW] before SetupUI");
    SetupUI();
    WriteDiagLog("[MW] after SetupUI");
    setWindowTitle("PSCAN Test Client - Hardware");
    resize(1200, 800);

    refreshTimer_ = new QTimer(this);
    connect(refreshTimer_, &QTimer::timeout, this, &MainWindow::OnRefreshTimer);
    refreshTimer_->setInterval(100);

    initWatchdog_ = new QTimer(this);
    initWatchdog_->setSingleShot(true);
    connect(initWatchdog_, &QTimer::timeout, this, &MainWindow::OnInitWatchdogTimeout);

    WriteDiagLog("[MW] before Load BSPDriver.dll");
    if (driver_.Load(L"BSPDriver.dll"))
    {
        WriteDiagLog("[MW] Load BSPDriver.dll OK (hardware init deferred)");
        statusLabel_->setText("Status: Driver loaded");
        statusBar()->showMessage("Driver loaded; hardware init deferred until Start/Single");
    }
    else
    {
        WriteDiagLog("[MW] Load BSPDriver.dll FAILED");
        statusLabel_->setText("Status: Driver not loaded");
        statusBar()->showMessage("Warning: BSPDriver.dll load failed");
    }

    WriteDiagLog("[MW] before UpdatePscanConfig");
    UpdatePscanConfig();
    WriteDiagLog("[MW] ctor exit");
}

MainWindow::~MainWindow()
{
    if (initializationInProgress_)
    {
        WriteDiagLog("[MW] shutdown while hardware Init is still running; leaving BSPDriver.dll loaded until process exit");
        driver_.DetachModuleForInFlightCall();
        return;
    }

    if (driver_.IsLoaded())
    {
        driver_.Stop();
    }
}

void MainWindow::SetupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addWidget(CreateParamGroup());
    topLayout->addWidget(CreateControlGroup());
    mainLayout->addLayout(topLayout);

    spectrumWidget_ = new SpectrumWidget(this);
    connect(spectrumWidget_, &SpectrumWidget::CursorMoved, this, &MainWindow::OnCursorMoved);
    mainLayout->addWidget(spectrumWidget_, 1);

    setCentralWidget(centralWidget);
    CreateStatusBar();
}

QWidget* MainWindow::CreateParamGroup()
{
    QGroupBox* group = new QGroupBox("Scan Parameters", this);
    QGridLayout* layout = new QGridLayout(group);

    layout->addWidget(new QLabel("Center frequency:"), 0, 0);
    centerFreqSpin_ = new QDoubleSpinBox(this);
    centerFreqSpin_->setRange(1e6, 6e10);
    centerFreqSpin_->setValue(config_.centerFreq);
    centerFreqSpin_->setSuffix(" Hz");
    centerFreqSpin_->setDecimals(0);
    layout->addWidget(centerFreqSpin_, 0, 1);

    layout->addWidget(new QLabel("Span:"), 1, 0);
    spanSpin_ = new QDoubleSpinBox(this);
    spanSpin_->setRange(1e3, 6e10);
    spanSpin_->setValue(config_.span);
    spanSpin_->setSuffix(" Hz");
    spanSpin_->setDecimals(0);
    layout->addWidget(spanSpin_, 1, 1);

    layout->addWidget(new QLabel("RBW:"), 2, 0);
    rbwSpin_ = new QSpinBox(this);
    rbwSpin_->setRange(1, 100000);
    rbwSpin_->setValue(static_cast<int>(config_.rbw / 1000));
    rbwSpin_->setSuffix(" kHz");
    layout->addWidget(rbwSpin_, 2, 1);

    layout->addWidget(new QLabel("Step:"), 3, 0);
    stepSpin_ = new QDoubleSpinBox(this);
    stepSpin_->setRange(1, 1e8);
    stepSpin_->setValue(config_.step);
    stepSpin_->setSuffix(" Hz");
    stepSpin_->setDecimals(0);
    layout->addWidget(stepSpin_, 3, 1);

    layout->addWidget(new QLabel("Reference level:"), 4, 0);
    refLevelSpin_ = new QDoubleSpinBox(this);
    refLevelSpin_->setRange(-200, 100);
    refLevelSpin_->setValue(config_.refLevel);
    refLevelSpin_->setSuffix(" dBm");
    layout->addWidget(refLevelSpin_, 4, 1);

    layout->addWidget(new QLabel("Unit:"), 5, 0);
    unitCombo_ = new QComboBox(this);
    unitCombo_->addItems({"dBm", "dBmV", "dBuV", "V", "W", "A"});
    layout->addWidget(unitCombo_, 5, 1);

    layout->addWidget(new QLabel("Detector:"), 6, 0);
    detectorCombo_ = new QComboBox(this);
    detectorCombo_->addItem("RMS");
    detectorCombo_->setEnabled(false);
    layout->addWidget(detectorCombo_, 6, 1);

    connect(centerFreqSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { OnConfigChanged(); });
    connect(spanSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { OnConfigChanged(); });
    connect(rbwSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { OnConfigChanged(); });
    connect(stepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { OnConfigChanged(); });
    connect(refLevelSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { OnConfigChanged(); });
    connect(unitCombo_, &QComboBox::currentTextChanged, this, [this]() { OnConfigChanged(); });
    connect(detectorCombo_, &QComboBox::currentTextChanged, this, [this]() { OnConfigChanged(); });

    return group;
}

QWidget* MainWindow::CreateControlGroup()
{
    QGroupBox* group = new QGroupBox("Control", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    modeLabel_ = new QLabel("Current mode: Stop", this);
    layout->addWidget(modeLabel_);

    QHBoxLayout* btnLayout = new QHBoxLayout();

    startButton_ = new QPushButton("Start", this);
    startButton_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::OnStartClicked);
    btnLayout->addWidget(startButton_);

    singleButton_ = new QPushButton("Single", this);
    singleButton_->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    connect(singleButton_, &QPushButton::clicked, this, &MainWindow::OnSingleClicked);
    btnLayout->addWidget(singleButton_);

    stopButton_ = new QPushButton("Stop", this);
    stopButton_->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    connect(stopButton_, &QPushButton::clicked, this, &MainWindow::OnStopClicked);
    btnLayout->addWidget(stopButton_);

    layout->addLayout(btnLayout);

    sweepCountLabel_ = new QLabel("Sweep count: 0", this);
    layout->addWidget(sweepCountLabel_);

    return group;
}

QWidget* MainWindow::CreateStatusBar()
{
    cursorLabel_ = new QLabel("Cursor: -- Hz, -- dBm", this);
    statusBar()->addPermanentWidget(cursorLabel_);

    statusLabel_ = new QLabel("Status: Ready", this);
    statusBar()->addPermanentWidget(statusLabel_);

    return nullptr;
}

void MainWindow::SetInitializationControlsEnabled(bool enabled)
{
    if (startButton_) startButton_->setEnabled(enabled);
    if (singleButton_) singleButton_->setEnabled(enabled);
    if (centerFreqSpin_) centerFreqSpin_->setEnabled(enabled);
    if (spanSpin_) spanSpin_->setEnabled(enabled);
    if (rbwSpin_) rbwSpin_->setEnabled(enabled);
    if (stepSpin_) stepSpin_->setEnabled(enabled);
    if (refLevelSpin_) refLevelSpin_->setEnabled(enabled);
    if (unitCombo_) unitCombo_->setEnabled(enabled);
    if (detectorCombo_) detectorCombo_->setEnabled(false); // Active hardware path is fixed RMS.
}

void MainWindow::BeginHardwareInitialization(bool continuous)
{
    if (!driver_.IsLoaded())
    {
        QMessageBox::warning(this, "Error", "Driver is not loaded. Check BSPDriver.dll.");
        return;
    }

    if (driverInitialized_)
    {
        BeginAcquisition(continuous);
        return;
    }

    if (initializationInProgress_)
    {
        statusBar()->showMessage("Hardware initialization is already in progress...");
        return;
    }

    initializationInProgress_ = true;
    pendingContinuousStart_ = continuous;
    SetInitializationControlsEnabled(false);
    WriteDiagLog(QString("[MW] hardware Init begin in worker CF=%1 Span=%2 RBW=%3")
        .arg(config_.centerFreq, 0, 'f', 1).arg(config_.span, 0, 'f', 1).arg(config_.rbw));
    statusLabel_->setText("Status: Initializing hardware (background)");
    statusBar()->showMessage("Initializing hardware in background...");

    const PFN_PscanApi_Init initFunction = driver_.GetInitFunction();
    const double centerFreq = config_.centerFreq;
    const double span = config_.span;
    const uint32_t rbw = config_.rbw;
    QPointer<MainWindow> window(this);

    initThread_ = QThread::create([window, initFunction, centerFreq, span, rbw]()
    {
        const bool success = initFunction && initFunction(centerFreq, span, rbw) != 0;
        if (window)
        {
            QMetaObject::invokeMethod(window.data(), [window, success]()
            {
                if (window) window->OnHardwareInitFinished(success);
            }, Qt::QueuedConnection);
        }
    });
    connect(initThread_.data(), &QThread::finished, initThread_.data(), &QObject::deleteLater);
    initThread_->start();
    initWatchdog_->start(15000);
}

void MainWindow::OnStartClicked()
{
    BeginHardwareInitialization(true);
}

void MainWindow::OnStopClicked()
{
    refreshTimer_->stop();
    if (initializationInProgress_)
    {
        pendingContinuousStart_ = false;
        modeLabel_->setText("Current mode: Stop");
        statusLabel_->setText("Status: Init still running");
        statusBar()->showMessage("Initialization cannot be cancelled safely; acquisition will not start when it completes.");
        WriteDiagLog("[MW] Stop requested while Init is running; cleared pending acquisition");
        return;
    }

    if (driver_.IsLoaded())
    {
        driver_.Stop();
    }
    modeLabel_->setText("Current mode: Stop");
    statusLabel_->setText("Status: Stop");
    statusBar()->showMessage("Stopped");
}

void MainWindow::OnSingleClicked()
{
    BeginHardwareInitialization(false);
}

void MainWindow::OnHardwareInitFinished(bool success)
{
    initializationInProgress_ = false;
    if (initWatchdog_) initWatchdog_->stop();
    SetInitializationControlsEnabled(true);

    if (!success)
    {
        WriteDiagLog("[MW] hardware Init returned FAILED");
        statusLabel_->setText("Status: Init failed");
        statusBar()->showMessage("Hardware initialization failed. See bsdriver_diag.log.");
        QMessageBox::warning(this, "Error", "Hardware initialization failed. Check PCIe hardware/driver and bsdriver_diag.log.");
        return;
    }

    driverInitialized_ = true;
    WriteDiagLog("[MW] hardware Init returned OK");
    if (!driver_.SetConfig(config_.centerFreq, config_.span, config_.rbw,
        config_.step, config_.refLevel))
    {
        WriteDiagLog("[MW] complete Pscan configuration failed");
        statusLabel_->setText("Status: Parameter configuration failed");
        statusBar()->showMessage("Complete Pscan configuration failed.");
        return;
    }
    hardwareConfigDirty_ = false;
    BeginAcquisition(pendingContinuousStart_);
}

void MainWindow::OnInitWatchdogTimeout()
{
    if (!initializationInProgress_)
    {
        return;
    }

    WriteDiagLog("[MW] hardware Init has not returned after 15 seconds");
    statusLabel_->setText("Status: Waiting for hardware init");
    statusBar()->showMessage("Hardware initialization is taking more than 15 seconds. Check PCIe device/driver; the UI remains available.");
}

void MainWindow::BeginAcquisition(bool continuous)
{
    if (driverInitialized_ && hardwareConfigDirty_)
    {
        if (!driver_.SetConfig(config_.centerFreq, config_.span, config_.rbw,
            config_.step, config_.refLevel))
        {
            WriteDiagLog("[MW] deferred complete Pscan configuration failed");
            statusLabel_->setText("Status: Parameter configuration failed");
            statusBar()->showMessage("Complete Pscan configuration failed.");
            return;
        }
        hardwareConfigDirty_ = false;
        WriteDiagLog("[MW] deferred complete Pscan configuration applied once");
    }

    continuousMode_ = continuous;
    if (continuous)
    {
        if (!driver_.Start())
        {
            WriteDiagLog("[MW] PscanApi_Start FAILED");
            modeLabel_->setText("Current mode: Stop");
            statusLabel_->setText("Status: Start failed");
            statusBar()->showMessage("Continuous acquisition failed to start.");
            return;
        }

        WriteDiagLog("[MW] PscanApi_Start OK; Qt will consume cached hardware frames");
        refreshTimer_->start();
        modeLabel_->setText("Current mode: Continuous");
        statusLabel_->setText("Status: Continuous");
        statusBar()->showMessage("Continuous acquisition...");
        return;
    }

    driver_.RunSingle();
    refreshTimer_->start();
    modeLabel_->setText("Current mode: Single");
    statusLabel_->setText("Status: Single");
    statusBar()->showMessage("Single scan...");
}
void MainWindow::OnConfigChanged()
{
    UpdatePscanConfig();
}

void MainWindow::UpdatePscanConfig()
{
    if (!centerFreqSpin_ || !spanSpin_ || !rbwSpin_ || !stepSpin_ || !refLevelSpin_ || !unitCombo_ || !detectorCombo_ || !spectrumWidget_)
    {
        return;
    }

    config_.centerFreq = centerFreqSpin_->value();
    config_.span = spanSpin_->value();
    config_.rbw = static_cast<uint32_t>(rbwSpin_->value() * 1000);
    config_.step = stepSpin_->value();
    config_.refLevel = refLevelSpin_->value();

    const QString unitStr = unitCombo_->currentText();
    if (unitStr == "dBmV") config_.unitType = PSCANCONFIG::UnitType::dBmV;
    else if (unitStr == "dBuV") config_.unitType = PSCANCONFIG::UnitType::dBmuV;
    else if (unitStr == "V") config_.unitType = PSCANCONFIG::UnitType::V;
    else if (unitStr == "W") config_.unitType = PSCANCONFIG::UnitType::W;
    else if (unitStr == "A") config_.unitType = PSCANCONFIG::UnitType::A;
    else config_.unitType = PSCANCONFIG::UnitType::dBm;

    const QString detStr = detectorCombo_->currentText();
    PSCANCONFIG::DetectorType detType = PSCANCONFIG::DetectorType::PositivePeak;
    if (detStr == "NegativePeak") detType = PSCANCONFIG::DetectorType::NegativePeak;
    else if (detStr == "Average") detType = PSCANCONFIG::DetectorType::Average;
    else if (detStr == "RMS") detType = PSCANCONFIG::DetectorType::RMS;
    else if (detStr == "Sample") detType = PSCANCONFIG::DetectorType::Sample;

    config_.traceModels.clear();
    config_.traceModels.emplace_back(PSCANCONFIG::TracesType::Trace1, detType);
    config_.UpdateDetectorBitmask();

    const double startFreq = config_.centerFreq - config_.span / 2.0;
    const double stopFreq = config_.centerFreq + config_.span / 2.0;
    spectrumWidget_->SetDisplayRange(startFreq, stopFreq, config_.refLevel, 10.0);
    spectrumWidget_->SetUnit(config_.unitType);
    hardwareConfigDirty_ = true;
}

void MainWindow::OnRefreshTimer()
{
    if (!driver_.IsLoaded())
    {
        return;
    }

    static bool firstCall = true;
    if (firstCall)
    {
        firstCall = false;
        WriteDiagLog("=== Timer started ===");
        WriteDiagLog(QString("  driver loaded: %1").arg(driver_.IsLoaded()));
        WriteDiagLog(QString("  CF=%1 Hz, Span=%2 Hz, RBW=%3 Hz, Step=%4 Hz")
            .arg(config_.centerFreq).arg(config_.span).arg(config_.rbw).arg(config_.step));
        WriteDiagLog(QString("  traceModels count: %1").arg(config_.traceModels.size()));
        WriteDiagLog(QString("  Display: startFreq=%1 stopFreq=%2 refLevel=%3 scale=%4")
            .arg(spectrumWidget_->GetStartFreq(), 0, 'f', 1)
            .arg(spectrumWidget_->GetStopFreq(), 0, 'f', 1)
            .arg(spectrumWidget_->GetRefLevel(), 0, 'f', 1)
            .arg(spectrumWidget_->GetScale(), 0, 'f', 1));
    }

    std::vector<double> freqs;
    std::vector<double> amps;
    const bool gotData = driver_.GetSpectrumData(freqs, amps) && !freqs.empty() && freqs.size() == amps.size();

    if (gotData)
    {
        double peakFreq = freqs[0];
        double peakAmp = amps[0];
        double minAmp = amps[0];
        double maxAmp = amps[0];
        int peakIdx = 0;
        for (size_t i = 0; i < freqs.size(); ++i)
        {
            if (amps[i] > peakAmp)
            {
                peakAmp = amps[i];
                peakFreq = freqs[i];
                peakIdx = static_cast<int>(i);
            }
            minAmp = std::min(minAmp, amps[i]);
            maxAmp = std::max(maxAmp, amps[i]);
        }

        const double firstFreq = freqs.front();
        const double lastFreq = freqs.back();
        const double receivedSpan = lastFreq - firstFreq;
        const double centerFreqFromData = (firstFreq + lastFreq) / 2.0;
        const double spanError = receivedSpan - config_.span;
        const double centerFreqError = centerFreqFromData - config_.centerFreq;

        static int g_clientCount = 0;
        if (g_clientCount < 10 || g_clientCount % 20 == 0)
        {
            WriteDiagLog(QString("[CLIENT_DATA][%1] RECEIVED_DATA").arg(g_clientCount));
            WriteDiagLog(QString("  points=%1").arg(freqs.size()));
            WriteDiagLog(QString("  freq_range=[%1 -> %2] Hz  span_received=%3 Hz")
                .arg(firstFreq, 0, 'f', 1).arg(lastFreq, 0, 'f', 1).arg(receivedSpan, 0, 'f', 1));
            WriteDiagLog(QString("  config: centerFreq=%1 Hz, span=%2 Hz")
                .arg(config_.centerFreq, 0, 'f', 1).arg(config_.span, 0, 'f', 1));
            WriteDiagLog(QString("  ERROR: span_err=%1 Hz, centerFreq_err=%2 Hz")
                .arg(spanError, 0, 'f', 1).arg(centerFreqError, 0, 'f', 1));
            WriteDiagLog(QString("  peak[%1]=%2 Hz / %3 dBm  peak_ratio=%4")
                .arg(peakIdx).arg(peakFreq, 0, 'f', 1).arg(peakAmp, 0, 'f', 2)
                .arg((freqs.size() > 1) ? static_cast<double>(peakIdx) / static_cast<double>(freqs.size() - 1) : 0.0, 0, 'f', 4));
            WriteDiagLog(QString("  amp_range=[%1, %2]")
                .arg(minAmp, 0, 'f', 2).arg(maxAmp, 0, 'f', 2));
        }
        ++g_clientCount;

        QVector<PSCANCONFIG::FreqAmpData> data;
        data.reserve(static_cast<int>(freqs.size()));
        for (size_t i = 0; i < freqs.size(); ++i)
        {
            data.append(PSCANCONFIG::FreqAmpData(freqs[i], amps[i]));
        }

        static int g_plotInputFrameCount = 0;
        if (g_plotInputFrameCount < 10)
        {
            WriteDiagLog(QString("[QT_PLOT_INPUT][%1]").arg(g_plotInputFrameCount));
            WriteDiagLog(QString("  points=%1 display=[%2 -> %3] configCF=%4 configSpan=%5")
                .arg(data.size())
                .arg(spectrumWidget_->GetStartFreq(), 0, 'f', 1)
                .arg(spectrumWidget_->GetStopFreq(), 0, 'f', 1)
                .arg(config_.centerFreq, 0, 'f', 1)
                .arg(config_.span, 0, 'f', 1));
            WriteDiagLog(QString("  dataRange=[%1 -> %2] dataCenter=%3 peak[%4]=%5/%6 peakRatio=%7")
                .arg(firstFreq, 0, 'f', 1)
                .arg(lastFreq, 0, 'f', 1)
                .arg(centerFreqFromData, 0, 'f', 1)
                .arg(peakIdx)
                .arg(peakFreq, 0, 'f', 1)
                .arg(peakAmp, 0, 'f', 2)
                .arg((data.size() > 1) ? static_cast<double>(peakIdx) / static_cast<double>(data.size() - 1) : 0.0, 0, 'f', 4));

            QString firstPoints = "  first5:";
            for (int i = 0; i < std::min<int>(5, data.size()); ++i)
            {
                firstPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteDiagLog(firstPoints);

            QString lastPoints = "  last5:";
            const int lastStart = std::max<int>(0, data.size() - 5);
            for (int i = lastStart; i < data.size(); ++i)
            {
                lastPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteDiagLog(lastPoints);

            const int peakStart = std::max<int>(0, peakIdx - 10);
            const int peakEnd = std::min<int>(data.size() - 1, peakIdx + 10);
            QString peakPoints = QString("  peakRegion[%1-%2]:").arg(peakStart).arg(peakEnd);
            for (int i = peakStart; i <= peakEnd; ++i)
            {
                peakPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteDiagLog(peakPoints);

            ++g_plotInputFrameCount;
        }

        if (spectrumWidget_->IsAutoYMode())
        {
            spectrumWidget_->AutoFitYRange(minAmp, maxAmp);
        }
        spectrumWidget_->SetSpectrumData(data);
        ++sweepCount_;
        sweepCountLabel_->setText(QString("Sweep count: %1 [HW %2 points]").arg(sweepCount_).arg(freqs.size()));
    }
    else
    {
        WriteDiagLog(QString("[NO_HW_DATA] #%1 GetSpectrumData returned false/empty").arg(sweepCount_));
        WriteDiagLog(QString("  config: CF=%1 Span=%2 RBW=%3 Step=%4 traceModels=%5")
            .arg(config_.centerFreq).arg(config_.span).arg(config_.rbw).arg(config_.step).arg(config_.traceModels.size()));
        statusLabel_->setText("Status: No hardware data");
        sweepCountLabel_->setText(QString("Sweep count: %1 [NO DATA]").arg(sweepCount_));
    }

    if (!continuousMode_ && driver_.GetRunMode() == 0)
    {
        refreshTimer_->stop();
        modeLabel_->setText("Current mode: Stop");
        statusLabel_->setText("Status: Stop");
        statusBar()->showMessage("Scan stopped");
    }
}

void MainWindow::OnCursorMoved(double freq, double amp)
{
    const QString freqStr = spectrumWidget_->FormatFrequency(freq);
    const QString ampStr = spectrumWidget_->FormatAmplitude(amp);
    cursorLabel_->setText(QString("Cursor: %1, %2").arg(freqStr, ampStr));
}
