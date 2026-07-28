#include <M5Unified.h>

#include "wifi_conn.h"
#include "mqtt_conn.h"

#include "pump_off.h"
#include "pump_on.h"
#include "window_closed.h"
#include "window_open.h"
#include "light_off.h"
#include "light_on.h"

#include "panel.h"
#include "scroll.h"

// drie panelen -- let op: geen "status"-bool meer los, die zit nu IN de Panel
// (state, previousState, waitStartMillis)
Panel lightPanel = { 10, 10, 300, 100, "Light", "On", "Off", "...", "greenhouse/light/set", "greenhouse/light/status" };
Panel pompPanel = { 10, 130, 300, 100, "Pomp", "On", "Off", "...", nullptr, nullptr };
Panel windowPanel = { 10, 250, 300, 100, "Window", "Open", "Closed", "...", nullptr, nullptr };

PanelSystem panels;
ScrollSystem scroller;

enum Screen {
  SCREEN_STATUS,
  SCREEN_BEDIENING,
};

Screen currentScreen = SCREEN_BEDIENING;

int tabBarY = 200;
int tabBarHeight = 40;

void drawTabBar() {

  M5Canvas canvas = panels.getCanvas();

  uint16_t activeColor = 0x03df;
  uint16_t inactiveColor = 0x0000;

  uint16_t statusColor = (currentScreen == SCREEN_STATUS) ? activeColor : inactiveColor;
  uint16_t bedieningsColor = (currentScreen == SCREEN_BEDIENING) ? activeColor : inactiveColor;

  canvas.fillRect(0, tabBarY, 160, tabBarHeight, statusColor);
  canvas.fillRect(160, tabBarY, 160, tabBarHeight, bedieningsColor);

  canvas.setTextColor(WHITE, statusColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_center);
  canvas.drawString("Status", 80, tabBarY + tabBarHeight / 2);

  canvas.setTextColor(WHITE, bedieningsColor);
  canvas.drawString("Bediening", 240, tabBarY + tabBarHeight / 2);
}

bool isTabTouched(int tabIndex) {

  if (M5.Touch.getCount() == 0) return false;

  auto detail = M5.Touch.getDetail(0);

  if (!detail.wasPressed()) return false;

  int tabX = tabIndex * 160;

  return (detail.x >= tabX && detail.x <= tabX + 160 && detail.y >= tabBarY && detail.y <= tabBarY + tabBarHeight);
}

void handleTabtouch() {

  if (isTabTouched(0) && currentScreen != SCREEN_STATUS) {
    currentScreen = SCREEN_STATUS;
    drawCurrentScreen();
  }

  if (isTabTouched(1) && currentScreen != SCREEN_BEDIENING) {
    currentScreen = SCREEN_BEDIENING;
    drawCurrentScreen();
  }
}

void drawStatusCard(int y, const char* title, const char* statusText, const uint16_t* icon, uint16_t statusColor) {

  M5Canvas canvas = panels.getCanvas();
  uint16_t cardColor = 0x18e3;

  canvas.fillRoundRect(10, y, 300, 60, 12, cardColor);

  int textX = 20;

  if (icon != nullptr) {
    canvas.setSwapBytes(true);
    canvas.pushImage(20, y + 14, 32, 32, icon, 0xFFFF);
    textX = 62;
  }

  canvas.setTextColor(WHITE, cardColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(top_left);
  canvas.drawString(title, textX, y + 8);

  canvas.setTextColor(statusColor, cardColor);
  canvas.setTextSize(1);
  canvas.drawString(statusText, textX, y + 14);
}

void drawStatusScreen() {

  panels.getCanvas().fillScreen(BLACK);

  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  drawStatusCard(10, "WiFi", wifiOk ? "Verbonden" : "Niet verbonden", nullptr, wifiOk ? GREEN : RED);
}

void drawCurrentScreen() {

  if (currentScreen == SCREEN_STATUS) {
    drawStatusScreen();
  } else {
    drawPanels();
  }
}


// Publiceert het GEVRAAGDE commando (nog geen bevestigde status)
void publishRequest(const Panel& panel, LightState requested) {
  if (panel.mqttTopic == nullptr) return;

  const char* payload = (requested == STATE_ON) ? "ON" : "OFF";
  MqttClient.publish(panel.mqttTopic, payload);

  Serial.print(panel.label);
  Serial.print(" verzoek -> ");
  Serial.println(payload);
}

void drawPanels() {
  panels.getCanvas().fillScreen(BLACK);

  panels.drawPanel(lightPanel, lightIconOn, lightIcon);
  panels.drawButtons(lightPanel);

  panels.drawPanel(pompPanel, pumpIconOn, pumpIconOff);
  panels.drawButtons(pompPanel);

  panels.drawPanel(windowPanel, windowIconOpen, windowIconClosed);
  panels.drawButtons(windowPanel);

  drawTabBar();
  panels.flush();
}

// Verwerkt een tik op AAN/UIT: alleen toegestaan als het paneel niet al
// aan het wachten is op een eerdere bevestiging.
void handlePanelTouch(Panel& panel) {
  if (panel.state == STATE_WAIT) return;  // al bezig, negeer nieuwe tikken

  RectButton onButton = panels.getOnButton(panel);
  RectButton offButton = panels.getOffButton(panel);

  if (panel.state != STATE_ON && panels.isButtonTouched(onButton)) {
    panels.requestState(panel);
    drawPanels();

    publishRequest(panel, STATE_ON);
  }

  if (panel.state != STATE_OFF && panels.isButtonTouched(offButton)) {
    panels.requestState(panel);
    drawPanels();

    publishRequest(panel, STATE_OFF);
  }
}

// Checkt voor alle panelen of een timeout is verstreken; herTekent indien nodig.
void checkAllTimeouts() {
  bool anyTimedOut = false;

  if (panels.checkTimeout(lightPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(pompPanel)) anyTimedOut = true;
  //if (panels.checkTimeout(windowPanel)) anyTimedOut = true;

  if (anyTimedOut) {
    drawPanels();
  }
}

// Wordt aangeroepen zodra er een MQTT-bericht binnenkomt.
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("M5 ontving op ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == lightPanel.mqttStatusTopic) {
    LightState confirmed = (message == "ON") ? STATE_ON : STATE_OFF;
    panels.confirmState(lightPanel, confirmed);
    drawPanels();
  }
}

void setup() {
  Serial.begin(115200);

  auto cfg = M5.config();
  M5.begin(cfg);

  panels.begin();

  connectWifi();
  connectMqtt();

  drawPanels();
}

void loop() {
  M5.update();

  ensureMqttConnected();

  handleTabtouch();

  if (currentScreen == SCREEN_BEDIENING) {
    scroller.handleScroll(panels, drawPanels);

    handlePanelTouch(lightPanel);
    handlePanelTouch(pompPanel);
    handlePanelTouch(windowPanel);
  }
  checkAllTimeouts();
}
