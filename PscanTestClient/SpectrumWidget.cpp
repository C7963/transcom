#include "SpectrumWidget.h"

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QResizeEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

static void WriteWidgetDataLog(const QString& msg)
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

static QString SpectrumPointsLogPath()
{
    return QCoreApplication::applicationDirPath() + "/spectrum_points_10frames.log";
}

static void InitializeSpectrumPointsLog()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }

    initialized = true;
    QFile log(SpectrumPointsLogPath());
    if (log.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        log.write("# PSCAN Qt spectrum drawing data: settled frames 20-29\n");
        log.write(QString("# session_start=%1\n")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .toUtf8());
        log.write("# columns=index,frequency_hz,amplitude\n");
    }
}

static void WriteSpectrumPointsFrame(
    int frameIndex,
    const QVector<PSCANCONFIG::FreqAmpData>& data,
    double displayStart,
    double displayStop,
    double refLevel,
    double scale)
{
    QFile log(SpectrumPointsLogPath());
    if (!log.open(QIODevice::Append | QIODevice::Text))
    {
        return;
    }

    QByteArray output;
    output.reserve(256 + data.size() * 48);
    output.append(QString(
        "FRAME_BEGIN frame=%1 timestamp=%2 expected_points=%3 actual_points=%3 "
        "display_start_hz=%4 display_stop_hz=%5 ref_level=%6 scale=%7\n")
        .arg(frameIndex)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(data.size())
        .arg(displayStart, 0, 'g', 17)
        .arg(displayStop, 0, 'g', 17)
        .arg(refLevel, 0, 'g', 17)
        .arg(scale, 0, 'g', 17)
        .toUtf8());
    output.append("index,frequency_hz,amplitude\n");

    for (int i = 0; i < data.size(); ++i)
    {
        output.append(QString("%1,%2,%3\n")
            .arg(i)
            .arg(data[i].frequency, 0, 'g', 17)
            .arg(data[i].amplitude, 0, 'g', 17)
            .toUtf8());
    }

    output.append(QString("FRAME_END frame=%1 written_points=%2\n\n")
        .arg(frameIndex)
        .arg(data.size())
        .toUtf8());
    log.write(output);
}
SpectrumWidget::SpectrumWidget(QWidget* parent)
    : QWidget(parent)
    , spectrumColor_(QColor(0, 200, 255))
    , startFreq_(88e6)
    , stopFreq_(108e6)
    , refLevel_(0.0)
    , scale_(10.0)
    , unit_(PSCANCONFIG::UnitType::dBm)
    , autoYMode_(true)
    , isDragging_(false)
    , isRightDragging_(false)
    , dragStartX_(0)
    , dragStartY_(0)
    , dragStartRefLevel_(0)
    , dragStartScale_(0)
    , dragStartStartFreq_(0)
    , dragStartStopFreq_(0)
    , contextMenu_(nullptr)
    , cursorX_(-1)
    , cursorY_(-1)
    , bgDirty_(true)
    , lastStartFreq_(0)
    , lastStopFreq_(0)
    , lastRefLevel_(0)
    , lastScale_(0)
    , lastUnit_(PSCANCONFIG::UnitType::dBm)
    , lastAutoYMode_(true)
{
    InitializeSpectrumPointsLog();

    setMinimumSize(600, 400);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    contextMenu_ = new QMenu(this);
    QAction* autoYAction = contextMenu_->addAction("Auto Y Axis");
    autoYAction->setCheckable(true);
    autoYAction->setChecked(true);
    connect(autoYAction, &QAction::toggled, this, [this](bool checked) {
        SetAutoYMode(checked);
        if (checked && !spectrumData_.isEmpty())
        {
            double minAmp = spectrumData_.first().amplitude;
            double maxAmp = spectrumData_.first().amplitude;
            for (const auto& point : spectrumData_)
            {
                minAmp = std::min(minAmp, point.amplitude);
                maxAmp = std::max(maxAmp, point.amplitude);
            }
            AutoFitYRange(minAmp, maxAmp);
        }
        update();
    });

    contextMenu_->addSeparator();
    QAction* fitYAction = contextMenu_->addAction("Fit Y Range");
    connect(fitYAction, &QAction::triggered, this, [this, autoYAction]() {
        if (!spectrumData_.isEmpty())
        {
            double minAmp = spectrumData_.first().amplitude;
            double maxAmp = spectrumData_.first().amplitude;
            for (const auto& point : spectrumData_)
            {
                minAmp = std::min(minAmp, point.amplitude);
                maxAmp = std::max(maxAmp, point.amplitude);
            }
            SetAutoYMode(false);
            AutoFitYRange(minAmp, maxAmp);
            autoYAction->setChecked(false);
            update();
        }
    });
}

