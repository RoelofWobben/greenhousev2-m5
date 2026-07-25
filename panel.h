#pragma once
#include <M5Unified.h>

struct RectButton {
  int x, y, w, h;
  const char* label;
};

enum LightState {
  STATE_OFF,
  STATE_ON,
  STATE_WAIT
};

struct Panel {
  int x, y, w, h;
  const char* label;
  const char* textOn;
  const char* textOff;
  const char* textWait;
  const char* mqttTopic;
  const char* mqttStatusTopic;

  LightState state = STATE_OFF;
  LightState previousState = STATE_OFF;
  unsigned long waitStartMillis = 0;
};

class PanelSystem {
private:
  M5Canvas canvas;

  uint16_t panelColor = 0x18E3;
  uint16_t grey = 0x39C7;
  uint16_t waitColor = 0xFD20;

  int scrollOffSet = 0;
  int minScrollOffSet = 0;
  int maxScrollOffSet = 110;

  static const unsigned long TIMEOUT_MS = 5000;

public:
  PanelSystem();

  void begin();
  void flush();

  void setScrollOffset(int newOffset);
  int getScrollOffset() const;

  void drawPanel(const Panel& panel, const uint16_t* iconOn, const uint16_t* iconOff);
  void drawSingleButton(const RectButton& button, uint16_t color);
  RectButton getOnButton(const Panel& panel);
  RectButton getOffButton(const Panel& panel);
  void drawButtons(const Panel& panel);
  bool isButtonTouched(const RectButton& button);

  void requestState(Panel& panel);
  void confirmState(Panel& panel, LightState confirmedState);
  bool checkTimeout(Panel& panel);

  M5Canvas& getCanvas();
};