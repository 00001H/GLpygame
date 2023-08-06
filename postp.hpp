#ifndef GLPY_POSTP_HPP
#define GLPY_POSTP_HPP
#include"gsdl.hpp"
namespace pygame{
    using Kernal = glm::mat3;
    Shader krnl;
    void ppInit(){
        krnl.program = loadprogram(u8"rsrc/2d_textured_vertex.glsl"sv,u8"rsrc/kernalfs.glsl"sv);
    }
    void ppApply(const zTexture& src, const Framebuffer& dst, Shader& shd=texture_shader, float sz=1.0f){
        dst.bind();
        blit(src,{0.0f,0.0f},sz,0.0f,{0.0f,0.0f,1.0f,1.0f},shd,false);
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
        sTexture _graphics;
        zTexture graphics;
        sTexture _tmphics;
        zTexture tmphics;
        GLsizei w;
        GLsizei h;
        mutable bool redrew = true;
        float sz;
        public:
            Scene(GLsizei wid, GLsizei hgt,float scaleup=1.0f) : 
            dep(GL_DEPTH24_STENCIL8, wid, hgt), 
            _graphics(
                new Texture(
                    nullptr, wid, hgt,
                    GL_RGBA, GL_RGBA,
                    GL_NEAREST, GL_NEAREST,
                    false
                )
            ), graphics(*_graphics)
            , _tmphics(
                new Texture(
                    nullptr, wid, hgt,
                    GL_RGBA, GL_RGBA,
                    GL_NEAREST, GL_NEAREST,
                    false
                )
            ), tmphics(*_tmphics)
            , w(wid), h(hgt), sz(scaleup){
                fb.bind();
                tmp.bind();
                Framebuffer::unbind();
                fb.attach_renderbuffer(GL_DEPTH_STENCIL_ATTACHMENT,dep);
                fb.attach_texture(*_graphics);
                tmp.attach_texture(*_tmphics);
                if(!fb.is_complete()){
                    throw cppp::u8_runtime_error(u8"GL internal error: framebuf not complete / Scene::Scene"sv);
                }
                if(!tmp.is_complete()){
                    throw cppp::u8_runtime_error(u8"GL internal error: tmpbuf not complete / Scene::Scene"sv);
                }
            }
            void bind(bool threed=false) const{
                glViewport(0,0,w,h);
                fb.bind();
                if(threed){
                    glEnable(GL_DEPTH_TEST);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                }else{
                    glDisable(GL_DEPTH_TEST);
                    glClear(GL_COLOR_BUFFER_BIT);
                }
                redrew = true;
            }
            void applyKernel(const Kernal& k,GLfloat off=1.0f/300.0f){
                glDisable(GL_DEPTH_TEST);
                krnl.um3("kernel",k);
                krnl.u1f("offset",off);
                ppApply(redrew?graphics:tmphics,tmp,krnl,size_fit(w,h));
                redrew = false;
            }
            void alpha(float a){
                graphics.alpha() = a;
            }
            void draw(Point pos={0.0f, 0.0f},GLsizei dw=0, GLsizei dh=0) const{
                glDisable(GL_DEPTH_TEST);
                Framebuffer::unbind();
                if(dw!=0&&dh!=0){
                    glViewport(0,0,dw,dh);
                }else if(display::glCtx){
                    display::glCtx->restore_viewport();
                }
                glClear(GL_COLOR_BUFFER_BIT);
                blit(tmphics,pos,sz,0.0f,{0.0f,0.0f,1.0f,1.0f},texture_shader,false);
            }
            Scene(const Scene&) = delete;
    };
}
#endif
