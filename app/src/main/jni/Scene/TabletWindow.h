// TabletWindow.h

#pragma once

#include "GL_Includes.h"

#include "Scene.h"

#include "SimpleIcon.h"
#include "TouchPoints.h"
#include "FPSTimer.h"
#include "vectortypes.h"

class TabletWindow
{
public:
    TabletWindow();
    virtual ~TabletWindow();

    void initGL();
    void setWindowSize(int w, int h) { m_winw = w; m_winh = h; }
    void display(int winw, int winh);
    void timestep(double absT, double dt);

    void OnSingleTouch(int pointerid, int action, int x, int y);
    void OnWheelEvent(double dx, double dy);

protected:
    void _DrawIcon(int winw, int winh);
    void _DrawText(int winw, int winh);
    void _DisplayOverlay(int winw, int winh);
    void _DisplayScene(int winw, int winh);

    Scene m_scene;

    SimpleIcon g_si;
    FPSTimer m_fps;
    Timer m_logDumpTimer;
    int m_winw;
    int m_winh;
    int m_iconx;
    int m_icony;
    float m_iconScale;
    std::string m_glVersion;
    std::string m_glRenderer;
    std::string m_glSLVersion;

    // 3D camera location
    float3 m_chassisPos;
    float3 m_chassisPosAtTouch;
    int2 m_lastTouchPoint;
    float m_chassisYaw;
    float m_chassisYawAtTouch;

    // Motion event states
    bool m_holding;
    unsigned int m_holdingMask;
    std::vector<touchState> m_pointerStates;
    TouchPoints m_tp;
    std::pair<touchState, touchState> m_pinchStart;
    float m_scaleAtPinchStart;

private:
    TabletWindow(const TabletWindow&);              ///< disallow copy constructor
    TabletWindow& operator = (const TabletWindow&); ///< disallow assignment operator
};