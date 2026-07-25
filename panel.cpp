#include "panel.h"

PanelSystem::PanelSystem() : canvas(&M5.Display) {}

void PanelSystem::begin() {
  canvas.setColorDepth(16);
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setSwapBytes(true);
}

void PanelSystem::flush() {
  canvas.pushSprite(0, 0);
}

void PanelSystem::setScrollOffset(int newOffset) {
  scrollOffSet = constrain(newOffset, minScrollOffSet, maxScrollOffSet);
}

int PanelSystem::getScrollOffset() const {
  return scrollOffSet;
}

void PanelSystem::drawPanel(const Panel& panel, const uint16_t* iconOn, const uint16_t* iconOff) {
  canvas.fillRoundRect(panel.x, panel.y - scrollOffSet, panel.w, panel.h, 12, panelColor);

  const uint16_t* icon = (panel.state == STATE_OFF) ? iconOff : iconOn;
  canvas.pushImage(panel.x + 20, panel.y + 10 - scrollOffSet, 32, 32, icon, 0xFFFF);

  canvas.setTextColor(WHITE, panelColor);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_left);
  canvas.drawString(panel.label, panel.x + 62, panel.y + 26 - scrollOffSet);
}

void PanelSystem::drawSingleButton(const RectButton& button, uint16_t color) {
  canvas.fillRect(button.x - 2, button.y - 2, button.w - 4, button.h + 4, panelColor);

  canvas.fillRoundRect(button.x, button.y, button.w, button.h, 10, color);

  canvas.setTextColor(WHITE, color);
  canvas.setTextSize(2);
  canvas.setTextDatum(middle_center);
  canvas.drawString(button.label, button.x + button.w / 2, button.y + button.h / 2);
}

RectButton PanelSystem::getOnButton(const Panel& panel) {
  return { panel.x, panel.y + 50 - scrollOffSet, 120, 40, panel.textOn };
}

RectButton PanelSystem::getOffButton(const Panel& panel) {
  return { panel.x + 160, panel.y + 50 - scrollOffSet, 120, 40, panel.textOff };
}

void PanelSystem::drawButtons(const Panel& panel) {
  RectButton onButton = getOnButton(panel);
  RectButton offButton = getOffButton(panel);

  if (panel.state == STATE_WAIT) {
    drawSingleButton(onButton, waitColor);
    drawSingleButton(offButton, waitColor);
  } else {
    bool isOn = (panel.state == STATE_ON);
    drawSingleButton(onButton, isOn ? GREEN : grey);
    drawSingleButton(offButton, isOn ? grey : GREEN);
  }
}

bool PanelSystem::isButtonTouched(const RectButton& button) {
  if (M5.Touch.getCount() == 0) return false;

  auto detail = M5.Touch.getDetail(0);
  if (!detail.wasPressed()) return false;

  return (detail.x >= button.x && detail.x <= button.x + button.w &&
          detail.y >= button.y && detail.y <= button.y + button.h);
}

void PanelSystem::requestState(Panel& panel) {
  panel.previousState = panel.state;
  panel.state = STATE_WAIT;
  panel.waitStartMillis = millis();
}

void PanelSystem::confirmState(Panel& panel, LightState confirmedState) {
  panel.state = confirmedState;
}

bool PanelSystem::checkTimeout(Panel& panel) {
  if (panel.state == STATE_WAIT && (millis() - panel.waitStartMillis > TIMEOUT_MS)) {
    panel.state = panel.previousState;
    return true;
  }
  return false;
}

M5Canvas& PanelSystem::getCanvas() {
  return canvas;
}