void SpectrumWidget::SetSpectrumData(const QVector<PSCANCONFIG::FreqAmpData>& data, const QColor& color)
{
    static int g_widgetFrameCount = 0;
    // Capture settled frames instead of startup FIFO contents.
    if (g_widgetFrameCount >= 20 && g_widgetFrameCount < 30)
    {
        WriteSpectrumPointsFrame(
            g_widgetFrameCount,
            data,
            startFreq_,
            stopFreq_,
            refLevel_,
            scale_);

        WriteWidgetDataLog(QString("[QT_WIDGET_DATA][%1]").arg(g_widgetFrameCount));
        WriteWidgetDataLog(QString("  points=%1 display=[%2 -> %3] refLevel=%4 scale=%5")
            .arg(data.size())
            .arg(startFreq_, 0, 'f', 1)
            .arg(stopFreq_, 0, 'f', 1)
            .arg(refLevel_, 0, 'f', 2)
            .arg(scale_, 0, 'f', 2));

        if (!data.isEmpty())
        {
            double minAmp = data[0].amplitude;
            double maxAmp = data[0].amplitude;
            double peakFreq = data[0].frequency;
            int peakIdx = 0;
            for (int i = 0; i < data.size(); ++i)
            {
                minAmp = std::min(minAmp, data[i].amplitude);
                if (data[i].amplitude > maxAmp)
                {
                    maxAmp = data[i].amplitude;
                    peakFreq = data[i].frequency;
                    peakIdx = i;
                }
            }

            const double dataCenter = (data.first().frequency + data.last().frequency) / 2.0;
            WriteWidgetDataLog(QString("  dataRange=[%1 -> %2] dataCenter=%3 peak[%4]=%5/%6 peakRatio=%7 ampRange=[%8,%9]")
                .arg(data.first().frequency, 0, 'f', 1)
                .arg(data.last().frequency, 0, 'f', 1)
                .arg(dataCenter, 0, 'f', 1)
                .arg(peakIdx)
                .arg(peakFreq, 0, 'f', 1)
                .arg(maxAmp, 0, 'f', 2)
                .arg((data.size() > 1) ? static_cast<double>(peakIdx) / static_cast<double>(data.size() - 1) : 0.0, 0, 'f', 4)
                .arg(minAmp, 0, 'f', 2)
                .arg(maxAmp, 0, 'f', 2));

            QString firstPoints = "  first5:";
            for (int i = 0; i < std::min<int>(5, data.size()); ++i)
            {
                firstPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteWidgetDataLog(firstPoints);

            QString lastPoints = "  last5:";
            const int lastStart = std::max<int>(0, data.size() - 5);
            for (int i = lastStart; i < data.size(); ++i)
            {
                lastPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteWidgetDataLog(lastPoints);

            const int peakStart = std::max<int>(0, peakIdx - 10);
            const int peakEnd = std::min<int>(data.size() - 1, peakIdx + 10);
            QString peakPoints = QString("  peakRegion[%1-%2]:").arg(peakStart).arg(peakEnd);
            for (int i = peakStart; i <= peakEnd; ++i)
            {
                peakPoints += QString(" [%1]=%2/%3").arg(i).arg(data[i].frequency, 0, 'f', 1).arg(data[i].amplitude, 0, 'f', 2);
            }
            WriteWidgetDataLog(peakPoints);
        }

    }

    ++g_widgetFrameCount;

    spectrumData_ = data;
    spectrumColor_ = color;
    update();
}

void SpectrumWidget::SetDisplayRange(double startFreq, double stopFreq, double refLevel, double scale)
{
    startFreq_ = startFreq;
    stopFreq_ = stopFreq;
    refLevel_ = refLevel;
    scale_ = scale;
    bgDirty_ = true;
    update();
}

void SpectrumWidget::SetUnit(PSCANCONFIG::UnitType unit)
{
    unit_ = unit;
    bgDirty_ = true;
    update();
}

void SpectrumWidget::SetRefLevel(double refLevel)
{
    refLevel_ = refLevel;
    bgDirty_ = true;
    emit RefLevelChanged(refLevel);
    update();
}

void SpectrumWidget::AutoFitYRange(double minAmp, double maxAmp)
{
    if (maxAmp <= minAmp)
    {
        maxAmp = minAmp + 10.0;
    }

    const double range = maxAmp - minAmp;
    double margin = range * 0.1;
    if (margin < 1.0)
    {
        margin = 1.0;
    }

    refLevel_ = maxAmp + margin;
    const double bottom = minAmp - margin;
    scale_ = (refLevel_ - bottom) / 8.0;
    if (scale_ < 0.1)
    {
        scale_ = 0.1;
    }

    bgDirty_ = true;
    update();
}

void SpectrumWidget::ClearData()
{
    spectrumData_.clear();
    bgDirty_ = true;
    update();
}

QString SpectrumWidget::FormatFrequency(double freq) const
{
    const double absFreq = std::abs(freq);
    if (absFreq >= 1e9) return QString::number(freq / 1e9, 'f', 3) + " GHz";
    if (absFreq >= 1e6) return QString::number(freq / 1e6, 'f', 3) + " MHz";
    if (absFreq >= 1e3) return QString::number(freq / 1e3, 'f', 3) + " kHz";
    return QString::number(freq, 'f', 0) + " Hz";
}

QString SpectrumWidget::FormatAmplitude(double amp) const
{
    QString unitStr;
    switch (unit_)
    {
    case PSCANCONFIG::UnitType::dBm: unitStr = "dBm"; break;
    case PSCANCONFIG::UnitType::dBmV: unitStr = "dBmV"; break;
    case PSCANCONFIG::UnitType::dBmuV: unitStr = "dBuV"; break;
    case PSCANCONFIG::UnitType::V: unitStr = "V"; break;
    case PSCANCONFIG::UnitType::W: unitStr = "W"; break;
    case PSCANCONFIG::UnitType::A: unitStr = "A"; break;
    default: unitStr = "dBm"; break;
    }

    if (unit_ == PSCANCONFIG::UnitType::dBm ||
        unit_ == PSCANCONFIG::UnitType::dBmV ||
        unit_ == PSCANCONFIG::UnitType::dBmuV)
    {
        return QString::number(amp, 'f', 2) + " " + unitStr;
    }

    return QString::number(amp, 'g', 4) + " " + unitStr;
}

void SpectrumWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    const QSize sz = size();
    if (offscreen_.size() != sz)
    {
        offscreen_ = QPixmap(sz);
        bgDirty_ = true;
    }

    CheckBgDirty();
    if (bgDirty_)
    {
        UpdateBackgroundCache();
        bgDirty_ = false;
    }

    {
        QPainter p(&offscreen_);
        p.drawPixmap(0, 0, bgCache_);
        if (!spectrumData_.isEmpty())
        {
            DrawSpectrum(p);
        }
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, offscreen_);
}

void SpectrumWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    bgDirty_ = true;
}

