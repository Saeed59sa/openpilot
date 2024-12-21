#include <QComboBox>
#include <QAbstractItemView>
#include <QScroller>
#include <QListView>
#include <QListWidget>
#include <QProcess>

#include "common/watchdog.h"
#include "common/util.h"

#include "selfdrive/ui/qt/offroad/community_panel.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

static QStringList get_list(const char* path) {
  QStringList stringList;
  QFile textFile(path);
  if (textFile.open(QIODevice::ReadOnly)) {
    QTextStream textStream(&textFile);
    while (true) {
      QString line = textStream.readLine();
      if (line.isNull()) {
        break;
      } else {
        stringList.append(line);
      }
    }
  }
  return stringList;
}

// Community Panel
CommunityPanel::CommunityPanel(QWidget* parent) : QWidget(parent) {
  main_layout = new QStackedLayout(this);
  homeScreen = new QWidget(this);
  QVBoxLayout* communityLayout = new QVBoxLayout(homeScreen);

  // selectedManufacturer
  QString selectedManufacturer = QString::fromStdString(Params().get("SelectedManufacturer"));
  QPushButton* selectManufacturer_btn = new QPushButton(selectedManufacturer.length() ? selectedManufacturer : tr("Select your Manufacturer"));
  selectManufacturer_btn->setObjectName("selectManufacturer_btn");
  blueButtonStyle(selectManufacturer_btn);

  connect(selectManufacturer_btn, &QPushButton::clicked, [=]() {
    QStringList manufacturers = {"[ Not Selected ]", "HYUNDAI", "KIA", "GENESIS"};
    QString selectedOption = MultiOptionDialog::getSelection(tr("Select your Manufacturer"), manufacturers,
                                                            selectedManufacturer.isEmpty() ? manufacturers[0] : selectedManufacturer,
                                                            this);
    if (!selectedOption.isEmpty()) {
      if (selectedOption == "[ Not Selected ]") {
        Params().remove("SelectedManufacturer");
        qApp->exit(18);
        watchdog_kick(0);
      } else {
        QString carListFile;
        if (selectedOption == "HYUNDAI") {
          carListFile = "/data/params/crwusiz/CarList_Hyundai";
          qApp->exit(18);
          watchdog_kick(0);
        } else if (selectedOption == "KIA") {
          carListFile = "/data/params/crwusiz/CarList_Kia";
          qApp->exit(18);
          watchdog_kick(0);
        } else if (selectedOption == "GENESIS") {
          carListFile = "/data/params/crwusiz/CarList_Genesis";
          qApp->exit(18);
          watchdog_kick(0);
        }

        if (!carListFile.isEmpty()) {
          QProcess::execute("cp -f " + carListFile + " /data/params/crwusiz/CarList");
        }

        Params().put("SelectedManufacturer", selectedOption.toStdString());
        ConfirmationDialog::alert(selectedOption, this);
      }
    }
    selectManufacturer_btn->setText(selectedManufacturer.length() ? selectedManufacturer : tr("Select your Manufacturer"));
  });

  // selectedCar
  QString selectedCar = QString::fromStdString(Params().get("SelectedCar"));
  QPushButton* selectCar_btn = new QPushButton(selectedCar.length() ? selectedCar : tr("Select your car"));
  selectCar_btn->setObjectName("selectCar_btn");
  blueButtonStyle(selectCar_btn);

  connect(selectCar_btn, &QPushButton::clicked, [=]() {
    QStringList cars = {"[ Not Selected ]"};
    QStringList items = get_list("/data/params/crwusiz/CarList");
    cars.append(items);
    QString selectedOption = MultiOptionDialog::getSelection(tr("Select your car"), cars,
                                                            selectedCar.isEmpty() ? cars[0] : selectedCar,
                                                            this);
    if (!selectedOption.isEmpty()) {
      if (selectedOption == "[ Not Selected ]") {
        Params().remove("SelectedCar");
        qApp->exit(18);
        watchdog_kick(0);
      } else {
        Params().put("SelectedCar", selectedOption.toStdString());
        qApp->exit(18);
        watchdog_kick(0);
        ConfirmationDialog::alert(selectedOption, this);
      }
    }
    selectCar_btn->setText(selectedCar.length() ? selectedCar : tr("Select your car"));
  });

  // selectedBranch
  QString selectedBranch = QString::fromStdString(Params().get("SelectedBranch"));
  QPushButton* selectBranch_btn = new QPushButton(selectedBranch.length() ? selectedBranch : tr("Select Branch"));
  selectBranch_btn->setObjectName("selectBranch_btn");
  blueButtonStyle(selectBranch_btn);

  connect(selectBranch_btn, &QPushButton::clicked, [=]() {
    QStringList branches = {"[ Not Selected ]"};
    QStringList items = get_list("/data/params/crwusiz/GitBranchList");
    branches.append(items);
    QString selectedOption = MultiOptionDialog::getSelection(tr("Select Branch"), branches,
                                                            selectedBranch.isEmpty() ? branches[0] : selectedBranch,
                                                            this);
    if (!selectedOption.isEmpty()) {
      if (selectedOption == "[ Not Selected ]") {
        Params().remove("SelectedBranch");
        qApp->exit(18);
        watchdog_kick(0);
      } else {
        Params().put("SelectedBranch", selectedOption.toStdString());
        qApp->exit(18);
        watchdog_kick(0);
        ConfirmationDialog::alert(selectedOption, this);
      }
    }
    selectBranch_btn->setText(selectedBranch.length() ? selectedBranch : tr("Select Branch"));
  });

  QPushButton* gitpull_btn = new QPushButton("Git Pull");
  gitpull_btn->setObjectName("gitpull_btn");
  QObject::connect(gitpull_btn, &QPushButton::clicked, this, [this]() {
    if (ConfirmationDialog::confirm(tr("Git Fetch and Reset<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/gitpull.sh");
    }
    const QString file_path = "/data/check_network.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    }
  });

  QPushButton* toggle_btn = new QPushButton(tr("Toggle"));
  toggle_btn->setObjectName("toggle_btn");
  QObject::connect(toggle_btn, &QPushButton::clicked, this, [this]() {
    this->currentCommunityIndex = 0;
    this->togglesCommunity(0);
    updateButtonStyles();
  });

  QPushButton* func_btn = new QPushButton(tr("Function"));
  func_btn->setObjectName("func_btn");
  QObject::connect(func_btn, &QPushButton::clicked, this, [this]() {
    this->currentCommunityIndex = 1;
    this->togglesCommunity(1);
    updateButtonStyles();
  });

  QPushButton* upload_btn = new QPushButton(tr("Upload"));
  upload_btn->setObjectName("upload_btn");
  QObject::connect(upload_btn, &QPushButton::clicked, this, [this]() {
    this->currentCommunityIndex = 2;
    this->togglesCommunity(2);
    updateButtonStyles();
  });

  updateButtonStyles();

  QHBoxLayout* row1Layout = new QHBoxLayout();
  row1Layout->setSpacing(30);
  row1Layout->addWidget(selectManufacturer_btn);
  row1Layout->addWidget(selectCar_btn);
  row1Layout->addWidget(selectBranch_btn);

  QHBoxLayout* row2Layout = new QHBoxLayout();
  row2Layout->setSpacing(30);
  row2Layout->addWidget(gitpull_btn);
  row2Layout->addWidget(toggle_btn);
  row2Layout->addWidget(func_btn);
  row2Layout->addWidget(upload_btn);

  QVBoxLayout* mainLayout = new QVBoxLayout();
  mainLayout->setSpacing(30);
  mainLayout->addLayout(row1Layout);
  mainLayout->addLayout(row2Layout);

  communityLayout->addLayout(mainLayout, 0);

  QWidget* toggles = new QWidget();
  QVBoxLayout* toggles_layout = new QVBoxLayout(toggles);

  // main toggle
  mainToggles = new ListWidget(this);
  mainToggles->addItem(new ParamControl("PrebuiltEnable", tr("Prebuilt Enable"), tr("Create prebuilt file to speed bootup"),
                                        "../assets/offroad/icon_addon.png", this));
  mainToggles->addItem(new ParamControl("LoggerEnable", tr("Logger Enable"), tr("Turn off this option to reduce system load"),
                                        "../assets/offroad/icon_addon.png", this));
  mainToggles->addItem(new ParamControl("IsHda2", tr("CANFD Car HDA2"), tr("Highway Drive Assist 2, turn it on."),
                                        "../assets/offroad/icon_long.png", this));
  mainToggles->addItem(new ParamControl("HyundaiCameraSCC", tr("HDA2 ADAS ECAN Modify"), tr("Connect the ADAS ECAN line to CAMERA"),
                                        "../assets/offroad/icon_long.png", this));
  mainToggles->addItem(new ParamControl("DriverCameraHardwareMissing", tr("DriverCamera Hardware Missing"), tr("If there is a problem with the driver camera hardware, drive without the driver camera."),
                                        "../assets/img_driver_face_static_x.png", this));
  mainToggles->addItem(new ParamControl("DriverCameraOnReverse", tr("Driver Camera On Reverse"), tr("Displays the driver camera when in reverse."),
                                        "../assets/img_driver_face_static.png", this));
  mainToggles->addItem(new ParamControl("RadarTrackEnable", tr("Enable Radar Track use"), tr("Enable Radar Track use (disable AEB)"),
                                        "../assets/offroad/icon_warning.png", this));

  // func
  auto cleardtc_btn = new ButtonControl(tr("Clear DTC"), tr("RUN"));
  QObject::connect(cleardtc_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Clear DTC<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/cleardtc.sh");
    }
  });

  auto gitcheckout_btn = new ButtonControl("Git Checkout", tr("RUN"));
  QObject::connect(gitcheckout_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Git Checkout<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/checkout.sh");
    }
  });

  auto gitreset_btn = new ButtonControl(tr("Git Reset -1"), tr("RUN"));
  QObject::connect(gitreset_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Git Reset<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/reset.sh");
    }
  });

  auto can_missing_error_log_btn = new ButtonControl("can missing log", tr("VIEW"));
  QObject::connect(can_missing_error_log_btn, &ButtonControl::clicked, [=]() {
    const QString file_path = "/data/can_missing.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto can_timeout_error_log_btn = new ButtonControl("can timeout log", tr("VIEW"));
  QObject::connect(can_timeout_error_log_btn, &ButtonControl::clicked, [=]() {
    const QString file_path = "/data/can_timeout.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto tmux_error_log_btn = new ButtonControl(tr("tmux log"), tr("VIEW"));
  QObject::connect(tmux_error_log_btn, &ButtonControl::clicked, [=]() {
    const QString file_path = "/data/tmux_error.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto tmux_console = new ButtonControl(tr("tmux console"), tr("VIEW"));
  QObject::connect(tmux_console, &ButtonControl::clicked, [=]() {
    QProcess process;
    QStringList arguments;
    arguments << "capture-pane" << "-p" << "-t" << "0" << "-S" << "-250";
    process.start("tmux", arguments);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    ConfirmationDialog::rich(output, this);
  });

  auto pandaflash_btn = new ButtonControl(tr("Panda Flash"), tr("RUN"));
  QObject::connect(pandaflash_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Panda Flash<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/panda/board/flash.py");
    }
  });

  auto pandarecover_btn = new ButtonControl(tr("Panda Recover"), tr("RUN"));
  QObject::connect(pandarecover_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Panda Recover<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/panda/board/recover.py");
    }
  });

  auto scons_rebuild_btn = new ButtonControl(tr("Scons Rebuild"), tr("RUN"));
  QObject::connect(scons_rebuild_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Scons Rebuild<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/scons_rebuild.sh");
    }
  });

  funcBtn = new ListWidget(this);
  funcBtn->addItem(tmux_error_log_btn);
  funcBtn->addItem(tmux_console);
  funcBtn->addItem(can_missing_error_log_btn);
  funcBtn->addItem(can_timeout_error_log_btn);
  funcBtn->addItem(gitcheckout_btn);
  funcBtn->addItem(gitreset_btn);
  funcBtn->addItem(scons_rebuild_btn);
  funcBtn->addItem(cleardtc_btn);
  funcBtn->addItem(pandaflash_btn);
  funcBtn->addItem(pandarecover_btn);

  // upload btn
  auto tmux_error_log_upload_btn = new ButtonControl(tr("tmux log"), tr("UPLOAD"));
  QObject::connect(tmux_error_log_upload_btn, &ButtonControl::clicked, [=]() {
    const QString file_path = "/data/tmux_error.log";
    if (QFile::exists(file_path)) {
      if (ConfirmationDialog::confirm(tr("tmux log upload<br><br>Process?"), tr("Process"), this)) {
        QProcess::execute("/data/openpilot/scripts/log_upload.sh tmux_error.log");
      }
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto tmux_console_upload = new ButtonControl(tr("tmux console"), tr("UPLOAD"));
  QObject::connect(tmux_console_upload, &ButtonControl::clicked, [=]() {
    int exitCode = QProcess::execute("sh", QStringList() << "-c" << "tmux capture-pane -p -t 0 -S -250 > /data/tmux_console.log");
    if (exitCode == 0) {
      if (ConfirmationDialog::confirm(tr("tmux console log upload<br><br>Process?"), tr("Process"), this)) {
        QProcess::execute("/data/openpilot/scripts/log_upload.sh tmux_console.log");
      }
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto carparams_dump_upload_btn = new ButtonControl(tr("carParams dump"), tr("UPLOAD"));
  QObject::connect(carparams_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("carParams dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh carParams");
    }
  });

  auto carstate_dump_upload_btn = new ButtonControl(tr("carState dump"), tr("UPLOAD"));
  QObject::connect(carstate_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("carState dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh carState");
    }
  });

  auto carcontrol_dump_upload_btn = new ButtonControl(tr("carControl dump"), tr("UPLOAD"));
  QObject::connect(carcontrol_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("carControl dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh carControl");
    }
  });

  auto controlsstate_dump_upload_btn = new ButtonControl(tr("controlsState dump"), tr("UPLOAD"));
  QObject::connect(controlsstate_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("controlsState dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh controlsState");
    }
  });

  auto devicestate_dump_upload_btn = new ButtonControl(tr("deviceState dump"), tr("UPLOAD"));
  QObject::connect(devicestate_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("deviceState dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh deviceState");
    }
  });

  auto pandastates_dump_upload_btn = new ButtonControl(tr("pandaStates dump"), tr("UPLOAD"));
  QObject::connect(pandastates_dump_upload_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("pandaStates dump upload<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/dump_upload.sh pandaStates");
    }
  });

  uploadBtn = new ListWidget(this);
  uploadBtn->addItem(tmux_error_log_upload_btn);
  uploadBtn->addItem(tmux_console_upload);
  uploadBtn->addItem(carparams_dump_upload_btn);
  uploadBtn->addItem(carstate_dump_upload_btn);
  uploadBtn->addItem(carcontrol_dump_upload_btn);
  uploadBtn->addItem(controlsstate_dump_upload_btn);
  uploadBtn->addItem(devicestate_dump_upload_btn);
  uploadBtn->addItem(pandastates_dump_upload_btn);

  toggles_layout->addWidget(mainToggles);
  toggles_layout->addWidget(funcBtn);
  toggles_layout->addWidget(uploadBtn);

  ScrollView* toggles_view = new ScrollView(toggles, this);
  communityLayout->addWidget(toggles_view, 1);

  homeScreen->setLayout(communityLayout);
  main_layout->addWidget(homeScreen);
  main_layout->setCurrentWidget(homeScreen);

  togglesCommunity(0);
}

