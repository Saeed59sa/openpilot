#include <cassert>
#include <cmath>
#include <string>
#include <tuple>
#include <vector>

#include <QDebug>

#include "common/watchdog.h"
#include "common/util.h"
#include "selfdrive/ui/qt/offroad/driverview.h"
#include "selfdrive/ui/qt/network/networking.h"
#include "selfdrive/ui/qt/offroad/settings.h"
#include "selfdrive/ui/qt/qt_window.h"
#include "selfdrive/ui/qt/widgets/prime.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/offroad/developer_panel.h"

#include <QComboBox>
#include <QAbstractItemView>
#include <QScroller>
#include <QListView>
#include <QListWidget>
#include <QProcess>

TogglesPanel::TogglesPanel(SettingsWindow *parent) : ListWidget(parent) {
  // param, title, desc, icon
  std::vector<std::tuple<QString, QString, QString, QString>> toggle_defs{
    {
      "OpenpilotEnabledToggle",
      tr("Enable openpilot"),
      tr("Use the openpilot system for adaptive cruise control and lane keep driver assistance. Your attention is required at all times to use this feature. Changing this setting takes effect when the car is powered off."),
      "../assets/offroad/icon_openpilot.png",
    },
    {
      "ExperimentalLongitudinalEnabled",
      tr("openpilot Longitudinal Control (Alpha)"),
      QString("<b>%1</b><br><br>%2")
      .arg(tr("WARNING: openpilot longitudinal control is in alpha for this car and will disable Automatic Emergency Braking (AEB)."))
      .arg(tr("On this car, openpilot defaults to the car's built-in ACC instead of openpilot's longitudinal control. "
              "Enable this to switch to openpilot longitudinal control. Enabling Experimental mode is recommended when enabling openpilot longitudinal control alpha.")),
      "../assets/offroad/icon_speed_limit.png",
    },
    {
      "ExperimentalMode",
      tr("Experimental Mode"),
      "",
      "../assets/img_experimental_white.svg",
    },
    {
      "DisengageOnAccelerator",
      tr("Disengage on Accelerator Pedal"),
      tr("When enabled, pressing the accelerator pedal will disengage openpilot."),
      "../assets/offroad/icon_disengage_on_accelerator.svg",
    },
    {
      "IsLdwEnabled",
      tr("Enable Lane Departure Warnings"),
      tr("Receive alerts to steer back into the lane when your vehicle drifts over a detected lane line without a turn signal activated while driving over 31 mph (50 km/h)."),
      "../assets/offroad/icon_ldws.png",
    },
    {
      "AutoLaneChangeEnabled",
      tr("Enable AutoLaneChange"),
      tr("Operation of the turn signal at 60㎞/h speed will result in a short change of the vehicle"),
      "../assets/offroad/icon_lca.png",
    },
    {
      "AlwaysOnDM",
      tr("Always-On Driver Monitoring"),
      tr("Enable driver monitoring even when openpilot is not engaged."),
      "../assets/offroad/icon_monitoring.png",
    },
    {
      "RecordFront",
      tr("Record and Upload Driver Camera"),
      tr("Upload data from the driver facing camera and help improve the driver monitoring algorithm."),
      "../assets/offroad/icon_monitoring.png",
    },
    {
      "IsMetric",
      tr("Use Metric System"),
      tr("Display speed in km/h instead of mph."),
      "../assets/offroad/icon_metric.png",
    },
  };


  std::vector<QString> longi_button_texts{tr("Aggressive"), tr("Standard"), tr("Relaxed")};
  long_personality_setting = new ButtonParamControl("LongitudinalPersonality", tr("Driving Personality"),
                                          tr("Standard is recommended. In aggressive mode, openpilot will follow lead cars closer and be more aggressive with the gas and brake. "
                                             "In relaxed mode openpilot will stay further away from lead cars. On supported cars, you can cycle through these personalities with "
                                             "your steering wheel distance button."),
                                          "../assets/offroad/icon_speed_limit.png",
                                          longi_button_texts);

  // set up uiState update for personality setting
  QObject::connect(uiState(), &UIState::uiUpdate, this, &TogglesPanel::updateState);

  for (auto &[param, title, desc, icon] : toggle_defs) {
    auto toggle = new ParamControl(param, title, desc, icon, this);

    bool locked = params.getBool((param + "Lock").toStdString());
    toggle->setEnabled(!locked);

    addItem(toggle);
    toggles[param.toStdString()] = toggle;

    // insert longitudinal personality after NDOG toggle
    if (param == "DisengageOnAccelerator") {
      addItem(long_personality_setting);
    }
  }

  // Toggles with confirmation dialogs
  toggles["ExperimentalMode"]->setActiveIcon("../assets/img_experimental.svg");
  toggles["ExperimentalMode"]->setConfirmation(true, true);
  toggles["ExperimentalLongitudinalEnabled"]->setConfirmation(true, false);

  connect(toggles["ExperimentalLongitudinalEnabled"], &ToggleControl::toggleFlipped, [=]() {
    updateToggles();
  });
}

