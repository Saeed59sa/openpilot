#pragma once

#include "selfdrive/ui/qt/offroad/settings.h"

class CommunityPanel : public QWidget {
  Q_OBJECT
public:
  explicit CommunityPanel(QWidget *parent = nullptr);

private:
  QStackedLayout* main_layout = nullptr;
  QWidget* homeScreen = nullptr;

  ListWidget* mainToggles;
  ListWidget* funcBtn;
  ListWidget* uploadBtn;

  void togglesCommunity(int widgetIndex);
  void blueButtonStyle(QPushButton* button);
};
