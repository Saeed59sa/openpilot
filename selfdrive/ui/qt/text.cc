#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QTimer>

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
  QHBoxLayout *ip_layout = new QHBoxLayout();
  QLabel *network_label = new QLabel();

  network_label->setStyleSheet("color: #e0e879");
  auto update_ip_address = [&]() {
    QString device_ip = "────────";
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
      if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost && address.toString().contains(".")) {
        device_ip = address.toString();
        break;
      }
    }
    network_label->setText(device_ip);
  };

  ip_layout->addWidget(network_label);
  ip_layout->setAlignment(Qt::AlignRight | Qt::AlignTop);

  main_layout->addLayout(ip_layout);

  QHBoxLayout *scroll_layout = new QHBoxLayout();

  QLabel *label = new QLabel(argv[1]);
  label->setWordWrap(true);
  label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  ScrollView *scroll = new ScrollView(label);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  scroll_layout->addWidget(scroll, 1);
  main_layout->addLayout(scroll_layout);

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

  btn2->setText(QObject::tr("Git Pull"));
  QObject::connect(btn2, &QPushButton::clicked, [=]() {
    QProcess::execute("sh /data/openpilot/scripts/gitpull.sh");
  });

  btn3->setText(QObject::tr("Git Reset -1"));
  QObject::connect(btn3, &QPushButton::clicked, [=]() {
    QProcess::execute("sh /data/openpilot/scripts/reset.sh");
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

  QTimer *timer = new QTimer(&a);
  QObject::connect(timer, &QTimer::timeout, update_ip_address);
  timer->start(5000);

  update_ip_address();

  window.setLayout(main_layout);
  window.show();

  return a.exec();
}
