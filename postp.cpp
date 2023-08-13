#include"postp.hpp"
namespace pygame{
    Scene::Scene(GLsizei wid, GLsizei hgt,float scaleup) :
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
}
