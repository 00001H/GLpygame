#include<string>
#include<cstdio>
#include"pygame.hpp"
using std::u8string;
using namespace pygame;
using namespace pygame::event;
using pygame::display::Window;
using pygame::time::Clock;
int main(){
    init();
{
    glVer(4,6);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    Window win{800,600,u8"this is a title"sv};
    win.set_as_OpenGL_target();
    drawInit();
    Chlib charlib;
    glViewport(0,0,800,600);
    Font& DEFAULT_FONT = charlib.loadfont(u8"StSong"sv,u8"demorsrc/st_song.ttf"sv);
    DEFAULT_FONT.set_dimensions(0,60);
    setupTemplate0();
    Point texpos = {30.0,32.0};
    std::u8string tt;
    glClearColor(0.0f,0.5f,0.75f,1.0f);
    while(!win.should_close()){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        for(Event evt : win.eventqueue.get()){
            if(evt.type == MOUSEBUTTONDOWN){
                MouseButtonEvent mevt = any_cast<MouseButtonEvent>(evt.value);
                texpos = mevt.pos;
            }
        }
        tt = u8"CJK Test: 中日韩"sv;
        draw::linerect({SCRCNTR,win.mouse_pos()},12.0f);
        draw_text(DEFAULT_FONT,tt,texpos,Color(1.0f),align::CENTER,v_align::CENTER);
        win.swap_buffers();
    }
}//RAII: Destroy allocated textures here, before quitting OpenGL.
    pygame::quit();
    return 0;
}