void SpectrumWidget::mouseMoveEvent(QMouseEvent* event)
{
    const int oldX = cursorX_;
    const int oldY = cursorY_;
    cursorX_ = static_cast<int>(event->position().x());
    cursorY_ = static_cast<int>(event->position().y());

    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;

    if (isRightDragging_ && plotHeight > 0)
    {
        const int dy = cursorY_ - dragStartY_;
        const double deltaAmp = -dy * scale_ * 8.0 / plotHeight;
        PanYAxis(deltaAmp);
        bgDirty_ = true;
        update();
        return;
    }

    if (isDragging_ && plotWidth > 0)
    {
        const int dx = cursorX_ - dragStartX_;
        const double oldSpan = dragStartStopFreq_ - dragStartStartFreq_;
        const double deltaFreq = -dx * oldSpan / plotWidth;
        startFreq_ = dragStartStartFreq_ + deltaFreq;
        stopFreq_ = dragStartStopFreq_ + deltaFreq;
        bgDirty_ = true;
        update();
        return;
    }

    if (cursorX_ >= MARGIN_LEFT && cursorX_ <= MARGIN_LEFT + plotWidth &&
        cursorY_ >= MARGIN_TOP && cursorY_ <= MARGIN_TOP + plotHeight)
    {
        emit CursorMoved(XToFreq(cursorX_), YToAmp(cursorY_));
    }

    if (std::abs(cursorX_ - oldX) >= 2 || std::abs(cursorY_ - oldY) >= 2)
    {
        bgDirty_ = true;
        update();
    }
}

