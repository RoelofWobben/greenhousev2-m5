#pragma once
#include <M5Unified.h>
#include "panel.h"
 
class ScrollSystem {
private:
  int touchStartY = 0;
  int scrollStartOffSet = 0;
  bool isDragging = false;
 
public:
  void handleScroll(PanelSystem& panels, void (*redrawAll)());
};
 