void CommunityPanel::togglesCommunity(int widgetIndex) {
  currentCommunityIndex = widgetIndex;
  mainToggles->setVisible(widgetIndex == 0);
  funcBtn->setVisible(widgetIndex == 1);
  uploadBtn->setVisible(widgetIndex == 2);
}

void CommunityPanel::blueButtonStyle(QPushButton* button) {
  button->setStyleSheet(R"(
    QPushButton {
      font-size: 50px; margin: 0px; padding: 15px; border-width: 0; border-radius: 15px;
      color: #FFFFFF; background-color: #2C2CE2;
    }
    QPushButton:pressed {
      background-color: #2424FF;
    }
  )");
}

void CommunityPanel::updateButtonStyles() {
  QString styleSheet = R"(
    #gitpull_btn, #toggle_btn, #func_btn, #upload_btn {
      height: 120px; border-radius: 15px; background-color: #393939;
    }
    #gitpull_btn:pressed, #toggle_btn:pressed, #func_btn:pressed, #upload_btn:pressed {
      background-color: #4a4a4a;
    }
  )";

  switch (currentCommunityIndex) {
  case 0:
    styleSheet += "#toggle_btn { background-color: #33ab4c; }";
    break;
  case 1:
    styleSheet += "#func_btn { background-color: #33ab4c; }";
    break;
  case 2:
    styleSheet += "#upload_btn { background-color: #33ab4c; }";
    break;
  }

  setStyleSheet(styleSheet);
}
