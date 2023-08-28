#ifndef GLFWPYGAME_HPP
#define GLFWPYGAME_HPP
#include"include.hpp"
#include"gsdl.hpp"
namespace pygame{
    namespace{
        bool _is_init=false;
    }
    void drawInit();
    void glVer(int mjr,int mnr,bool core=true);
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
                void ignore(){
                    eventlist.clear();
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
            cppp::codepoint ch;
            TextEvent(cppp::codepoint c) : ch(c){}
            void apply(std::u8string& s) const{
                cppp::appendCodepointToString(ch,s);
            }
        };
    }
    namespace display{
        class Window;
        inline Window* glCtx=nullptr;
        inline Rect aspected_viewport(int winwi,int winht,float aspect){
            float wwwi = std::min(float(winwi),float(winht)*aspect);
            float wcwh = wwwi/aspect;
            float wpad = (float(winwi)-wwwi)/2.0f;
            float hpad = (float(winht)-wcwh)/2.0f;
            glViewport(int(wpad),int(hpad),int(wwwi),int(wcwh));
            return {Point(wpad,hpad),glm::vec2{wwwi,wcwh}};
        }
        class window_creation_failed : public cppp::u8_logic_error{
            using cppp::u8_logic_error::u8_logic_error;
        };
        inline std::unordered_map<GLFWwindow*,Window*> winmaps;
        class Window{
            int repeatedKey=0;
            int repeatedScan=0;
            int repeatedMods=0;
            size_t effectiveRepeatedFrames=0u;
            bool repeating=false;
            size_t repeatBegin=47u;
            size_t repeatExec=12u;
            Point toPygameCoords(glm::vec2 in){
                in -= displayframe.ltop();
                return {in.x/displayframe.w*1920.0f,in.y/displayframe.h*1080.0f};
            }
            static Point getMousePos(GLFWwindow *win){
                double x,y;
                glfwGetCursorPos(win,&x,&y);
                Window* self = winmaps.at(win);
                return self->toPygameCoords({x,y});
            }
            static void _handle_mmotion(GLFWwindow* win,double x,double y){
                Window* self = winmaps.at(win);
                self->eventqueue.put(event::Event(event::MOUSEMOTION,self->toPygameCoords({x,y})));
            }
            static void _handle_mbutton(GLFWwindow* win,int btn,int action,int){
                winmaps.at(win)->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::MOUSEBUTTONDOWN : event::MOUSEBUTTONUP),event::MouseButtonEvent(getMousePos(win),btn)));
            }
            static void _handle_kpress(GLFWwindow* win,int key,int scan,int action,int mods);
            static void _handle_text(GLFWwindow* win, uint_least32_t ch){
                winmaps.at(win)->eventqueue.put(event::Event(event::TEXT,event::TextEvent(ch)));
            }
            static void _handle_resize(GLFWwindow* win,int wd,int ht){
                winmaps.at(win)->tellResize(wd,ht);
            }
            static void _restore(Window& w){
                w.restore_viewport();
            }
            GLFWwindow* win;
            std::function<void(Window&)> fbcbf;
            GLFWmonitor* fullscreen_monitor;
            GLFWmonitor* current_monitor;
            uint32_t fullscreen_fps = 0;
            GLsizei sw;
            GLsizei sh;
            Rect displayframe;
            mutable bool closed=false;
            float aspect;
            public:
                void configure_repeat(size_t bgn,size_t dly){
                    repeatBegin = bgn;
                    repeatExec = dly;
                }
                event::Events eventqueue;
                void tick_repeats(){
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
                [[deprecated("Use snake_case instead")]] void tickRepeats(){
                    tick_repeats();
                }
                Window(const Window&) = delete;
                Window& operator=(const Window&) = delete;
                Window(GLsizei width,GLsizei height,const std::u8string_view& title,GLFWmonitor *monitor=nullptr,GLFWwindow *share=NULL) :
                win(glfwCreateWindow(width,height,cppp::copy_as_plain(title).c_str(),monitor,share)),
                fbcbf(_restore),
                fullscreen_monitor((monitor==nullptr)?glfwGetPrimaryMonitor():monitor),
                current_monitor(monitor), fullscreen_fps(60), sw(width), sh(height),
                displayframe({0.0f,0.0f},{sw,sh}), aspect(float(width)/float(height)){
                    if(win==NULL){
                        throw window_creation_failed(u8"Window creation failed!"sv);
                    }
                    winmaps.try_emplace(win,this);
                    glfwSetCursorPosCallback(win,_handle_mmotion);
                    glfwSetMouseButtonCallback(win,_handle_mbutton);
                    glfwSetKeyCallback(win,_handle_kpress);
                    glfwSetFramebufferSizeCallback(win,_handle_resize);
                    glfwSetCharCallback(win,_handle_text);
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
                void restore_viewport(){
                    displayframe = aspected_viewport(sw,sh,aspect);
                }
                void onresize(std::function<void(Window&)> func){
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
                    if(_is_init){
                        close();
                    }
                    if(glCtx==this){
                        glCtx=nullptr;
                    }
                }
            private:
                void tellResize(int wd,int ht){
                    sw = wd;
                    sh = ht;
                    fbcbf(*this);
                }
        };
        void default_resize_fun(Window&);
        inline void init(){
            if(_is_init)return;
            glfwInit();
            _is_init = true;
        }
        inline void quit(){
            if(!_is_init)return;
            _is_init = false;
            glfwTerminate();
        }
        typedef std::shared_ptr<Window> pWindow;
    }//namespace display;
}//namespace pygame;
#endif//GLFWPYGAME_H