void SpectrumWidget::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        const double factor = event->angleDelta().y() > 0 ? 0.9 : 1.1;
        const double centerAmp = YToAmp(static_cast<int>(event->position().y()));
        ZoomYAxis(factor, centerAmp);
        SetAutoYMode(false);
        bgDirty_ = true;
        update();
        return;
    }

    const double factor = event->angleDelta().y() > 0 ? 0.9 : 1.1;
    double centerFreq = (startFreq_ + stopFreq_) / 2.0;
    if (!(event->modifiers() & Qt::ShiftModifier))
    {
        centerFreq = XToFreq(static_cast<int>(event->position().x()));
    }

    const double oldSpan = stopFreq_ - startFreq_;
    double newSpan = oldSpan * factor;
    if (newSpan < 1.0)
    {
        newSpan = 1.0;
    }

    if (event->modifiers() & Qt::ShiftModifier)
    {
        startFreq_ = centerFreq - newSpan / 2.0;
        stopFreq_ = centerFreq + newSpan / 2.0;
    }
    else
    {
        const double ratio = (centerFreq - startFreq_) / oldSpan;
        startFreq_ = centerFreq - ratio * newSpan;
        stopFreq_ = startFreq_ + newSpan;
    }

    bgDirty_ = true;
    update();
}

void SpectrumWidget::mousePressEvent(QMouseEvent* event)
{
    const QPointF pos = event->position();
    if (event->button() == Qt::LeftButton)
    {
        isDragging_ = true;
        dragStartX_ = static_cast<int>(pos.x());
        dragStartY_ = static_cast<int>(pos.y());
        dragStartStartFreq_ = startFreq_;
        dragStartStopFreq_ = stopFreq_;
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->button() == Qt::RightButton)
    {
        isRightDragging_ = true;
        dragStartX_ = static_cast<int>(pos.x());
        dragStartY_ = static_cast<int>(pos.y());
        dragStartRefLevel_ = refLevel_;
        dragStartScale_ = scale_;
        setCursor(Qt::SizeVerCursor);
    }

    QWidget::mousePressEvent(event);
}

void SpectrumWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isDragging_)
    {
        isDragging_ = false;
        unsetCursor();
    }
    else if (event->button() == Qt::RightButton && isRightDragging_)
    {
        isRightDragging_ = false;
        SetAutoYMode(false);
        emit RefLevelChanged(refLevel_);
        unsetCursor();
    }

    QWidget::mouseReleaseEvent(event);
}

void SpectrumWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!contextMenu_->actions().isEmpty())
    {
        contextMenu_->actions().first()->setChecked(autoYMode_);
    }
    contextMenu_->exec(event->globalPos());
    event->accept();
}

void SpectrumWidget::ZoomYAxis(double factor, double centerAmp)
{
    double newScale = scale_ * factor;
    if (newScale < 0.01) newScale = 0.01;
    if (newScale > 100000.0) newScale = 100000.0;

    const double topAmp = refLevel_;
    const double bottomAmp = refLevel_ - scale_ * 8.0;
    const double newRange = newScale * 8.0;
    const double ratio = (topAmp - centerAmp) / (topAmp - bottomAmp);

    refLevel_ = centerAmp + ratio * newRange;
    scale_ = newScale;
}

