#ifndef GLFWPYGAME_HPP
#define GLFWPYGAME_HPP
#include"include.hpp"
#include"gsdl.hpp"
namespace pygame{
    namespace{
        bool __is_init=false;
    }
    void drawInit();
    void glVer(int mjr,int mnr,bool core=true){
    #ifndef GLPY_NOWARN_GLVER
        if(mjr<4||(mjr==4&&mnr<=5)){
            std::cerr << "Warning: Using OpenGL<=4.5. Please enable the required OpenGL extensions(DSA and bindless textures) or this library won't work properly. Use #define GLPY_NOWARN_GLVER to supress." << std::endl;
        }
    #endif
        glfwWindowHint(GLFW_CLIENT_API,GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE,core?GLFW_OPENGL_CORE_PROFILE:GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,mjr);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,mnr);
    }
    namespace event{
        enum{
            MOUSEMOTION=0xfa01,MOUSEBUTTONDOWN=0xfa02,MOUSEBUTTONUP=0xfa03,
            KEYDOWN=0xfb01,KEYUP=0xfb02,KEYREPEAT=0xfb03,TEXT=0xfb10,USEREVT=0xff00,
        };
        using EventType = decltype(MOUSEMOTION);
        struct Event{
            int type;
            std::any value;
            Event(int etype,std::any thing) : type(etype),value(thing){}
        };
        class Events{
            private:
                std::vector<Event> eventlist;
            public:
                void add(Event event){
                    eventlist.push_back(event);
                }
                std::vector<Event> get(){
                    std::vector<Event> retval{std::move(eventlist)};
                    eventlist.clear();
                    return retval;
                }
                void put(Event event){
                    eventlist.push_back(event);
                }
        };
        struct MouseButtonEvent{
            Point pos;
            int btn;
            MouseButtonEvent(Point pos,int btn) : pos(pos),btn(btn) {}
        };
        struct KeyEvent{
            int glfw_key;
            uint32_t scancode;
            uint32_t mods;
            KeyEvent(int key,int scan,uint32_t mods) : glfw_key(key),scancode(scan),mods(mods) {}
            bool is_key(int key,uint32_t _orscan=-1u) const{return (((key==GLFW_KEY_UNKNOWN)||(glfw_key==GLFW_KEY_UNKNOWN))?(scancode==_orscan):(key==glfw_key));}
        };
        struct TextEvent{
            char32_t ch;
            TextEvent(char32_t c) : ch(c){}
            TextEvent(uint_least32_t c) : ch(char32_t(c)){}
            void apply(std::string& s){
                if(ch=='\b'){
                    if(!s.empty())s.pop_back();
                }else{
                    s += ch;
                }
            }
        };
    }
    namespace display{
        class Window;
        void default_resize_fun(Window&,int wi,int ht){
            glViewport(0,0,wi,ht);
        }
        class window_creation_failed : public std::logic_error{
            using std::logic_error::logic_error;
        };
        std::unordered_map<GLFWwindow*,Window*> winmaps;
        Window* glCtx=nullptr;
        class Window{
            int repeatedKey=0;
            int repeatedScan=0;
            int repeatedMods=0;
            size_t effectiveRepeatedFrames=0u;
            bool repeating=false;
            size_t repeatBegin=47u;
            size_t repeatExec=12u;
            private:
                static Point toPygameCoords(glm::dvec2 in,double sw,double sh){
                    return {(float)(in.x/sw*1920.0),(float)(in.y/sh*1080.0)};
                }
                static Point getMousePos(GLFWwindow *win){
                    double x,y;
                    glfwGetCursorPos(win,&x,&y);
                    Window *self = winmaps[win];
                    return toPygameCoords(vec2{x,y},self->sw,self->sh);
                }
                static void _Handle_MouseMotion(GLFWwindow* win,double x,double y){
                    Window *self = winmaps[win];
                    self->eventqueue.put(event::Event(event::MOUSEMOTION,toPygameCoords(vec2{x,y},self->sw,self->sh)));
                }
                static void _Handle_MouseButton(GLFWwindow* win,int btn,int action,int){
                    winmaps[win]->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::MOUSEBUTTONDOWN : event::MOUSEBUTTONUP),event::MouseButtonEvent(getMousePos(win),btn)));
                }
                static void _Handle_KeyPress(GLFWwindow* win,int key,int scan,int action,int mods){
                    if(action==GLFW_REPEAT)return;//Ignore system repeats
                    winmaps[win]->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::KEYDOWN : event::KEYUP),event::KeyEvent(key,scan,mods)));
                    if(action==GLFW_PRESS){
                        winmaps[win]->repeatedKey = key;
                        winmaps[win]->repeatedScan = scan;
                        winmaps[win]->repeatedMods = mods;
                        winmaps[win]->effectiveRepeatedFrames = 0u;
                        winmaps[win]->repeating = true;
                    }else if(key==winmaps[win]->repeatedKey){
                        winmaps[win]->repeating = false;
                    }
                }
                static void _Handle_Text(GLFWwindow* win, uint_least32_t ch){
                    winmaps[win]->eventqueue.put(event::Event(event::TEXT,event::TextEvent(ch)));
                }
                static void _Handle_FBResize(GLFWwindow* win,int wd,int ht){
                    winmaps[win]->tellResize(wd,ht);
                }
                GLFWwindow* win = nullptr;
                GLFWmonitor* fullscreen_monitor = nullptr;
                GLFWmonitor* current_monitor = nullptr;
                int fullscreen_fps = 0;
                GLsizei sw,sh;
                std::function<void(Window&,int,int)> fbcbf;
                mutable bool closed=false;
            public:
                void configureRepeat(size_t bgn,size_t dly){
                    repeatBegin = bgn;
                    repeatExec = dly;
                }
                event::Events eventqueue;
                void tickRepeats(){
                    if(repeating){
                        ++effectiveRepeatedFrames;
                        if(effectiveRepeatedFrames>=repeatBegin){
                            if((effectiveRepeatedFrames-repeatBegin)>=repeatExec){
                                effectiveRepeatedFrames = repeatBegin;
                                eventqueue.put(event::Event(event::KEYREPEAT,event::KeyEvent(repeatedKey,repeatedScan,repeatedMods)));
                            }
                        }
                    }
                }
                Window(const Window&) = delete;
                Window& operator=(const Window&) = delete;
                Window(GLsizei width,GLsizei height,const char *title,GLFWmonitor *monitor=nullptr,GLFWwindow *share=NULL){
                    win = glfwCreateWindow(width,height,title,monitor,share);
                    if(win==NULL){
                        throw window_creation_failed("Window creation failed!");
                    }
                    fbcbf = [](Window&,int,int){};
                    winmaps.insert_or_assign(win,this);
                    glfwSetCursorPosCallback(win,_Handle_MouseMotion);
                    glfwSetMouseButtonCallback(win,_Handle_MouseButton);
                    glfwSetKeyCallback(win,_Handle_KeyPress);
                    glfwSetFramebufferSizeCallback(win,_Handle_FBResize);
                    glfwSetCharCallback(win,_Handle_Text);
                    sw = width;
                    sh = height;
                    current_monitor = monitor;
                    fullscreen_monitor = ((monitor==nullptr)?glfwGetPrimaryMonitor():monitor);
                }
                bool isWindowed() const{
                    return current_monitor==nullptr;
                }
                bool isFullscreen() const{
                    return !isWindowed();
                }
                void setFullscreenMonitor(GLFWmonitor *mon){
                    fullscreen_monitor = mon;
                }
                void setFullscreenFPS(int fps){//0 for automatic
                    fullscreen_fps = fps;
                }
                void toggleFullscreen(){
                    if(isWindowed()){
                        current_monitor = fullscreen_monitor;
                    }else{
                        current_monitor = nullptr;
                    }
                    glfwSetWindowMonitor(win,current_monitor,0,0,sw,sh,
                    ((fullscreen_fps==0)?GLFW_DONT_CARE:fullscreen_fps));
                }
                Point mouse_pos() const{
                    return getMousePos(win);
                }
                [[deprecated("Use snake_case instead")]] Point getMousePos() const{
                    return mouse_pos();
                }
                GLFWwindow* glfw_handle() const{
                    return win;
                }
                [[deprecated("Use snake_case instead")]] GLFWwindow* glfwWindow() const{
                    return glfw_handle();
                }
                bool should_close() const{
                    return glfwWindowShouldClose(win);
                }
                [[deprecated("Use snake_case instead")]] bool shouldClose() const{
                    return should_close();
                }
                void set_as_OpenGL_target(){
                    glfwMakeContextCurrent(win);
                    drawInit();
                    glCtx = this;
                }
                [[deprecated("Use snake_case instead")]] void setAsOpenGLTarget(){
                    set_as_OpenGL_target();
                }
                GLsizei width() const{
                    return sw;
                }
                [[deprecated("Use snake_case instead")]] GLsizei getWidth() const{
                    return width();
                }
                GLsizei height() const{
                    return sh;
                }
                [[deprecated("Use snake_case instead")]] GLsizei getHeight() const{
                    return height();
                }
                void onresize(std::function<void(Window&,GLsizei,GLsizei)> func){
                    fbcbf = func;
                }
                void swap_buffers() const{
                    glfwSwapBuffers(win);
                }
                [[deprecated("Use snake_case instead")]] void swapBuffers() const{
                    swap_buffers();
                }
                void close() const{
                    if(!closed)glfwDestroyWindow(win);
                    closed = true;
                }
                bool get_key(int key) const{
                    return glfwGetKey(win,key)==GLFW_PRESS;
                }
                [[deprecated("Use snake_case instead")]] bool getKey(int key) const{
                    return get_key(key);
                }
                bool get_mouse_button(int btn) const{
                    return glfwGetMouseButton(win,btn)==GLFW_PRESS;
                }
                [[deprecated("Use snake_case instead")]] bool getMouseBtn(int btn) const{
                    return get_mouse_button(btn);
                }
                ~Window(){
                    if(__is_init){
                        close();
                    }
                    if(glCtx==this){
                        glCtx=nullptr;
                    }
                }
            private:
                void tellResize(int wd,int ht){
                    sw = double(wd);
                    sh = double(ht);
                    fbcbf(*this,wd,ht);
                }
        };
        void init(){
            if(__is_init)return;
            glfwInit();
            __is_init = true;
        }
        void quit(){
            if(!__is_init)return;
            __is_init = false;
            glfwTerminate();
        }
        typedef std::shared_ptr<Window> pWindow;
    }//namespace display;
}//namespace pygame;
#endif//GLFWPYGAME_H