void TogglesPanel::updateState(const UIState &s) {
  const SubMaster &sm = *(s.sm);

  if (sm.updated("selfdriveState")) {
    auto personality = sm["selfdriveState"].getSelfdriveState().getPersonality();
    if (personality != s.scene.personality && s.scene.started && isVisible()) {
      long_personality_setting->setCheckedButton(static_cast<int>(personality));
    }
    uiState()->scene.personality = personality;
  }
}

void TogglesPanel::expandToggleDescription(const QString &param) {
  toggles[param.toStdString()]->showDescription();
}

void TogglesPanel::showEvent(QShowEvent *event) {
  updateToggles();
}

void TogglesPanel::updateToggles() {
  auto experimental_mode_toggle = toggles["ExperimentalMode"];
  auto op_long_toggle = toggles["ExperimentalLongitudinalEnabled"];
  const QString e2e_description = QString("%1<br>"
                                          "<h4>%2</h4><br>"
                                          "%3<br>"
                                          "<h4>%4</h4><br>"
                                          "%5<br>")
                                  .arg(tr("openpilot defaults to driving in <b>chill mode</b>. Experimental mode enables <b>alpha-level features</b> that aren't ready for chill mode. Experimental features are listed below:"))
                                  .arg(tr("End-to-End Longitudinal Control"))
                                  .arg(tr("Let the driving model control the gas and brakes. openpilot will drive as it thinks a human would, including stopping for red lights and stop signs. "
                                          "Since the driving model decides the speed to drive, the set speed will only act as an upper bound. This is an alpha quality feature; "
                                          "mistakes should be expected."))
                                  .arg(tr("New Driving Visualization"))
                                  .arg(tr("The driving visualization will transition to the road-facing wide-angle camera at low speeds to better show some turns. The Experimental mode logo will also be shown in the top right corner."));

  const bool is_release = params.getBool("IsReleaseBranch");
  auto cp_bytes = params.get("CarParamsPersistent");
  if (!cp_bytes.empty()) {
    AlignedBuffer aligned_buf;
    capnp::FlatArrayMessageReader cmsg(aligned_buf.align(cp_bytes.data(), cp_bytes.size()));
    cereal::CarParams::Reader CP = cmsg.getRoot<cereal::CarParams>();

    if (!CP.getExperimentalLongitudinalAvailable() || is_release) {
      params.remove("ExperimentalLongitudinalEnabled");
    }
    op_long_toggle->setVisible(CP.getExperimentalLongitudinalAvailable() && !is_release);
    if (hasLongitudinalControl(CP)) {
      // normal description and toggle
      experimental_mode_toggle->setEnabled(true);
      experimental_mode_toggle->setDescription(e2e_description);
      long_personality_setting->setEnabled(true);
    } else {
      // no long for now
      experimental_mode_toggle->setEnabled(false);
      long_personality_setting->setEnabled(false);
      params.remove("ExperimentalMode");

      const QString unavailable = tr("Experimental mode is currently unavailable on this car since the car's stock ACC is used for longitudinal control.");

      QString long_desc = unavailable + " " + \
                          tr("openpilot longitudinal control may come in a future update.");
      if (CP.getExperimentalLongitudinalAvailable()) {
        if (is_release) {
          long_desc = unavailable + " " + tr("An alpha version of openpilot longitudinal control can be tested, along with Experimental mode, on non-release branches.");
        } else {
          long_desc = tr("Enable the openpilot longitudinal control (alpha) toggle to allow Experimental mode.");
        }
      }
      experimental_mode_toggle->setDescription("<b>" + long_desc + "</b><br><br>" + e2e_description);
    }

    experimental_mode_toggle->refresh();
  } else {
    experimental_mode_toggle->setDescription(e2e_description);
    op_long_toggle->setVisible(false);
  }
}

