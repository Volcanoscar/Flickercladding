// sdl_main.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "GL_Includes.h"

#include "cpp_interface.h"
#include "AndroidTouchEnums.h"
#include "TouchReplayer.h"
#include "Timer.h"
#include "Logging.h"

#include <SDL.h>
#undef main

SDL_Window* g_pWindow = NULL;
int winw = 800;
int winh = 800;
bool portrait = true;

TouchReplayer g_trp;
Timer g_playbackTimer;

void initGL()
{
    initScene();
}

void exitGL()
{
    exitScene();
}

void display()
{
    drawScene();
}

void setAppScreenSize()
{
    int w = portrait ? winw : winh;
    int h = portrait ? winh : winw;
    surfaceChangedScene(w, h);
    //glfwSetWindowSize(g_pWindow, w, h);
    glViewport(0, 0, w, h);
}

// OpenGL debug callback
void APIENTRY myCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *msg,
    const void *data)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    case GL_DEBUG_SEVERITY_MEDIUM:
    case GL_DEBUG_SEVERITY_LOW:
        LOG_INFO("[[GL Debug]] %x %x %x %x %s\n", source, type, id, severity, msg);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        break;
    }
}

bool init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return false;

    g_pWindow = SDL_CreateWindow(
        "GL Skeleton - SDL2",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        winw, winh,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (g_pWindow == NULL)
    {
        LOG_ERROR("%s\n", SDL_GetError());
        SDL_Quit();
    }

    // thank you http://www.brandonfoltz.com/2013/12/example-using-opengl-3-0-with-sdl2-and-glew/
    SDL_GLContext glContext = SDL_GL_CreateContext(g_pWindow);
    if (glContext == NULL)
    {
        printf("There was an error creating the OpenGL context!\n");
        return 0;
    }

    SDL_GL_MakeCurrent(g_pWindow, glContext);

    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    return true;
}

int main(int argc, char *argv[])
{
    if (init() == false)
        return 1;

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        LOG_ERROR("Failed to initialize OpenGL context");
        return -1;
    }

    setLoaderFunc((void*)&SDL_GL_GetProcAddress);
    initGL();
    surfaceChangedScene(winw, winh);

    SDL_Event event;
    int quit = 0;
    while (quit == 0)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = 1;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                //g_app.mouseDown(event.button.button, event.button.state, event.button.x, event.button.y);
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                //g_app.mouseMove(event.motion.x, event.motion.y);
            }
            else if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        display();
        SDL_GL_SwapWindow(g_pWindow);
    }

    SDL_Quit();
    return 0;
}