void SpectrumWidget::PanYAxis(double deltaAmp)
{
    refLevel_ += deltaAmp;
}

void SpectrumWidget::DrawGrid(QPainter& painter)
{
    painter.setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));

    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    const int numDivX = 10;
    const int numDivY = 8;

    for (int i = 0; i <= numDivX; ++i)
    {
        const int x = MARGIN_LEFT + (plotWidth * i) / numDivX;
        painter.drawLine(x, MARGIN_TOP, x, MARGIN_TOP + plotHeight);
    }

    for (int i = 0; i <= numDivY; ++i)
    {
        const int y = MARGIN_TOP + (plotHeight * i) / numDivY;
        painter.drawLine(MARGIN_LEFT, y, MARGIN_LEFT + plotWidth, y);
    }
}

void SpectrumWidget::DrawSpectrum(QPainter& painter)
{
    if (spectrumData_.size() < 2)
    {
        return;
    }

    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;

    QPen pen(spectrumColor_, 2);
    painter.setPen(pen);

    int prevX = -1;
    int prevY = -1;
    bool prevValid = false;

    for (const auto& point : spectrumData_)
    {
        if (std::isnan(point.amplitude) || point.amplitude < -150.0)
        {
            prevValid = false;
            continue;
        }

        const int x = FreqToX(point.frequency);
        int y = AmpToY(point.amplitude);
        if (x < MARGIN_LEFT || x > MARGIN_LEFT + plotWidth)
        {
            prevValid = false;
            continue;
        }

        y = std::max(MARGIN_TOP, std::min(y, MARGIN_TOP + plotHeight));
        if (prevValid)
        {
            painter.drawLine(prevX, prevY, x, y);
        }
        else
        {
            painter.drawPoint(x, y);
        }

        prevX = x;
        prevY = y;
        prevValid = true;
    }

    if (!prevValid)
    {
        return;
    }

    QColor fillColor = spectrumColor_;
    fillColor.setAlpha(25);
    QVector<QPoint> points;
    points.reserve(spectrumData_.size());
    int leftX = MARGIN_LEFT + plotWidth;
    int rightX = MARGIN_LEFT;

    for (const auto& point : spectrumData_)
    {
        if (std::isnan(point.amplitude) || point.amplitude < -150.0)
        {
            continue;
        }

        const int x = FreqToX(point.frequency);
        if (x >= MARGIN_LEFT && x <= MARGIN_LEFT + plotWidth)
        {
            int y = AmpToY(point.amplitude);
            y = std::max(MARGIN_TOP, std::min(y, MARGIN_TOP + plotHeight));
            points.append(QPoint(x, y));
            leftX = std::min(leftX, x);
            rightX = std::max(rightX, x);
        }
    }

    if (points.size() >= 2 && leftX <= rightX)
    {
        QPolygon poly;
        poly.reserve(points.size() + 2);
        poly.append(QPoint(leftX, MARGIN_TOP + plotHeight));
        for (int i = 0; i < points.size(); ++i)
        {
            poly.append(points[i]);
        }
        poly.append(QPoint(rightX, MARGIN_TOP + plotHeight));
        painter.setPen(Qt::NoPen);
        painter.setBrush(fillColor);
        painter.drawPolygon(poly);
        painter.setPen(pen);
    }
}

void SpectrumWidget::DrawCursor(QPainter& painter)
{
    if (cursorX_ < MARGIN_LEFT || cursorX_ > width() - MARGIN_RIGHT || cursorY_ < MARGIN_TOP || cursorY_ > height() - MARGIN_BOTTOM)
    {
        return;
    }

    painter.setPen(QPen(QColor(255, 255, 0, 150), 1, Qt::DashLine));
    painter.drawLine(cursorX_, MARGIN_TOP, cursorX_, height() - MARGIN_BOTTOM);

    painter.setPen(QPen(Qt::yellow, 1));
    painter.drawLine(cursorX_ - 10, cursorY_, cursorX_ + 10, cursorY_);
    painter.drawLine(cursorX_, cursorY_ - 10, cursorX_, cursorY_ + 10);
}

