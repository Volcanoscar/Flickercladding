// Scene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
//#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "IScene.h"
#include "GL_Includes.h"
#include "ShaderWithVariables.h"

///@brief The Scene class renders everything in the VR world that will be the same
/// in the Oculus and Control windows. The RenderForOneEye function is the display entry point.
class Scene : public IScene
{
public:
    Scene();
    virtual ~Scene();

    virtual void initGL();
    virtual void exitGL();
    virtual void timestep(double absTime, double dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual bool RayIntersects(
        const float* pRayOrigin,
        const float* pRayDirection,
        float* pTParameter, // [inout]
        float* pHitLocation, // [inout]
        float* pHitNormal // [inout]
        ) const;

protected:
    void DrawColorCube() const;
    void DrawGrid() const;
    void DrawOrigin() const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;

protected:
    void _InitCubeAttributes();
    void _InitPlaneAttributes();

    void _DrawBouncingCubes(
        const glm::mat4& modelview,
        glm::vec3 center,
        float radius,
        float scale) const;
    void _DrawScenePlanes(const glm::mat4& modelview) const;

    ShaderWithVariables m_basic;
    ShaderWithVariables m_plane;

    float m_phaseVal;

public:
    float m_amplitude;

private: // Disallow copy ctor and assignment operator
    Scene(const Scene&);
    Scene& operator=(const Scene&);
};
