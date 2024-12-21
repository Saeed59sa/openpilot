#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>

#include "system/hardware/hw.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

int main(int argc, char *argv[]) {
  initApp(argc, argv);
  QApplication a(argc, argv);
  QWidget window;
  setMainWindow(&window);

  QVBoxLayout *main_layout = new QVBoxLayout(&window);
  QHBoxLayout *top_layout = new QHBoxLayout();
  QLabel *label = new QLabel(argv[1]);
  label->setWordWrap(true);
  label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  ScrollView *scroll = new ScrollView(label);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  top_layout->addWidget(scroll, 1);

  QLabel *network_label = new QLabel();
  QString device_ip = "────────";
  const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
  for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
    if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
      device_ip = address.toString();
  }
  network_label->setText(device_ip);
  network_label->setStyleSheet("color: #e0e879");
  top_layout->addWidget(network_label, 0);
  main_layout->addLayout(top_layout);

  QHBoxLayout *button_layout = new QHBoxLayout();
  QPushButton *btn = new QPushButton();
  QPushButton *btn2 = new QPushButton();
  QPushButton *btn3 = new QPushButton();

#ifdef __aarch64__
  btn->setText(QObject::tr("Reboot"));
  QObject::connect(btn, &QPushButton::clicked, [=]() {
    Hardware::reboot();
  });
#else
  btn->setText(QObject::tr("Exit"));
  QObject::connect(btn, &QPushButton::clicked, &a, &QApplication::quit);
#endif

  btn2->setText(QObject::"Git Pull");
  QObject::connect(btn2, &QPushButton::clicked, [=]() {
    QProcess::execute("sh /data/openpilot/scripts/gitpull.sh");
    Hardware::reboot();
  });

  btn3->setText(QObject::tr("Git Reset -1"));
  QObject::connect(btn3, &QPushButton::clicked, [=]() {
    QProcess::execute("sh /data/openpilot/scripts/reset.sh");
    Hardware::reboot();
  });

  button_layout->addWidget(btn);
  button_layout->addWidget(btn2);
  button_layout->addWidget(btn3);
  button_layout->setSpacing(20);

  main_layout->addLayout(button_layout);

  window.setStyleSheet(R"(
    * {
      outline: none;
      color: white;
      background-color: black;
      font-size: 50px;
    }
    QPushButton {
      padding: 30px;
      padding-right: 30px;
      padding-left: 30px;
      border: 2px solid white;
      border-radius: 20px;
      margin-right: 30px;
    }
  )");

  return a.exec();
}
