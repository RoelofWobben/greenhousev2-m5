#include "scroll.h"

void ScrollSystem::handleScroll(PanelSystem& panels, void (*redrawAll)()) {
  if (M5.Touch.getCount() == 0) {
    isDragging = false;
    return;
  }

  auto detail = M5.Touch.getDetail(0);

  if (detail.wasPressed()) {
    touchStartY = detail.y;
    scrollStartOffSet = panels.getScrollOffset();
    isDragging = true;
  }

  if (isDragging && detail.isDragging()) {
    int deltaY = detail.y - touchStartY;
    int oldOffset = panels.getScrollOffset();

    panels.setScrollOffset(scrollStartOffSet - deltaY);

    if (panels.getScrollOffset() != oldOffset) {
      redrawAll();
    }
  }
}