#include "selfdrive/ui/qt/onroad/buttons.h"

#include <QPainter>

#include "selfdrive/ui/qt/util.h"

void drawIcon(QPainter &p, const QPoint &center, const QPixmap &img, const QBrush &bg, float opacity) {
  p.setRenderHint(QPainter::Antialiasing);
  p.setOpacity(1.0);  // bg dictates opacity of ellipse
  p.setPen(Qt::NoPen);
  p.setBrush(bg);
  p.drawEllipse(center, btn_size / 2, btn_size / 2);
  p.setOpacity(opacity);
  p.drawPixmap(center - QPoint(img.width() / 2, img.height() / 2), img);
  p.setOpacity(1.0);
}

void drawIconRotate(QPainter &p, const QPoint &center, const QPixmap &img, const QBrush &bg, float opacity, float angle) {
  // Draw background
  p.setRenderHint(QPainter::Antialiasing);
  p.setOpacity(1.0);  // bg dictates opacity of ellipse
  p.setPen(Qt::NoPen);
  p.setBrush(bg);
  p.drawEllipse(center, btn_size / 2, btn_size / 2);

  // Draw image with optional rotation
  p.setOpacity(opacity);
  p.save();
  p.translate(center);
  p.rotate(-angle);  // Rotate clockwise by `angle` degrees
  p.drawPixmap(-QPoint(img.width() / 2, img.height() / 2), img);
  p.restore();
  p.setOpacity(1.0);
}

void drawIconGradient(QPainter &p, const QPoint &center, const QPixmap &img, const QBrush &bg, float opacity, float angle) {
  // Draw background
  p.setRenderHint(QPainter::Antialiasing);
  p.setOpacity(1.0);  // bg dictates opacity of ellipse
  p.setPen(Qt::NoPen);
  p.setBrush(bg);
  p.drawEllipse(center, btn_size / 2, btn_size / 2);

  // Draw the arc (border) with gradient based on angle
  QConicalGradient gradient(center, 90);  // Start angle at 12 o'clock
  gradient.setColorAt(0.0, limeColor(200));  // Start color (lime)
  gradient.setColorAt(0.5, orangeColor(200)); // Middle color (orange)
  gradient.setColorAt(1.0, redColor(200));   // End color (red)

  // Adjust the radius to keep the border inside the background circle
  int borderThickness = 10;  // Border thickness
  int adjustedRadius = btn_size / 2 - borderThickness / 2;  // Reduce radius by half the border thickness

  p.setPen(QPen(QBrush(gradient), borderThickness));  // Set gradient border color and thickness (10px)
  p.setBrush(Qt::NoBrush);                            // No fill, only border
  p.drawArc(QRect(center.x() - adjustedRadius, center.y() - adjustedRadius, adjustedRadius * 2, adjustedRadius * 2), 90 * 16, -angle * 16);

  // Draw image with optional rotation
  p.setOpacity(opacity);
  p.save();
  p.translate(center);
  p.rotate(-angle);  // Rotate clockwise by `angle` degrees
  p.drawPixmap(-QPoint(img.width() / 2, img.height() / 2), img);
  p.restore();
  p.setOpacity(1.0);
}

// ExperimentalButton
ExperimentalButton::ExperimentalButton(QWidget *parent) : experimental_mode(false), engageable(false), QPushButton(parent) {
  setFixedSize(btn_size, btn_size);

  engage_img = loadPixmap("../assets/img_experimental_white.svg", {img_size, img_size});
  experimental_img = loadPixmap("../assets/img_experimental.svg", {img_size, img_size});
  QObject::connect(this, &QPushButton::clicked, this, &ExperimentalButton::changeMode);
}

void ExperimentalButton::changeMode() {
  const auto cp = (*uiState()->sm)["carParams"].getCarParams();
  bool can_change = hasLongitudinalControl(cp) && params.getBool("ExperimentalModeConfirmed");
  if (can_change) {
    params.putBool("ExperimentalMode", !experimental_mode);
  }
}

void ExperimentalButton::updateState(const UIState &s) {
  const auto cs = (*s.sm)["selfdriveState"].getSelfdriveState();
  bool eng = cs.getEngageable() || cs.getEnabled();
  if ((cs.getExperimentalMode() != experimental_mode) || (eng != engageable)) {
    engageable = eng;
    experimental_mode = cs.getExperimentalMode();
    update();
  }
}

void ExperimentalButton::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  QPixmap img = experimental_mode ? experimental_img : engage_img;
  drawIcon(p, QPoint(btn_size / 2, btn_size / 2), img, QColor(0, 0, 0, 100), 0.8);
}
