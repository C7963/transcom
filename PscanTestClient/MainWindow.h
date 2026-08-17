#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QPointer>
#include "PscanDefs.h"
#include "PscanDriver.h"

class SpectrumWidget;
class QPushButton;
class QThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void OnStartClicked();
    void OnStopClicked();
    void OnSingleClicked();
    void OnConfigChanged();
    void OnRefreshTimer();
    void OnCursorMoved(double freq, double amp);
    void OnHardwareInitFinished(bool success);
    void OnInitWatchdogTimeout();

private:
    void SetupUI();
    QWidget* CreateParamGroup();
    QWidget* CreateControlGroup();
    QWidget* CreateStatusBar();
    void UpdatePscanConfig();
    void BeginHardwareInitialization(bool continuous);
    void BeginAcquisition(bool continuous);
    void SetInitializationControlsEnabled(bool enabled);

    SpectrumWidget* spectrumWidget_;
    QDoubleSpinBox* centerFreqSpin_;
    QDoubleSpinBox* spanSpin_;
    QSpinBox* rbwSpin_;
    QDoubleSpinBox* stepSpin_;
    QDoubleSpinBox* refLevelSpin_;
    QComboBox* unitCombo_;
    QComboBox* detectorCombo_;

    QLabel* modeLabel_;
    QLabel* statusLabel_;
    QLabel* cursorLabel_;
    QLabel* sweepCountLabel_;

    QPushButton* startButton_;
    QPushButton* singleButton_;
    QPushButton* stopButton_;

    QTimer* refreshTimer_;
    QTimer* initWatchdog_;
    QPointer<QThread> initThread_;
    PscanDriver driver_;
    PSCANCONFIG::PscanConfig config_;

    int sweepCount_;
    bool continuousMode_;
    bool driverInitialized_;
    bool hardwareConfigDirty_;
    bool initializationInProgress_;
    bool pendingContinuousStart_;
};

#endif // MAINWINDOW_H
