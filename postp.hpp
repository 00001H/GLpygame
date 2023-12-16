/*
DONE: Adapt this to the new OOP system
TODO: Test
*/
#ifndef GLPY_POSTP_HPP
#define GLPY_POSTP_HPP
#include"constants.hpp"
#include"gsdl.hpp"
namespace pygame{
    using Kernal = glm::mat3;
    inline Shader& kernel_shader(Window& w){
        constexpr std::u8string_view ksn{u8"kernel_sh"sv};
        if(!w.hassh(ksn)){
            w.addsh(ksn).loadfile(u8"rsrc/2d_textured_vertex.glsl"sv,u8"rsrc/kernalfs.glsl"sv);
        }
        return w.getsh(ksn);
    }
    inline void post_process(Window& w,const zTexture& src, const Framebuffer& dst, Shader& shd,float sz=1.0f){
        dst.bind();
        w.blit(src,{0.0f,0.0f},(SCRDIMS/src.dims())*sz,0.0f,{0.0f,0.0f,1.0f,1.0f},&shd,false);
    }
    inline Kernal edgeDet{
        1,1,1,
        1,-8,1,
        1,1,1
    };
    inline float size_fit(float w, float h,float dw=1920.0f, float dh=1080.0f){
        float wmul = dw/w;
        float hmul = dh/h;
        return glm::min(wmul,hmul);
    }
    class Scene{
        Window& wn;
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
        Shader* ksh;
        public:
            Scene(Window& w,GLsizei wid, GLsizei hgt,float scaleup=1.0f);
            void bind(bool threed=false) const{
                wn.gl_call(glViewport,0,0,w,h);
                fb.bind();
                if(threed){
                    wn.gl_call(glEnable,GL_DEPTH_TEST);
                    wn.gl_call(glClear,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                }else{
                    wn.gl_call(glDisable,GL_DEPTH_TEST);
                    wn.gl_call(glClear,GL_COLOR_BUFFER_BIT);
                }
                redrew = true;
            }
            void apply_kernel(const Kernal& k,GLfloat off=1.0f/300.0f){
                if(!ksh){
                    ksh = &kernel_shader(wn);
                }
                wn.gl_call(glDisable,GL_DEPTH_TEST);
                ksh->um3("kernel",k);
                ksh->u1f("offset",off);
                post_process(wn,redrew?graphics:tmphics,tmp,*ksh);
                redrew = false;
            }
            void apply(Shader& s){
                post_process(wn,redrew?graphics:tmphics,tmp,s);
            }
            void alpha(float a){
                graphics.alpha() = a;
            }
            void draw(Point pos={0.0f, 0.0f},GLsizei dw=0, GLsizei dh=0) const{
                wn.gl_call(glDisable,GL_DEPTH_TEST);
                fb.unbind();
                if(dw!=0&&dh!=0){
                    wn.gl_call(glViewport,0,0,dw,dh);
                }else if(gl_context){
                    gl_context->restore_viewport();
                }
                wn.gl_call(glClear,GL_COLOR_BUFFER_BIT);
                wn.blit(tmphics,pos,{sz,sz},0.0f,{0.0f,0.0f,1.0f,1.0f},nullptr,false);
            }
            Scene(const Scene&) = delete;
    };
}
#endif
