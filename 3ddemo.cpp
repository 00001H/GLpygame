#include<string>
#include<cstdio>
#include<vector>
#include<utility>
#include<any>
#include<iostream>
#include<pygame.hpp>
#include<cppp.hpp>
using std::string;
using std::vector;
using std::any_cast;
using namespace pygame;
using namespace pygame::event;
using pygame::display::Window;
using pygame::time::Clock;
using std::cout;
using std::cin;
using std::endl;
pContext3D ctx = nullptr;
void APIENTRY glDebugOutput(GLenum,GLenum,unsigned int,GLenum,GLsizei,const char*,const void*);
class CubeObject{
    public:
        float yaccel = 0;
        Cube cbe;
        CubeTexture tex;
        CubeObject(Cube c,CubeTexture t) : cbe(c),tex(t) {}
        void draw() const{
            pygame::draw::cube(ctx,cbe,tex);
        }
};
const float PLAYER_REACH = 3.0;
const float GRAVITY=-0.003;
const float PLAYER_HEIGHT=1.75;
const float JUMP_YACCEL=0.09;
const float CAMSENSIT = 0.1f,CAMSPD=0.06f;
float inline constexpr modulo(float x,float y){
    while(x>y)x-=y;
    while(x<y)x+=y;
    return x;
}
float inline constexpr modudist(float x,float y,float mod){
    using glm::abs;
    using glm::min;
    x = modulo(x,mod);
    y = modulo(y,mod);
    return min(abs(x-y),min(abs(x+mod-y),abs(x-mod-y)));
}
static_assert(modudist(0,10,11)==1);