DevicePanel::DevicePanel(SettingsWindow *parent) : ListWidget(parent) {
  setSpacing(7);
  addItem(new LabelControl(tr("Dongle ID"), getDongleId().value_or(tr("N/A"))));
  addItem(new LabelControl(tr("Serial"), params.get("HardwareSerial").c_str()));

  pair_device = new ButtonControl(tr("Pair Device"), tr("PAIR"),
                                  tr("Pair your device with comma connect (connect.comma.ai) and claim your comma prime offer."));
  connect(pair_device, &ButtonControl::clicked, [=]() {
    PairingPopup popup(this);
    popup.exec();
  });
  addItem(pair_device);

  // offroad-only buttons

  auto dcamBtn = new ButtonControl(tr("Driver Camera"), tr("PREVIEW"),
                                   tr("Preview the driver facing camera to ensure that driver monitoring has good visibility. (vehicle must be off)"));
  connect(dcamBtn, &ButtonControl::clicked, [this, dcamBtn]() {
    dcamBtn->setEnabled(false);
    DriverViewDialog driver_view(this);
    driver_view.exec();
    dcamBtn->setEnabled(true);
  });
  addItem(dcamBtn);

  auto resetCalibBtn = new ButtonControl(tr("Reset Calibration"), tr("RESET"), "");
  connect(resetCalibBtn, &ButtonControl::showDescriptionEvent, this, &DevicePanel::updateCalibDescription);
  connect(resetCalibBtn, &ButtonControl::clicked, [&]() {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to reset calibration?"), tr("Reset"), this)) {
      params.remove("CalibrationParams");
      params.remove("LiveTorqueParameters");
      emit closeSettings();
      QTimer::singleShot(1000, []() {
        Params().putBool("SoftRestartTriggered", true);
      });
    }
  });
  addItem(resetCalibBtn);

  auto retrainingBtn = new ButtonControl(tr("Review Training Guide"), tr("REVIEW"), tr("Review the rules, features, and limitations of openpilot"));
  connect(retrainingBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to review the training guide?"), tr("Review"), this)) {
      emit reviewTrainingGuide();
    }
  });
  addItem(retrainingBtn);

  /*if (Hardware::TICI()) {
    auto regulatoryBtn = new ButtonControl(tr("Regulatory"), tr("VIEW"), "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file("../assets/offroad/fcc.html");
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    });
    addItem(regulatoryBtn);
  }*/

  auto translateBtn = new ButtonControl(tr("Change Language"), tr("CHANGE"), "");
  connect(translateBtn, &ButtonControl::clicked, [=]() {
    QMap<QString, QString> langs = getSupportedLanguages();
    QString selection = MultiOptionDialog::getSelection(tr("Select a language"), langs.keys(), langs.key(uiState()->language), this);
    if (!selection.isEmpty()) {
      // put language setting, exit Qt UI, and trigger fast restart
      params.put("LanguageSetting", langs[selection].toStdString());
      qApp->exit(18);
      watchdog_kick(0);
    }
  });
  addItem(translateBtn);

  QObject::connect(uiState()->prime_state, &PrimeState::changed, [this] (PrimeState::Type type) {
    pair_device->setVisible(type == PrimeState::PRIME_TYPE_UNPAIRED);
  });
  QObject::connect(uiState(), &UIState::offroadTransition, [=](bool offroad) {
    for (auto btn : findChildren<ButtonControl *>()) {
      if (btn != pair_device) {
        btn->setEnabled(offroad);
      }
    }
    resetCalibBtn->setEnabled(true);
    translateBtn->setEnabled(true);
  });

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  // softreset button
  QPushButton *restart_btn = new QPushButton(tr("Soft Restart"));
  restart_btn->setObjectName("restart_btn");
  power_layout->addWidget(restart_btn);
  QObject::connect(restart_btn, &QPushButton::released, [=]() {
    emit closeSettings();
    QTimer::singleShot(1000, []() {
      Params().putBool("SoftRestartTriggered", true);
    });
  });

  QPushButton *reboot_btn = new QPushButton(tr("Reboot"));
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, this, &DevicePanel::reboot);

  QPushButton *poweroff_btn = new QPushButton(tr("Power Off"));
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, this, &DevicePanel::poweroff);

  if (!Hardware::PC()) {
    connect(uiState(), &UIState::offroadTransition, poweroff_btn, &QPushButton::setVisible);
  }

  setStyleSheet(R"(
    #restart_btn { height: 120px; border-radius: 15px; background-color: #2C2CE2; }
    #restart_btn:pressed { background-color: #2424FF; }
    #reboot_btn { height: 120px; border-radius: 15px; background-color: #2CE22C; }
    #reboot_btn:pressed { background-color: #24FF24; }
    #poweroff_btn { height: 120px; border-radius: 15px; background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  addItem(power_layout);
}

