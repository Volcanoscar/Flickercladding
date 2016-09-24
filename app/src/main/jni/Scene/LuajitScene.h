// LuajitScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <string>
#include <queue>
#include <lua.hpp>

#include "IScene.h"
#include "GL_Includes.h"

struct queuedTouchEvent {
    int pointerid;
    int action;
    int x;
    int y;
};

class LuajitScene : public IScene
{
public:
    LuajitScene();
    virtual ~LuajitScene();

    virtual int SetSceneName(const std::string& sceneName);
    virtual void initGL();
    virtual void exitGL();
    virtual void keypressed(int key);
    virtual void timestep(double absTime, double dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;
    virtual void onSingleTouch(int pointerid, int action, int x, int y);

    virtual const std::string& ErrorText() const { return m_errorText; }

    virtual void setTracking_Hydra(double absTime, const void* pData);
    virtual void setTracking_ViveWand(double absTime, int idx, const void* pPose, const void* pState);

    virtual bool RayIntersects(
        const float* pRayOrigin,
        const float* pRayDirection,
        float* pTParameter, // [inout]
        float* pHitLocation, // [inout]
        float* pHitNormal // [inout]
        ) const;

    void* m_pLoaderFunc;

protected:
    lua_State* m_Lua;
    mutable bool m_errorOccurred;
    std::string m_errorText;

    std::queue<queuedTouchEvent> m_queuedEvents;

private: // Disallow copy ctor and assignment operator
    LuajitScene(const LuajitScene&);
    LuajitScene& operator=(const LuajitScene&);
};