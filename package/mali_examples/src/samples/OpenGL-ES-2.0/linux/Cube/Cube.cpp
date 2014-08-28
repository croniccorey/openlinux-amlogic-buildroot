/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * \file Cube.cpp
 * \brief A simple rotating cube.
 */

#include "Cube.h"

#include "Text.h"
#include "Shader.h"
#include "Texture.h"
#include "Matrix.h"
#include "Timer.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <string>

#include "Platform.h"
#include "EGLRuntime.h"

#include <EGL/egl.h>

#define WINDOW_W 800
#define WINDOW_H 600

using std::string;
using namespace MaliSDK;

/* Asset directories and filenames. */
string resourceDirectory = "assets/";
string vertexShaderFilename = "Cube_cube.vert";
string fragmentShaderFilename = "Cube_cube.frag";

/* Shader variables. */
GLuint programID;
GLint iLocPosition;
GLint iLocColor; 
GLint iLocMVP;

int windowWidth = -1;
int windowHeight = -1;

/* A text object to draw text on the screen. */
Text *text;

bool setupGraphics(int width, int height)
{
    windowWidth = width;
    windowHeight = height;
    
    /* Full paths to the shader and texture files */
    string vertexShaderPath = resourceDirectory + vertexShaderFilename; 
    string fragmentShaderPath = resourceDirectory + fragmentShaderFilename;

    /* Initialize OpenGL ES. */
    GL_CHECK(glEnable(GL_BLEND));
    /* Should do: src * (src alpha) + dest * (1-src alpha). */
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* Initialize the Text object and add some text. */
    text = new Text(resourceDirectory.c_str(), windowWidth, windowHeight);
    text->addString(0, 0, "Simple Cube Example", 255, 255, 0, 255);

    /* Process shaders. */
    GLuint fragmentShaderID, vertexShaderID;
    Shader::processShader(&vertexShaderID, vertexShaderPath.c_str(), GL_VERTEX_SHADER);
    Shader::processShader(&fragmentShaderID, fragmentShaderPath.c_str(), GL_FRAGMENT_SHADER);

    /* Create programID (ready to attach shaders) */
    programID = GL_CHECK(glCreateProgram());

    /* Attach shaders and link programID */
    GL_CHECK(glAttachShader(programID, vertexShaderID));
    GL_CHECK(glAttachShader(programID, fragmentShaderID));
    GL_CHECK(glLinkProgram(programID));
    GL_CHECK(glUseProgram(programID));

    /* Get attribute locations of non-fixed attributes like colour and texture coordinates. */
    iLocPosition = GL_CHECK(glGetAttribLocation(programID, "av4position"));
    iLocColor = GL_CHECK(glGetAttribLocation(programID, "av3colour"));

    LOGD("iLocPosition = %i\n", iLocPosition);
    LOGD("iLocColor   = %i\n", iLocColor);

    /* Get uniform locations */
    iLocMVP = GL_CHECK(glGetUniformLocation(programID, "mvp"));

    LOGD("iLocMVP      = %i\n", iLocMVP);

    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    /* Set clear screen color. */
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

    return true;
}

void renderFrame(void)
{
    GL_CHECK(glUseProgram(programID));

    /* Enable attributes for position, color and texture coordinates etc. */
    GL_CHECK(glEnableVertexAttribArray(iLocPosition));
    GL_CHECK(glEnableVertexAttribArray(iLocColor));

    /* Populate attributes for position, color and texture coordinates etc. */
    GL_CHECK(glVertexAttribPointer(iLocPosition, 3, GL_FLOAT, GL_FALSE, 0, vertices));
    GL_CHECK(glVertexAttribPointer(iLocColor, 3, GL_FLOAT, GL_FALSE, 0, colors));

    static float angleX = 0, angleY = 0, angleZ = 0;
    /*
     * Do some rotation with Euler angles. It is not a fixed axis as
     * quaternions would be, but the effect is nice.
     */
    Matrix modelView = Matrix::createRotationX(angleX);
    Matrix rotation = Matrix::createRotationY(angleY);
    
    modelView = rotation * modelView;
    
    rotation = Matrix::createRotationZ(angleZ);
    
    modelView = rotation * modelView;
    
    /* Pull the camera back from the cube */
    modelView[14] -= 2.5;
    
    Matrix perspective = Matrix::matrixPerspective(45.0f, windowWidth/(float)windowHeight, 0.01f, 100.0f);
    Matrix modelViewPerspective = perspective * modelView;

    GL_CHECK(glUniformMatrix4fv(iLocMVP, 1, GL_FALSE, modelViewPerspective.getAsArray()));

    /* Update cube's rotation angles for animating. */
    angleX += 3;
    angleY += 2;
    angleZ += 1;

    if(angleX >= 360) angleX -= 360;
    if(angleX < 0) angleX += 360;
    if(angleY >= 360) angleY -= 360;
    if(angleY < 0) angleY += 360;
    if(angleZ >= 360) angleZ -= 360;
    if(angleZ < 0) angleZ += 360;

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

    /* Draw any text. */
    text->draw();
}

int main(void)
{
    /* Intialize the Platform object for platform specific functions. */
    Platform* platform = Platform::getInstance();
    /* Initialize windowing system. */
    platform->createWindow(WINDOW_W, WINDOW_H);

    /* Initialize EGL. */
    EGLRuntime::initializeEGL(EGLRuntime::OPENGLES2);
    EGL_CHECK(eglMakeCurrent(EGLRuntime::display, EGLRuntime::surface, EGLRuntime::surface, EGLRuntime::context));

    /* Initialize OpenGL ES graphics subsystem. */
    setupGraphics(WINDOW_W, WINDOW_H);

    /* Timer variable to calculate FPS. */
    Timer fpsTimer;
    fpsTimer.reset();

    bool end = false;
    /* The rendering loop to draw the scene. */
    while(!end)
    {
        /* If something has happened to the window, end the sample. */
        if(platform->checkWindow() != Platform::WINDOW_IDLE)
        {
            end = true;
        }
        
        /* Calculate FPS. */
        float fFPS = fpsTimer.getFPS();
        if(fpsTimer.isTimePassed(1.0f))
        {
            LOGI("FPS:\t%.1f\n", fFPS);
        }

        /* Render a single frame */
        renderFrame();
     
        /* 
         * Push the EGL surface color buffer to the native window.
         * Causes the rendered graphics to be displayed on screen.
         */
        eglSwapBuffers(EGLRuntime::display, EGLRuntime::surface);
    }

    /* Shut down OpenGL ES. */
    /* Shut down Text. */
    delete text;

    /* Shut down EGL. */
    EGLRuntime::terminateEGL();

    /* Shut down windowing system. */
    platform->destroyWindow();

    /* Shut down the Platform object. */
    delete platform;

    return 0;
}
