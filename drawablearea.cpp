#include <QPainter>
#include <QRect>

#include <QEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include "drawablearea.h"

DrawableArea::DrawableArea(QWidget* parent)
  : QWidget(parent),
    image(QImage(size(),QImage::Format_RGB32)),
    backgroundColor(qRgb(255, 255, 255)),
    userPenColor(QColor("green")),
    trackerPenColor1(QColor("red")),
    trackerPenColor2(QColor("blue"))
{
  image.fill(backgroundColor);
  setAttribute(Qt::WA_MouseTracking);
  setAttribute(Qt::WA_StaticContents);
  connect(&timer,SIGNAL(timeout()),this,SLOT(refreshTrack()));
  timer.setInterval(50);
  timer.start();
}

void DrawableArea::refreshTrack()
{
  //if (buttonPushed)
  {
    QPoint pos = QCursor::pos();
    {
      if (lastTrackPred.isNull()) // first tracked mouse click
      {
        lastTrackPred = tracker.initializeStartState(pos); // get estimated value (track)
      }

      std::pair<QPoint,QPoint> prediction = tracker.getTrackPosition(pos);

      QPainter painter(&image);
      painter.setPen(trackerPenColor1);
      painter.drawLine(lastTrackPred,prediction.first);
      if (!lastTrackCorr.isNull())
      {
        painter.setPen(trackerPenColor2);
        painter.drawLine(lastTrackCorr,prediction.second);
      }
      lastTrackPred = prediction.first;
      lastTrackCorr = prediction.second;
    }
    if (lastPoint.isNull())
      lastPoint = pos;

    QPainter painter(&image);
    painter.setPen(userPenColor);
    painter.drawLine(lastPoint,pos);
    painter.end();
    lastPoint = pos;
    update();

  }
}

void DrawableArea::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  const QRect rect = event->rect();
  painter.drawImage(rect.topLeft(), image, rect);
}

void DrawableArea::resizeEvent(QResizeEvent* event)
{
  if (width() > image.width() || height() > image.height())
  {
    int newWidth = qMax(width(), image.width());
    int newHeight = qMax(height(), image.height());
    resizeImage(QSize(newWidth, newHeight));
    update();
  }
  QWidget::resizeEvent(event);
}

bool DrawableArea::event(QEvent* event)
{
  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    {
      lastPoint = static_cast<QMouseEvent*>(event)->pos();
      QMouseEvent* ev = static_cast<QMouseEvent*>(event);
      if (ev->buttons() & Qt::RightButton) // if RMB clicked, clear image
      {
        lastPoint = QPoint();
        lastTrackPred = QPoint();
        lastTrackCorr = QPoint();
        image = QImage(image.size(),image.format());
        image.fill(backgroundColor);
        update();
      }
      break;
    }
    default:
      return QWidget::event(event);
  }
  return true;
}

void DrawableArea::resizeImage(const QSize& newSize)
{
  if (image.size() == newSize)
    return;

  QImage newImage(newSize, QImage::Format_RGB32);
  newImage.fill(backgroundColor);
  QPainter painter(&newImage);
  painter.drawImage(QPoint(0, 0), image);
  image = newImage;
}
