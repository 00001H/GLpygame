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
using namespace pygame::constants;
using pygame::display::Window;
using std::cout;
using std::cin;
using std::endl;
void APIENTRY glDebugOutput(GLenum,GLenum,unsigned int,GLenum,GLsizei,const char*,const void*);
class CubeObject{
    public:
        float yaccel = 0.0f;
        const Context3D& ctx;
        Cube cbe;
        CubeTexture tex;
        CubeObject(const Context3D& cc,Cube c,CubeTexture t) : ctx(cc),cbe(c),tex(t) {}
        void draw() const{
            pygame::draw::cube(ctx,cbe,tex);
        }
};
const float PLAYER_REACH = 3.0;
const float GRAVITY=-0.003;
const float PLAYER_HEIGHT=1.75;
const float JUMP_YACCEL=0.09;
const float CAMSENSIT = 0.001f,CAMSPD=0.06f;
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
    pygame::init();
{
    std::shared_ptr<Chlib> pcharlib=nullptr;
    try{
        pcharlib = std::make_shared<Chlib>();
    }catch(FTError &e){
        std::cout << e.what() << std::endl;
        int som;
        std::cin >> som;
    }
    Chlib &charlib = *pcharlib;
    pygame::glVer(4,6);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_DEBUG_CONTEXT);
    Window win{1600,900,"3D Test"};
    win.set_as_OpenGL_target();
    pygame::setupTemplate0();
    #if GL_DEBUG_CONTEXT
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    #endif
    Font& DEFAULT_FONT = charlib.loadfont("Cnew","rsrc/courier_new.ttf");
    DEFAULT_FONT.set_dimensions(0,45);
    sTexture fteximg{loadTexture2D("demorsrc/grid.png")};
    sTexture tex{loadTexture2D("demorsrc/a.png")};
    sTexture tex2{loadTexture2D("demorsrc/b.png")};
    sTexture tex3{loadTexture2D("demorsrc/c.png")};
    sTexture pp{loadTexture2D("demorsrc/p.png")};
    CubeTexture ctex{*tex};
    CubeTexture ptex{*pp};
    ctex.left = *tex2;
    ctex.back = *tex3;
    ctex.back.alpha() = 0.7;
    ctex.top.brightness() = 0.4;
    CubeTexture floortex{*fteximg};
    Cube floorcube{{-25.0f,-1.0f,-25.0f},{50.0f,1.0f,50.0f}};
    Cube pole{{-0.1f,0.0f,-0.1f},{0.2f,1.0f,0.2f}};
    Context3D ctx{Camera(glm::vec3(2.0f,4.0f,7.0f)),glm::radians(70.0f),0.1f,100.0f,float(win.width())/float(win.height())};
    YPR& cam_ang = ctx.camera.angles;
    glm::vec3& cam_pos = ctx.camera.pos;
    cam_ang.pitch = 0.0f;
    cam_ang.yaw = -half_pi;
    glfwFocusWindow(win.glfw_handle());
    glfwSetInputMode(win.glfw_handle(),GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    bool cur_grab=true;
    glm::dvec2 lastpos,cupos,dist;
    lastpos = win.mouse_pos();
    glClearColor(0.0f,0.5f,0.75f,1.0f);
    vector<CubeObject> objects;
    CubeObject cube1(ctx,Cube(glm::vec3(0.0f),{1.0f,2.0f,1.0f}),ctex);
    CubeObject cube2(ctx,Cube({10.0f,0.0f,6.0f},{1.0f,3.0f,1.0f}),ctex);
    CubeObject *grabbing=nullptr;
    objects.push_back(cube1);
    objects.push_back(cube2);
    float player_yaccel=0.0f;
    float mind = 0.0f;
    Scene scene{1920,1080};
    std::string kr0,kr1,kr2;
    int krlis[11]{0};
    krlis[4] = 8;
    krlis[6] = 3;
    krlis[7] = 8;
    int mmsel=3;
    while(!win.should_close()){
        glfwPollEvents();
        floorcube.pos().x += float(win.get_key(GLFW_KEY_RIGHT)-win.get_key(GLFW_KEY_LEFT))*0.01f;
        for(Event evt : win.eventqueue.get()){
            if(evt.type == MOUSEBUTTONDOWN){
                MouseButtonEvent mevt = any_cast<MouseButtonEvent>(evt.value);
                if(mevt.btn==0){
                    glfwSetInputMode(win.glfw_handle(),GLFW_CURSOR,GLFW_CURSOR_DISABLED);
                    cur_grab=true;
                }
            }
            if(evt.type == KEYUP){
                if(any_cast<KeyEvent>(evt.value).glfw_key == GLFW_KEY_ESCAPE){
                    glfwSetInputMode(win.glfw_handle(),GLFW_CURSOR,GLFW_CURSOR_NORMAL);
                    cur_grab=false;
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
                    krlis[mmsel] += 1+win.get_key(GLFW_KEY_LEFT_SHIFT)+win.get_key(GLFW_KEY_LEFT_SHIFT);
                }else if(key == GLFW_KEY_G){
                    krlis[mmsel] -= 1+win.get_key(GLFW_KEY_LEFT_SHIFT)+win.get_key(GLFW_KEY_LEFT_SHIFT);
                    if(mmsel==7&&krlis[mmsel]<1)krlis[mmsel]=1;
                }else if(key == GLFW_KEY_E){
                    if(!cur_grab);
                    else if(grabbing)grabbing=nullptr;
                    else for(CubeObject& obj : objects){
                        float ln = glm::length(obj.cbe.center()-cam_pos);
                        if(ln>PLAYER_REACH)continue;
                        glm::vec3 dest = cam_pos+cam_ang.direction()*ln;
                        ln = glm::length(obj.cbe.center()-dest);
                        if(ln>0.5f)continue;
                        grabbing = &obj;
                        float minimangle = 0.0f;
                        float angle;
                        float mindlta = 999.9f;
                        float dlta;
                        for(size_t _aa=0uz;_aa<4uz;++_aa){
                            angle = float(_aa)*half_pi;
                            dlta = modudist(grabbing->cbe.a().yaw,angle-cam_ang.yaw,tau);
                            if(dlta<mindlta){
                                mindlta = dlta;
                                minimangle = angle;
                            }
                        }
                        mind = minimangle;
                        break;
                    }
                }else if(key == GLFW_KEY_SPACE){
                    if(cur_grab&&cam_pos.y-PLAYER_HEIGHT==0.0f){
                        player_yaccel = std::max(player_yaccel,JUMP_YACCEL);
                    }
                }
            }
        }
        cupos = win.mouse_pos();
        if(cur_grab){
            dist = cupos-lastpos;
            cam_ang.pitch += dist.y*CAMSENSIT;
            cam_ang.yaw += dist.x*CAMSENSIT;
            while(cam_ang.yaw>tau)cam_ang.yaw-=tau;
            while(cam_ang.yaw<0.0f)cam_ang.yaw+=tau;
            cam_ang.pitch = std::clamp(cam_ang.pitch,-half_pi,half_pi);
            cam_pos += cam_ang.ywfront()*CAMSPD*static_cast<float>(win.get_key(GLFW_KEY_W)-win.get_key(GLFW_KEY_S));
            cam_pos += cam_ang.ywright()*CAMSPD*static_cast<float>(win.get_key(GLFW_KEY_D)-win.get_key(GLFW_KEY_A));
            if(grabbing!=nullptr){
                grabbing->cbe.set_center(cam_pos+cam_ang.direction()*PLAYER_REACH);
                grabbing->cbe.pos().y = std::max(grabbing->cbe.pos().y,0.0f);
                grabbing->cbe.a().yaw = modulo(mind-cam_ang.yaw,tau);
            }
        }
        lastpos = cupos;
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        scene.bind(true);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        pygame::draw::cube(ctx,floorcube,floortex);
        pole.pos() = cam_pos-glm::vec3(0.1f,cam_pos.y,0.1f);
        pygame::draw::cube(ctx,pole,ptex);
        pole.pos() = cam_pos+cam_ang.direction()*2.4f-glm::vec3(0.1f,0.0f,0.1f);
        pygame::draw::cube(ctx,pole,ptex);
        for(CubeObject& p : objects){
            p.draw();
            p.cbe.a().yaw += 0.08f;
        }
        if(cur_grab){
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
            if(cam_pos.y-PLAYER_HEIGHT>0){
                player_yaccel += GRAVITY;
            }else{
                cam_pos.y = PLAYER_HEIGHT;
                player_yaccel = std::max<float>(player_yaccel,0);
            }
            cam_pos.y += player_yaccel;
        }
        scene.applyKernel(pygame::Kernal{krlis[0],krlis[1],krlis[2],krlis[3],krlis[4],krlis[5],krlis[8],krlis[9],krlis[10]}/float(krlis[7]),float(krlis[6])/1000.0f);
        scene.draw({0.0f,0.0f},win.width(),win.height());
        pygame::draw_text(DEFAULT_FONT,"GLpygame 3D demo(ver.0226rc1) all pasterights reserved",{10.0f,10.0f});
        pygame::draw_text(DEFAULT_FONT,dumppos(cam_pos),{10.0f,50.0f});
        kr0 = cppp::subst<int>(seltex(mmsel),{krlis[0],krlis[1],krlis[2]})+" ^ ";
        kr1 = cppp::subst<int>(seltex(mmsel-3),{krlis[3],krlis[4],krlis[5]})+surr(mmsel==6,std::to_string(krlis[6]))+surr(mmsel==7,std::to_string(krlis[7]));
        kr2 = cppp::subst<int>(seltex(mmsel-8),{krlis[8],krlis[9],krlis[10]})+" v ";
        pygame::draw_text(DEFAULT_FONT,kr0,{30.0f,125.0f});
        pygame::draw_text(DEFAULT_FONT,kr1,{30.0f,175.0f});
        pygame::draw_text(DEFAULT_FONT,kr2,{30.0f,225.0f});
        pygame::draw::rect(Rect({SCRCNTR-glm::vec2(12.0f,12.0f)},{24.0f,24.0f}),GREEN);
        win.swap_buffers();
    }
}
    pygame::quit();
    return 0;
}
void APIENTRY glDebugOutput(GLenum source,GLenum type,uint32_t id,GLenum severity, GLsizei length,const char *message,const void *userParam){
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
