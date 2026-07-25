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
Panel lightPanel  = { 10, 10,  300, 100, "Light",  "On",   "Off",   "...", "greenhouse/light/set",  "greenhouse/light/status" };
Panel pompPanel   = { 10, 130, 300, 100, "Pomp",   "On",   "Off",   "...", nullptr,                  nullptr };
Panel windowPanel = { 10, 250, 300, 100, "Window", "Open", "Closed","...", nullptr,                  nullptr };

PanelSystem panels;
ScrollSystem scroller;

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

  panels.flush();
}

// Verwerkt een tik op AAN/UIT: alleen toegestaan als het paneel niet al
// aan het wachten is op een eerdere bevestiging.
void handlePanelTouch(Panel& panel) {
  if (panel.state == STATE_WAIT) return;   // al bezig, negeer nieuwe tikken

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
  if (panels.checkTimeout(pompPanel)) anyTimedOut = true;
  if (panels.checkTimeout(windowPanel)) anyTimedOut = true;

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

  //connectWifi();
  //connectMqtt();

  drawPanels();
}

void loop() {
  M5.update();

  //ensureMqttConnected();

  scroller.handleScroll(panels, drawPanels);

  handlePanelTouch(lightPanel);
  handlePanelTouch(pompPanel);
  handlePanelTouch(windowPanel);

  checkAllTimeouts();
}
