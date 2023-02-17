#ifndef POSTP
#define POSTP
#include"gsdl.hpp"
namespace pygame{
    using Kernal = glm::mat3;
    Shader krnl;
    void ppInit(){
        krnl.program = loadprogram(L"rsrc/2d_textured_vertex.glsl",L"rsrc/kernalfs.glsl");
    }
    void ppApply(prTexture src, const Framebuffer& dst, Shader& shd=texture_shader, float sz=1.0f){
        dst.bind();
        blit(src, {0.0f,0.0f}, sz, 0, shd);
    }
    Kernal edgeDet{
        1,1,1,
        1,-8,1,
        1,1,1
    };
    float size_fit(float w, float h,float dw=1920.0f, float dh=1080.0f){
        float wmul = dw/w;
        float hmul = dh/h;
        return glm::min(wmul,hmul);
    }
    class Scene{
        Framebuffer fb;
        mutable Framebuffer tmp;
        mutable Renderbuffer dep;
        prTexture graphics;
        mutable Texture* tmphics;
        GLsizei w;
        GLsizei h;
        mutable bool redrew = true;
        float sz;
        public:
            Scene(GLsizei wid, GLsizei hgt,float scaleup=1.0f) : 
            dep(GL_DEPTH24_STENCIL8, wid, hgt), 
            graphics(
                new Texture(
                    nullptr, wid, hgt,
                    GL_RGBA, GL_RGBA,
                    GL_NEAREST, GL_NEAREST,
                    false
                )
            ),
            tmphics(
                new Texture(
                    nullptr, wid, hgt,
                    GL_RGBA, GL_RGBA,
                    GL_NEAREST, GL_NEAREST,
                    false
                )
            )
            , w(wid), h(hgt), sz(scaleup){
                fb.attachRenderbuf(GL_DEPTH_STENCIL_ATTACHMENT,dep);
                fb.attachTexture(*graphics.p);
                tmp.attachTexture(*tmphics);
                if(!fb.isComplete()){
                    throw std::runtime_error("GL internal error: framebuf not complete / Scene::Scene");
                }
                if(!tmp.isComplete()){
                    throw std::runtime_error("GL internal error: tmpbuf not complete / Scene::Scene");
                }
            }
            void bind(bool threed=true) const{
                glDisable(GL_DEPTH_TEST);
                glViewport(0,0,w,h);
                tmp.bind();
                glClear(GL_COLOR_BUFFER_BIT);
                Framebuffer::unbind();
                if(threed)glEnable(GL_DEPTH_TEST);
                redrew = true;
                fb.bind();
            }
            void applyKernel(Kernal k,GLfloat off=1.0f/300.0f){
                glDisable(GL_DEPTH_TEST);
                glProgramUniformMatrix3fv(krnl.program,krnl.getLocation("kernel"),1,false,glm::value_ptr(k));
                glProgramUniform1f(krnl.program,krnl.getLocation("offset"),off);
                prTexture src = (redrew?graphics.p:prTexture(tmphics));
                ppApply(src,tmp,krnl,size_fit(w,h));
                redrew = false;
            }
            void alpha(float a){
                graphics.alpha = a;
            }
            void draw(Point pos={0.0f, 0.0f},GLsizei dw=0, GLsizei dh=0) const{
                glDisable(GL_DEPTH_TEST);
                if(dw!=0&&dh!=0)glViewport(0,0,dw,dh);
                Framebuffer::unbind();
                glClear(GL_COLOR_BUFFER_BIT);
                blit(tmphics,pos,sz);
            }
            ~Scene(){
                delete graphics.p;
                delete tmphics;
            }
            Scene(const Scene&) = delete;
    };
}
#endif
