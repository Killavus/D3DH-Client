//
// Created by shrum on 21.03.18.
//

#ifndef INC_3DHUMANCAPTURE_CALIBRATION_WINDOW_H
#define INC_3DHUMANCAPTURE_CALIBRATION_WINDOW_H

#include "glfw3.h"

class Window {
    static bool glfw_initialized;
    static bool glad_initialized;
private:
    unsigned int res_x,res_y;
    GLFWwindow * window;

    static void initialize_glfw();
    static void terminate_glfw();
    static void initialize_glad();

    void set_opengl();
public:

    Window(unsigned int res_x,unsigned int res_y,const char * title);

    GLFWwindow * getGLFWwindow() const;
    void clear_color();
    void swap_buffers();
    void poll_events();

    unsigned int get_res_x();

    unsigned int get_res_y();

    ~Window();

};

#endif //INC_3DHUMANCAPTURE_CALIBRATION_WINDOW_H
