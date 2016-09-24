// TabletWindow.h

#pragma once

bool initScene();
void surfaceChangedScene(int w, int h);
void drawScene();

void onSingleTouchEvent(int pointerid, int action, float x, float y);
void onWheelEvent(double dx, double dy);