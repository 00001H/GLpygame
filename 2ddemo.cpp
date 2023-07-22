#include<string>
#include<cstdio>
#include"pygame.hpp"
using std::string;
using namespace pygame;
using namespace pygame::event;
using pygame::display::Window;
using pygame::time::Clock;
int main(){
    init();
    glVer(4,6);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    Window win = Window(800,600,"this is a title");
    win.setAsOpenGLTarget();
    drawInit();
    Chlib charlib;
    glViewport(0,0,800,600);
    Font& DEFAULT_FONT = charlib.loadfont("StSong","demorsrc/st_song.ttf");
    DEFAULT_FONT.set_dimensions(0,60);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    Point texpos = {30.0,32.0};
    std::string tt;
    Clock clk;
    glClearColor(0.0f,0.5f,0.75f,1.0f);
    while(!win.shouldClose()){
        glfwPollEvents();
        for(Event evt : win.eventqueue.get()){
            if(evt.type == MOUSEBUTTONDOWN){
                MouseButtonEvent mevt = any_cast<MouseButtonEvent>(evt.value);
                texpos = mevt.pos;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        tt = "CJK Test: 中日韩 Current FPS:";
        char carr[150];
        sprintf(carr,"%.5f",clk.get_fps());
        tt += carr;
        draw_text(DEFAULT_FONT,tt,texpos,Color(1.0f),align::CENTER,v_align::CENTER);
        win.swapBuffers();
        clk.tick(60);
    }
    pygame::quit();
    return 0;
}
