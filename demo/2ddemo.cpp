#include<string>
#include<cstdio>
#include"pygame.hpp"
using std::u8string;
using namespace pygame;
using namespace pygame::event;
using pygame::Window;
using pygame::time::Clock;
void APIENTRY dbgout(GLenum,GLenum,unsigned int,GLenum,GLsizei,const char*,const void*);
int main(){
{
    glfwInit();
    gl_ver(4,6);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,true);
    Window w{800,600,u8"this is a title"sv};
    w.gl_call(glViewport,0,0,800,600);
    w.gl_call(glEnable,GL_DEBUG_OUTPUT);
    w.gl_call(glEnable,GL_DEBUG_OUTPUT_SYNCHRONOUS);
    w.gl_call(glDebugMessageCallback,dbgout,nullptr);
    w.gl_call(glDebugMessageControl,GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE,0,nullptr,GL_TRUE);
    w.init();
    setup_template_0(w);
    Font& DEFAULT_FONT = pygame::chlib.loadfont(u8"StSong"s,u8"demorsrc/st_song.ttf"s);
    DEFAULT_FONT.set_dimensions(0,60);
    Point texpos = {30.0f,32.0f};
    std::u8string tt;
    w.gl_call(glClearColor,0.0f,0.5f,0.75f,1.0f);
    float s{1.0f};
    v_align a{v_align::TOP};
    Line dm{{345.0f,234.0f},{244.0f,300.0f}};
    while(!w.should_close()){
        w.gl_call(glClear,GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        for(Event evt : w.eventqueue.get()){
            if(evt.type == MOUSEBUTTONDOWN){
                MouseButtonEvent mevt = any_cast<MouseButtonEvent>(evt.value);
                texpos = mevt.pos;
            }else if(evt.type == KEYDOWN){
                KeyEvent const& e{evt.v<KeyEvent>()};
                if(e.is_key(GLFW_KEY_UP)){
                    ++s;
                }else if(e.is_key(GLFW_KEY_DOWN)){
                    --s;
                }else if(e.is_key(GLFW_KEY_T)){
                    a = v_align::TOP;
                }else if(e.is_key(GLFW_KEY_G)){
                    a = v_align::CENTER;
                }else if(e.is_key(GLFW_KEY_B)){
                    a = v_align::BOTTOM;
                }
            }
        }
        tt = u8"QWERTY CJK Test: 中日韩"sv;
        w.rect({SCRCNTR-glm::vec2{5.0f},glm::vec2{10.0f}},CYAN);
        w.rect({{0.0f,0.0f},w.mouse_pos()/3.0f},CYAN);
        w.linerect({SCRCNTR,w.mouse_pos()},12.0f);
        w.linerect(dm,10.0f,dm.above_point(w.mouse_pos())?GREEN:RED);
        w.draw_text(DEFAULT_FONT,tt,texpos,Color(1.0f),s,align::CENTER,a);
        w.swap_buffers();
    }
}//RAII: Destroy allocated textures here, before quitting OpenGL.
    pygame::quit();
    return 0;
}
void APIENTRY dbgout(GLenum source,GLenum type,uint32_t id,GLenum severity, GLsizei,const char* message,const void*){
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    cppp::fcout << u8"---------------"sv << std::endl;
    cppp::fcout << u8"Debug message ("sv << id << u8"): "sv <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             cppp::fcout << u8"Source: API"sv; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   cppp::fcout << u8"Source: Window System"sv; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: cppp::fcout << u8"Source: Shader Compiler"sv; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     cppp::fcout << u8"Source: Third Party"sv; break;
        case GL_DEBUG_SOURCE_APPLICATION:     cppp::fcout << u8"Source: Application"sv; break;
        case GL_DEBUG_SOURCE_OTHER:           cppp::fcout << u8"Source: Other"sv; break;
    }
    std::endl(std::cout);

    switch (type){
        case GL_DEBUG_TYPE_ERROR:               cppp::fcout << u8"Type: Error"sv; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: cppp::fcout << u8"Type: Deprecated Behaviour"sv; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  cppp::fcout << u8"Type: Undefined Behaviour"sv; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         cppp::fcout << u8"Type: Portability"sv; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         cppp::fcout << u8"Type: Performance"sv; break;
        case GL_DEBUG_TYPE_MARKER:              cppp::fcout << u8"Type: Marker"sv; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          cppp::fcout << u8"Type: Push Group"sv; break;
        case GL_DEBUG_TYPE_POP_GROUP:           cppp::fcout << u8"Type: Pop Group"sv; break;
        case GL_DEBUG_TYPE_OTHER:               cppp::fcout << u8"Type: Other"sv; break;
    }
    std::endl(std::cout);
    
    switch (severity){
        case GL_DEBUG_SEVERITY_HIGH:         cppp::fcout << u8"Severity: high"sv; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       cppp::fcout << u8"Severity: medium"sv; break;
        case GL_DEBUG_SEVERITY_LOW:          cppp::fcout << u8"Severity: low"sv; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: cppp::fcout << u8"Severity: notification"sv; break;
    }
    std::endl(std::endl(std::cout));
}