void SpectrumWidget::DrawLabels(QPainter& painter)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9));

    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    const int numDivX = 10;
    const int numDivY = 8;

    for (int i = 0; i <= numDivX; i += 2)
    {
        const int x = MARGIN_LEFT + (plotWidth * i) / numDivX;
        const double freq = startFreq_ + (stopFreq_ - startFreq_) * i / numDivX;
        const QString label = FormatFrequency(freq);
        const QFontMetrics fm(painter.font());
        const int textWidth = fm.horizontalAdvance(label);
        painter.drawText(x - textWidth / 2, height() - MARGIN_BOTTOM + 20, label);
    }

    for (int i = 0; i <= numDivY; ++i)
    {
        const int y = MARGIN_TOP + (plotHeight * i) / numDivY;
        const double amp = refLevel_ - scale_ * i;
        painter.drawText(5, y + 4, FormatAmplitude(amp));
    }

    painter.setFont(QFont("Arial", 11, QFont::Bold));
    painter.drawText(MARGIN_LEFT, 20, "Spectrum Analysis");
}

int SpectrumWidget::FreqToX(double freq) const
{
    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    const double span = stopFreq_ - startFreq_;
    if (std::abs(span) < 1e-9)
    {
        return MARGIN_LEFT + plotWidth / 2;
    }
    const double ratio = (freq - startFreq_) / span;
    return MARGIN_LEFT + static_cast<int>(ratio * plotWidth);
}

int SpectrumWidget::AmpToY(double amp) const
{
    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    const double denominator = scale_ * 8.0;
    if (std::abs(denominator) < 1e-9)
    {
        return MARGIN_TOP + plotHeight / 2;
    }
    const double ratio = (refLevel_ - amp) / denominator;
    return MARGIN_TOP + static_cast<int>(ratio * plotHeight);
}

double SpectrumWidget::XToFreq(int x) const
{
    const int plotWidth = width() - MARGIN_LEFT - MARGIN_RIGHT;
    if (plotWidth <= 0)
    {
        return (startFreq_ + stopFreq_) / 2.0;
    }
    const double ratio = static_cast<double>(x - MARGIN_LEFT) / plotWidth;
    return startFreq_ + ratio * (stopFreq_ - startFreq_);
}

double SpectrumWidget::YToAmp(int y) const
{
    const int plotHeight = height() - MARGIN_TOP - MARGIN_BOTTOM;
    if (plotHeight <= 0)
    {
        return refLevel_;
    }
    const double ratio = static_cast<double>(y - MARGIN_TOP) / plotHeight;
    return refLevel_ - ratio * scale_ * 8.0;
}

void SpectrumWidget::CheckBgDirty()
{
    if (startFreq_ != lastStartFreq_ ||
        stopFreq_ != lastStopFreq_ ||
        refLevel_ != lastRefLevel_ ||
        scale_ != lastScale_ ||
        unit_ != lastUnit_ ||
        autoYMode_ != lastAutoYMode_)
    {
        bgDirty_ = true;
    }
}

void SpectrumWidget::UpdateBackgroundCache()
{
    const QSize sz = size();
    if (bgCache_.size() != sz)
    {
        bgCache_ = QPixmap(sz);
    }

    QPainter p(&bgCache_);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(20, 20, 30));

    DrawGrid(p);
    DrawCursor(p);
    DrawLabels(p);

    if (spectrumData_.isEmpty())
    {
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, "Waiting for data...");
    }

    p.setPen(autoYMode_ ? QColor(100, 255, 100) : QColor(255, 200, 100));
    p.setFont(QFont("Arial", 8));
    const QString modeText = autoYMode_ ? "[Auto Y]" : "[Manual Y]";
    p.drawText(width() - 90, 15, modeText);

    lastStartFreq_ = startFreq_;
    lastStopFreq_ = stopFreq_;
    lastRefLevel_ = refLevel_;
    lastScale_ = scale_;
    lastUnit_ = unit_;
    lastAutoYMode_ = autoYMode_;
}