void DevicePanel::updateCalibDescription() {
  QString desc =
      tr("openpilot requires the device to be mounted within 4° left or right and "
         "within 5° up or 9° down. openpilot is continuously calibrating, resetting is rarely required.");
  std::string calib_bytes = params.get("CalibrationParams");
  if (!calib_bytes.empty()) {
    try {
      AlignedBuffer aligned_buf;
      capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
      auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
      if (calib.getCalStatus() != cereal::LiveCalibrationData::Status::UNCALIBRATED) {
        double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
        double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
        desc += tr(" Your device is pointed %1° %2 and %3° %4.")
                    .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? tr("down") : tr("up"),
                         QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? tr("left") : tr("right"));
      }
    } catch (kj::Exception) {
      qInfo() << "invalid CalibrationParams";
    }
  }
  qobject_cast<ButtonControl *>(sender())->setDescription(desc);
}

void DevicePanel::reboot() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to reboot?"), tr("Reboot"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoReboot", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("Disengage to Reboot"), this);
  }
}

void DevicePanel::poweroff() {
  if (!uiState()->engaged()) {
    if (ConfirmationDialog::confirm(tr("Are you sure you want to power off?"), tr("Power Off"), this)) {
      // Check engaged again in case it changed while the dialog was open
      if (!uiState()->engaged()) {
        params.putBool("DoShutdown", true);
      }
    }
  } else {
    ConfirmationDialog::alert(tr("Disengage to Power Off"), this);
  }
}

void SettingsWindow::showEvent(QShowEvent *event) {
  setCurrentPanel(0);
}

