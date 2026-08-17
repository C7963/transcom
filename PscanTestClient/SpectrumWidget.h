#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QAction>
#include <QColor>
#include <QMenu>
#include <QPixmap>
#include <QVector>
#include <QWidget>
#include "PscanDefs.h"

class QContextMenuEvent;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;

class SpectrumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumWidget(QWidget* parent = nullptr);

    void SetSpectrumData(
        const QVector<PSCANCONFIG::FreqAmpData>& data,
        const QColor& color = QColor(0, 200, 255));

    void SetDisplayRange(double startFreq, double stopFreq, double refLevel, double scale);
    void SetUnit(PSCANCONFIG::UnitType unit);
    void AutoFitYRange(double minAmp, double maxAmp);

    void SetAutoYMode(bool enabled) { autoYMode_ = enabled; bgDirty_ = true; }
    bool IsAutoYMode() const { return autoYMode_; }

    void SetRefLevel(double refLevel);
    double GetRefLevel() const { return refLevel_; }
    double GetStartFreq() const { return startFreq_; }
    double GetStopFreq() const { return stopFreq_; }
    double GetScale() const { return scale_; }

    void ClearData();
    QString FormatFrequency(double freq) const;
    QString FormatAmplitude(double amp) const;

signals:
    void CursorMoved(double freq, double amp);
    void RefLevelChanged(double refLevel);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void DrawGrid(QPainter& painter);
    void DrawSpectrum(QPainter& painter);
    void DrawCursor(QPainter& painter);
    void DrawLabels(QPainter& painter);

    int FreqToX(double freq) const;
    int AmpToY(double amp) const;
    double XToFreq(int x) const;
    double YToAmp(int y) const;

    void ZoomYAxis(double factor, double centerAmp);
    void PanYAxis(double deltaAmp);
    void UpdateBackgroundCache();
    void CheckBgDirty();

    QVector<PSCANCONFIG::FreqAmpData> spectrumData_;
    QColor spectrumColor_;

    double startFreq_;
    double stopFreq_;
    double refLevel_;
    double scale_;
    PSCANCONFIG::UnitType unit_;
    bool autoYMode_;

    bool isDragging_;
    bool isRightDragging_;
    int dragStartX_;
    int dragStartY_;
    double dragStartRefLevel_;
    double dragStartScale_;
    double dragStartStartFreq_;
    double dragStartStopFreq_;

    QMenu* contextMenu_;
    int cursorX_;
    int cursorY_;

    static const int MARGIN_LEFT = 70;
    static const int MARGIN_RIGHT = 20;
    static const int MARGIN_TOP = 30;
    static const int MARGIN_BOTTOM = 50;

    QPixmap bgCache_;
    bool bgDirty_;
    QPixmap offscreen_;

    double lastStartFreq_;
    double lastStopFreq_;
    double lastRefLevel_;
    double lastScale_;
    PSCANCONFIG::UnitType lastUnit_;
    bool lastAutoYMode_;
};

#endif // SPECTRUMWIDGET_H