#define GL_DEBUG_CONTEXT false
std::string surr(bool apply, std::string inside){
    if(apply)return "["+inside+"]";
    return " "+inside+" ";
}
std::string seltex(int i){
    std::string a = surr(i==0,"$");
    std::string b = surr(i==1,"$");
    std::string c = surr(i==2,"$");
    return cppp::subst<>("|$ $ $|",{a,b,c});
}
int main(){
    std::shared_ptr<Chlib> pcharlib=nullptr;
    try{
        pcharlib = std::make_shared<Chlib>();
    }catch(FTError &e){
        std::cout << e.what() << std::endl;
        int som;
        std::cin >> som;
    }
    Chlib &charlib = *pcharlib;
    pygame::init();
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    pygame::glVer(4,6);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_DEBUG_CONTEXT);
    Window win(SW,SH,"3D Test",glfwGetPrimaryMonitor());
    win.setAsOpenGLTarget();
    stbi_set_flip_vertically_on_load(true);
    pygame::setupTemplate0();
    #if GL_DEBUG_CONTEXT
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    #endif
    glClearColor(0.7,0.7,0.7,1.0);
    pygame::draw_made_with_glpy(win);
    Font& DEFAULT_FONT = charlib.loadfont("Cnew","rsrc/courier_new.ttf");
    DEFAULT_FONT.set_dimensions(0,45);
    Clock clk;
    Texture* fteximg = loadTexture2D("demorsrc/grid.png");
    Texture* tex = loadTexture2D("demorsrc/a.png");
    CubeTexture ctex = CubeTexture(tex);
    ctex.left = loadTexture2D("demorsrc/b.png");
    ctex.back = loadTexture2D("demorsrc/c.png");
    ctex.back.alpha = 0.7;
    ctex.top.brightness = 0.4;
    CubeTexture floortex = CubeTexture(fteximg);
    Cube floorcube(glm::vec3(-25,-1,-25),50,1,50);
    ctx = new Context3D(70.0f,0.1f,100.0f,win.getWidth(),win.getHeight());
    ctx->init_camera(glm::vec3(2.0,4.0,7.0));
    auto& camera = ctx->camera;
    camera.pitch = -5;
    camera.yaw = -90;
    glfwFocusWindow(win.glfwWindow());
    glfwSetInputMode(win.glfwWindow(),GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    bool Curgrab=true;
    glm::dvec2 lastpos,cupos,dist;
    glfwGetCursorPos(win.glfwWindow(),&lastpos.x,&lastpos.y);
    glClearColor(0.0f,0.5f,0.75f,1.0f);
    vector<CubeObject> objects;
    CubeObject cube1(Cube(glm::vec3(0,0,0),1.0,2.0,1.0),ctex);
    CubeObject cube2(Cube(glm::vec3(10,0,6),1.0,3.0,1.0),ctex);
    CubeObject *grabbing=nullptr;
    objects.push_back(cube1);
    objects.push_back(cube2);
    float player_yaccel=0.0;
    int mind=0;
    int rota=0;
    Scene scene{1920,1080};
    std::string kr0,kr1,kr2;
    int krlis[11]{0};
    krlis[4] = 8;
    krlis[6] = 3;
    krlis[7] = 8;
    int mmsel=3;
    while(!win.shouldClose()){
        glfwPollEvents();
        for(Event evt : win.eventqueue.get()){
            if(evt.type == MOUSEBUTTONDOWN){
                MouseButtonEvent mevt = any_cast<MouseButtonEvent>(evt.value);
                if(mevt.btn==0){
                    glfwSetInputMode(win.glfwWindow(),GLFW_CURSOR,GLFW_CURSOR_DISABLED);
                    Curgrab=true;
                }
            }
            if(evt.type == KEYUP){
                if(any_cast<KeyEvent>(evt.value).glfw_key == GLFW_KEY_ESCAPE){
                    glfwSetInputMode(win.glfwWindow(),GLFW_CURSOR,GLFW_CURSOR_NORMAL);
                    Curgrab=false;
                }
            }
            if(evt.type == KEYDOWN){
                int key = any_cast<KeyEvent>(evt.value).glfw_key;
                if(key == GLFW_KEY_RIGHT){
                    ++mmsel;
                    if(mmsel>=11)mmsel=10;
                }else if(key == GLFW_KEY_LEFT){
                    --mmsel;
                    if(mmsel<0)mmsel=0;
                }else if(key == GLFW_KEY_UP){
                    if(mmsel==6||mmsel==7)mmsel=2;
                    else{
                        if(mmsel>7&&(mmsel-3)<=7)mmsel -= 2;
                        mmsel -= 3;
                        if(mmsel<0)mmsel=0;
                    }
                }else if(key == GLFW_KEY_DOWN){
                    if(mmsel==6||mmsel==7)mmsel=10;
                    else{
                        if(mmsel<6&&(mmsel+3)>=6)mmsel += 2;
                        mmsel += 3;
                        if(mmsel>=11)mmsel=10;
                    }
                }else if(key == GLFW_KEY_T){
                    krlis[mmsel] += 1+win.getKey(GLFW_KEY_LEFT_SHIFT)+win.getKey(GLFW_KEY_LEFT_SHIFT);
                }else if(key == GLFW_KEY_G){
                    krlis[mmsel] -= 1+win.getKey(GLFW_KEY_LEFT_SHIFT)+win.getKey(GLFW_KEY_LEFT_SHIFT);
                    if(mmsel==7&&krlis[mmsel]<1)krlis[mmsel]=1;
                }else if(key == GLFW_KEY_E){
                    if(!Curgrab);
                    else if(grabbing)grabbing=nullptr;
                    else for(CubeObject& obj : objects){
                        float ln = glm::length(obj.cbe.center()-camera.pos);
                        if(ln>PLAYER_REACH)continue;
                        glm::vec3 dest = camera.pos+camera.direction()*ln;
                        ln = glm::length(obj.cbe.center()-dest);
                        if(ln>0.5)continue;
                        grabbing = &obj;
                        int minimangle=0;
                        int mindlta=999;
                        int dlta;
                        for(int angle=0;angle<=270;angle+=90){
                            dlta = modudist(grabbing->cbe.yaw,angle-camera.yaw,360);
                            if(dlta<mindlta){
                                mindlta = dlta;
                                minimangle = angle;
                            }
                        }
                        mind = minimangle;
                        break;
                    }
                }else if(key == GLFW_KEY_SPACE){
                    if(Curgrab&&camera.pos.y-PLAYER_HEIGHT==0){
                        player_yaccel = std::max(player_yaccel,JUMP_YACCEL);
                    }
                }else if(key == GLFW_KEY_R){
                    if(Curgrab)rota += 90;
                }else if(key == GLFW_KEY_Q){
                    if(Curgrab)rota-=90;
                }
            }
        }
        cupos = win.mouse_pos();
        if(grabbing==nullptr)rota=0;
        rota = modulo(rota,360);
        if(Curgrab){
            dist = cupos-lastpos;
            camera.pitch -= dist.y*CAMSENSIT;
            camera.yaw += dist.x*CAMSENSIT;
            while(camera.yaw>360)camera.yaw-=360;
            while(camera.yaw<0)camera.yaw+=360;
            camera.pitch = std::max(-89.9f,std::min(89.9f,camera.pitch));
            camera.pos += camera.xzfront()*CAMSPD*static_cast<float>(win.getKey(GLFW_KEY_W)-win.getKey(GLFW_KEY_S));
            camera.pos += camera.xzright()*CAMSPD*static_cast<float>(win.getKey(GLFW_KEY_D)-win.getKey(GLFW_KEY_A));
            if(grabbing!=nullptr){
                grabbing->cbe.set_center(camera.pos+camera.direction()*PLAYER_REACH);
                grabbing->cbe.pos().y = std::max(grabbing->cbe.pos().y,0.0f);
                grabbing->cbe.yaw = modulo(mind-camera.yaw,360);
            }
        }
        lastpos = cupos;

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        scene.bind(true);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        pygame::draw::cube(ctx,floorcube,floortex);
        for(CubeObject& p : objects){
            p.draw();
        }

        if(Curgrab){
            for(CubeObject& gobj : objects){
                if(&gobj!=grabbing){
                    if(gobj.cbe.pos().y>0){
                        gobj.yaccel += GRAVITY;
                    }else{
                        gobj.cbe.pos().y = 0;
                        gobj.yaccel = std::max<float>(gobj.yaccel,0);
                    }
                    gobj.cbe.pos().y += gobj.yaccel;
                }
            }
            if(camera.pos.y-PLAYER_HEIGHT>0){
                player_yaccel += GRAVITY;
            }else{
                camera.pos.y = PLAYER_HEIGHT;
                player_yaccel = std::max<float>(player_yaccel,0);
            }
            camera.pos.y += player_yaccel;
        }
        scene.applyKernel(pygame::Kernal{krlis[0],krlis[1],krlis[2],krlis[3],krlis[4],krlis[5],krlis[8],krlis[9],krlis[10]}/float(krlis[7]),float(krlis[6])/1000.0f);
        scene.draw({0.0f,0.0f},SW,SH);
        pygame::draw_text(DEFAULT_FONT,"GLpygame 3D demo(ver.0226rc1) all pasterights reserved",{10.0,10.0});
        kr0 = cppp::subst<int>(seltex(mmsel),{krlis[0],krlis[1],krlis[2]})+" ^ ";
        kr1 = cppp::subst<int>(seltex(mmsel-3),{krlis[3],krlis[4],krlis[5]})+surr(mmsel==6,std::to_string(krlis[6]))+surr(mmsel==7,std::to_string(krlis[7]));
        kr2 = cppp::subst<int>(seltex(mmsel-8),{krlis[8],krlis[9],krlis[10]})+" v ";
        pygame::draw_text(DEFAULT_FONT,kr0,{30.0,125.0});
        pygame::draw_text(DEFAULT_FONT,kr1,{30.0,175.0});
        pygame::draw_text(DEFAULT_FONT,kr2,{30.0,225.0});
        win.swap_buffers();
        clk.tick(60);
    }
    pygame::quit();
    return 0;
}
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam){
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    }
    std::endl(std::cout);

    switch (type){
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    }
    std::endl(std::cout);
    
    switch (severity){
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    }
    std::endl(std::endl(std::cout));
}