void SettingsWindow::setCurrentPanel(int index, const QString &param) {
  panel_widget->setCurrentIndex(index);
  nav_btns->buttons()[index]->setChecked(true);
  if (!param.isEmpty()) {
    emit expandToggleDescription(param);
  }
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();

  // close button
  QPushButton *close_btn = new QPushButton(tr("×"));
  close_btn->setStyleSheet(R"(
    QPushButton {
      font-size: 140px;
      padding-bottom: 20px;
      border-radius: 100px;
      background-color: #292929;
      font-weight: 400;
    }
    QPushButton:pressed {
      background-color: #3B3B3B;
    }
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, &QPushButton::clicked, this, &SettingsWindow::closeSettings);

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::closeSettings, this, &SettingsWindow::closeSettings);

  TogglesPanel *toggles = new TogglesPanel(this);
  QObject::connect(this, &SettingsWindow::expandToggleDescription, toggles, &TogglesPanel::expandToggleDescription);

  auto networking = new Networking(this);
  QObject::connect(uiState()->prime_state, &PrimeState::changed, networking, &Networking::setPrimeType);

  QList<QPair<QString, QWidget *>> panels = {
    {tr("Device"), device},
    {tr("Network"), networking},
    {tr("Toggles"), toggles},
    {tr("Software"), new SoftwarePanel(this)},
    {tr("Community"), new CommunityPanel(this)},
    {tr("Developer"), new DeveloperPanel(this)},
  };

  nav_btns = new QButtonGroup(this);
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 60px;
        font-weight: 500;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )");
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    const int lr_margin = name != tr("Network") ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
    QStackedWidget, ScrollView {
      background-color: #292929;
      border-radius: 30px;
    }
  )");
}

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
  communityLayout->setMargin(40);

  // selectedManufacturer
  QString selectedManufacturer = QString::fromStdString(Params().get("SelectedManufacturer"));
  QPushButton* selectManufacturer_btn = new QPushButton(selectedManufacturer.length() ? selectedManufacturer : tr("Select your Manufacturer"));
  selectManufacturer_btn->setObjectName("selectManufacturer_btn");
  selectManufacturer_btn->setStyleSheet(R"(
    QPushButton { font-size: 50px; margin: 0px; padding: 15px; border-width: 0; border-radius: 15px; color: #FFFFFF; background-color: #2C2CE2; }
    QPushButton:pressed { background-color: #2424FF; }
  )");
  connect(selectManufacturer_btn, &QPushButton::clicked, [=]() {
    QStringList manufacturers = {"[ Not Selected ]", "HYUNDAI", "KIA", "GENESIS"};

    QString selectedOption = MultiOptionDialog::getSelection(tr("Select your Manufacturer"), manufacturers,
                                                            selectedManufacturer.isEmpty() ? manufacturers[0] : selectedManufacturer,
                                                            this);

    if (!selectedOption.isEmpty()) {
      if (selectedOption == "[ Not Selected ]") {
        Params().remove("SelectedManufacturer");
        qApp->exit(18);
        watchdog_kick(0);      } else {
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
  selectCar_btn->setStyleSheet(R"(
    QPushButton { font-size: 50px; margin: 0px; padding: 15px; border-width: 0; border-radius: 15px; color: #FFFFFF; background-color: #2C2CE2; }
    QPushButton:pressed { background-color: #2424FF; }
  )");

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
  selectBranch_btn->setStyleSheet(R"(
    QPushButton { font-size: 50px; margin: 0px; padding: 15px; border-width: 0; border-radius: 15px; color: #FFFFFF; background-color: #2C2CE2; }
    QPushButton:pressed { background-color: #2424FF; }
  )");

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

  QPushButton* toggle_btn = new QPushButton(tr("Toggle"));
  toggle_btn->setObjectName("toggle_btn");
  QObject::connect(toggle_btn, &QPushButton::clicked, this, [this]() {this->togglesCommunity(0); });

  QPushButton* func_btn = new QPushButton(tr("Function"));
  func_btn->setObjectName("func_btn");
  QObject::connect(func_btn, &QPushButton::clicked, this, [this]() {this->togglesCommunity(1); });

  QPushButton* upload_btn = new QPushButton(tr("Upload"));
  upload_btn->setObjectName("upload_btn");
  QObject::connect(upload_btn, &QPushButton::clicked, this, [this]() {this->togglesCommunity(2); });

  setStyleSheet(R"(
    #toggle_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #toggle_btn:pressed { background-color: #4a4a4a; }
    #func_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #func_btn:pressed { background-color: #4a4a4a; }
    #upload_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #upload_btn:pressed { background-color: #4a4a4a; }
  )");

  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->setSpacing(30);

  gridLayout->addWidget(selectManufacturer_btn, 0, 0);
  gridLayout->addWidget(selectCar_btn, 0, 1);
  gridLayout->addWidget(selectBranch_btn, 0, 2);

  gridLayout->addWidget(toggle_btn, 1, 0);
  gridLayout->addWidget(func_btn, 1, 1);
  gridLayout->addWidget(upload_btn, 1, 2);

  communityLayout->addLayout(gridLayout, 0);

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

  auto gitcheckout_btn = new ButtonControl(tr("Git Checkout"), tr("RUN"));
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

  auto gitpull_btn = new ButtonControl(tr("Git Fetch and Reset"), tr("RUN"));
  QObject::connect(gitpull_btn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm(tr("Git Fetch and Reset<br><br>Process?"), tr("Process"), this)) {
      QProcess::execute("/data/openpilot/scripts/gitpull.sh");
    }
    const QString file_path = "/data/check_network.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    }
  });

  auto can_missing_error_log_btn = new ButtonControl(tr("can missing log"), tr("VIEW"));
  QObject::connect(can_missing_error_log_btn, &ButtonControl::clicked, [=]() {
    const QString file_path = "/data/can_missing.log";
    if (QFile::exists(file_path)) {
      const std::string txt = util::read_file(file_path.toStdString());
      ConfirmationDialog::rich(QString::fromStdString(txt), this);
    } else {
      ConfirmationDialog::rich(tr("log file not found"), this);
    }
  });

  auto can_timeout_error_log_btn = new ButtonControl(tr("can timeout log"), tr("VIEW"));
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
  funcBtn->addItem(gitpull_btn);
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
  mainToggles->setVisible(widgetIndex == 0);
  funcBtn->setVisible(widgetIndex == 1);
  uploadBtn->setVisible(widgetIndex == 2);
}
