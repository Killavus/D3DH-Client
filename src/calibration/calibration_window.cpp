//
// Created by shrum on 21.03.18.
//

#include <cstdio>
#include <cstdlib>
#include <glad/glad.h>

#include "calibration/calibration_window.h"

bool Window::glfw_initialized = false;
bool Window::glad_initialized = false;

void Window::initialize_glfw() {
    if(!glfw_initialized)
    {
        if( !glfwInit() )
        {
            fprintf( stderr, "Failed to initialize GLFW\n" );
            exit(-1);
        }
    }
}

void Window::terminate_glfw() {
    if(glfw_initialized)
    {
        glfwTerminate();
    }
}

void Window::initialize_glad() {
    if(!glad_initialized) {
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            fprintf(stderr, "Failed to initialize GLAD\n");
            terminate_glfw();
            exit(-1);
        } else {
          glad_initialized = true;
        }
    }
}

void Window::set_opengl() {

    glfwWindowHint(GLFW_SAMPLES,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
}

Window::Window(unsigned int res_x, unsigned int res_y, const char *title)
        :res_x(res_x),res_y(res_y)
{
    initialize_glfw();

    set_opengl();

    window = glfwCreateWindow(res_x,res_y,title, nullptr, nullptr);
    if(window == nullptr)
    {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
        terminate_glfw();
        exit(-1);
    }

    glfwMakeContextCurrent(window);

    initialize_glad();

    glEnable(GL_CULL_FACE);
//    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    glfwSetCursorPos(window,res_x/2.0f,res_y/2.0f);
}

GLFWwindow *Window::getGLFWwindow() const {
    return window;
}

void Window::clear_color() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swap_buffers() {
    glfwSwapBuffers(window);
}

void Window::poll_events() {
    glfwPollEvents();
}

unsigned int Window::get_res_x() {
    return res_x;
}

unsigned int Window::get_res_y() {
    return res_y;
}

Window::~Window() {
    terminate_glfw();
}
