#include"glfwPygame.hpp"
namespace pygame{
    void gl_ver(int mjr,int mnr,bool core){
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,mjr);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,mnr);
        glfwWindowHint(GLFW_OPENGL_PROFILE,core?GLFW_OPENGL_CORE_PROFILE:GLFW_OPENGL_COMPAT_PROFILE);
    }
    namespace display{
        void Window::_handle_kpress(GLFWwindow* win,int key,int scan,int action,int mods){
            if(action==GLFW_REPEAT)return;//Ignore system repeats
            winmaps.at(win)->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::KEYDOWN : event::KEYUP),event::KeyEvent(key,scan,mods)));
            if(action==GLFW_PRESS){
                winmaps.at(win)->repeatedKey = key;
                winmaps.at(win)->repeatedScan = scan;
                winmaps.at(win)->repeatedMods = mods;
                winmaps.at(win)->effectiveRepeatedFrames = 0u;
                winmaps.at(win)->repeating = true;
            }else if(key==winmaps.at(win)->repeatedKey){
                winmaps.at(win)->repeating = false;
            }
        }
        void default_resize_fun(Window& w){
            w.restore_viewport();
        }
    }
}
