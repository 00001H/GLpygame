#ifndef GSDL_H
#define GSDL_H

#include<algorithm>
#include<string>
#include<stdexcept>
#include<memory>

#include<glad/glad.h>
#include<glfw/glfw3.h>
#define GLM_FORCE_SWIZZLE
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include<fileutils.hpp>
#include<chlibs.hpp>
#include<texture.hpp>
#include<3dgeometry.hpp>
#include"cppp.hpp"

namespace pygame{
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    typedef vec2 Point;
    typedef vec4 Color;
    struct Line{
        Point a;
        Point b;
        Line(Point a,Point b) : a(a), b(b) {}
        float distance() const{
            return glm::distance(a,b);
        }
    };
    struct Triangle{
        Point a;
        Point b;
        Point c;
        Triangle(Point a,Point b,Point c) : a(a), b(b), c(c) {}
    };
    struct Rect{
        float x;
        float y;
        float w;
        float h;
        Rect() noexcept = default;
        Rect(float x,float y,float w,float h) : x(x),y(y),w(w),h(h) {}
        bool colliderect(const Rect &other) const{
            return ((x<(other.y+other.w))//left is lefter than other right
                  &&((x+w)>(other.x))//right is righter than other left
                  &&(y<(other.y+other.h))//top is topper than other bottom//note:bigger y = lower
                  &&((y+h)>(other.y)));//bottom is bottomer than other top
        }
        bool collidepoint(const Point& point) const{
            return (((point.x)<(x+w))//lefter than right
                  &&((point.x)>x)//righter than left
                  &&((point.y)<(y+h))//topper than bottom
                  &&((point.y)>y));//bottomer than top
        }
    };
    struct prTexture{
        Texture* p;
        float alpha=1.0;
        float brightness=1.0;
        prTexture(Texture* p) : p(p){}
        prTexture() = default;
        Texture* operator->() noexcept{
            return p;
        }
        const Texture* operator->() const noexcept{
            return p;
        }
    };
    struct CubeTexture{
        public:
            prTexture front;
            prTexture back;
            prTexture top;
            prTexture bottom;
            prTexture left;
            prTexture right;
            CubeTexture(Texture* face) : 
            front(face),back(face),top(face),bottom(face),
            left(face),right(face){}
    };
    Texture* loadTexture2D(const char* filename){
        int w,h,color_chnls;
        unsigned char *data = stbi_load(filename,&w,&h,&color_chnls,0);
        if(data==nullptr){
            throw pygame::error((std::string)"Unable to load texture: "+filename);
        }
        Texture* v = new Texture(data,w,h,(color_chnls==3)?GL_RGB:GL_RGBA,GL_RGBA);
        stbi_image_free(data);
        return v;
    }
    prTexture inline prLoadTexture2D(const char* filename){
        return prTexture(loadTexture2D(filename));
    }
    class Renderbuffer{
        private:
            GLuint renderbuf=-1;
        public:
            Renderbuffer(const Renderbuffer&) = delete;
            Renderbuffer(GLenum fmt,GLsizei w,GLsizei h){
                glGenRenderbuffers(1,&renderbuf);
                glBindRenderbuffer(GL_RENDERBUFFER,renderbuf);
                glRenderbufferStorage(GL_RENDERBUFFER,fmt,w,h);
            }
            auto getId(){
                return renderbuf;
            }
            ~Renderbuffer(){
                glDeleteRenderbuffers(1,&renderbuf);
            }
    };
    class Framebuffer{
        private:
            GLuint fbo;
            int colorattach=0;
        public:
            Framebuffer(){
                glGenFramebuffers(1,&fbo);
            }
            Framebuffer(const Framebuffer&) = delete;
            ~Framebuffer(){
                glDeleteFramebuffers(1,&fbo);
            }
            auto getId() const{
                return fbo;
            }
            bool isComplete(GLenum forMode=GL_FRAMEBUFFER) const{
                return glCheckNamedFramebufferStatus(fbo,forMode)==GL_FRAMEBUFFER_COMPLETE;
            }
            void bind(GLenum target=GL_FRAMEBUFFER) const{
                glBindFramebuffer(target,fbo);
            }
            void attachRenderbuf(GLenum target,Renderbuffer &rbuf){
                glNamedFramebufferRenderbuffer(fbo,target,GL_RENDERBUFFER,rbuf.getId());
            }
            void attachTexture(Texture& texture){
                int attachment_point = GL_COLOR_ATTACHMENT0+(colorattach++);
                if(colorattach>GL_MAX_COLOR_ATTACHMENTS){
                    throw std::logic_error("Too many color attachments for framebuffer "+std::to_string(fbo)+" !");
                }
                glNamedFramebufferTexture(fbo,attachment_point,texture.getId(),0);
            }
            static void unbind(GLenum target=GL_FRAMEBUFFER){
                glBindFramebuffer(target,0);
            }
    };
    GLuint loadprogram(const wchar_t*vs,const wchar_t*fs){
        using namespace std;
        GLint compiled;
        std::wstring vsrcutf16 = loadStringFile(vs);
        std::string vtxsrc(vsrcutf16.begin(),vsrcutf16.end());
        const char *vtxsrc_cstr = vtxsrc.c_str();
        GLuint vshad = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshad,1,&vtxsrc_cstr,NULL);
        glCompileShader(vshad);
        glGetShaderiv(vshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            glGetShaderiv(vshad,GL_INFO_LOG_LENGTH,&ln);
            char infolog[ln];
            glGetShaderInfoLog(vshad,ln,nullptr,infolog);
            throw vshad_compilation_failed(infolog);
        }

        std::wstring fsrcutf16 = loadStringFile(fs);
        std::string frgsrc(fsrcutf16.begin(),fsrcutf16.end());
        const char *frgsrc_cstr = frgsrc.c_str();
        GLuint fshad = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshad,1,&frgsrc_cstr,NULL);
        glCompileShader(fshad);
        glGetShaderiv(fshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            glGetShaderiv(fshad,GL_INFO_LOG_LENGTH,&ln);
            char infolog[ln];
            glGetShaderInfoLog(fshad,ln,nullptr,infolog);
            throw fshad_compilation_failed(infolog);
        }

        GLuint program = glCreateProgram();
        glAttachShader(program,vshad);
        glAttachShader(program,fshad);
        glLinkProgram(program);

        GLint linked;
        glGetProgramiv(program,GL_LINK_STATUS,&linked);
        if(!linked){
            GLint ln;
            glGetShaderiv(fshad,GL_INFO_LOG_LENGTH,&ln);
            char infolog[ln];
            glGetProgramInfoLog(program,ln,nullptr,infolog);
            throw program_linking_failed(infolog);
        }
        glDeleteShader(vshad);
        glDeleteShader(fshad);
        return program;
    }
    struct Shader{
        GLuint program=-1;
        mutable std::unordered_map<std::string,GLint> locations;
        GLint getLocation(const char* location){
            if(locations.count(location)==0){
                locations.emplace(location,glGetUniformLocation(program,location));
            }
            return locations[location];
        }
    };
    GLuint fill_vao,fill_vbo,texture_vao,texture_vbo;
    GLuint colored_polygon_vao,colored_polygon_vbo;
    GLuint texture_3d_vao,texture_3d_vbo;
    Shader texture_shader,text_shader,fill_shader,single_color_shader;
    Shader texture_3d_shader;

    GLfloat simple_quad[]={//GL_TRIANGLE_FAN
        0.0,0.0,0.0,0.0,
        0.0,1.0,0.0,1.0,
        1.0,1.0,1.0,1.0,
        1.0,0.0,1.0,0.0
    };
    void gllInit(){
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            throw error("GLAD load failed! Did your forget to bind an OpenGL context?");
        }
        text_shader.program = loadprogram(L"rsrc/2d_textured_vertex.glsl",L"rsrc/text_rendering_fragment.glsl");
        texture_shader.program = loadprogram(L"rsrc/2d_textured_vertex.glsl",L"rsrc/texture_sampling_fragment.glsl");
        fill_shader.program = loadprogram(L"rsrc/2d_colored_vertex.glsl",L"rsrc/fill_fragment.glsl");
        single_color_shader.program = loadprogram(L"rsrc/2d_vertex.glsl",L"rsrc/single_color_fragment.glsl");
        texture_3d_shader.program = loadprogram(L"rsrc/3d_textured_vertex.glsl",L"rsrc/3d_textured_fragment.glsl");

        glGenVertexArrays(1,&texture_vao);
        glGenBuffers(1,&texture_vbo);
        glBindVertexArray(texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_vbo);
        //2(xy)+2(st) = 4/vertex
        //let's support 20 vtx
        //that's 80*float
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*80,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(GLfloat),(void*)(2*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glGenVertexArrays(1,&fill_vao);
        glGenBuffers(1,&fill_vbo);
        glBindVertexArray(fill_vao);
        glBindBuffer(GL_ARRAY_BUFFER,fill_vbo);
        //2(xy)+3(rgb) = 5/vertex
        //let's support 20 vtx as well
        //that's 100*float
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*100,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)(2*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glGenVertexArrays(1,&colored_polygon_vao);
        glGenBuffers(1,&colored_polygon_vbo);
        glBindVertexArray(colored_polygon_vao);
        glBindBuffer(GL_ARRAY_BUFFER,colored_polygon_vbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*64,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1,&texture_3d_vao);
        glGenBuffers(1,&texture_3d_vbo);
        glBindVertexArray(texture_3d_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_3d_vbo);
        //xyz(3)+st(2)=5/vertex
        //Let's support 20 vtx
        //100 * glfloat
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*100,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

    }
    void gllDeinit(){
        glDeleteProgram(text_shader.program);
        glDeleteProgram(texture_shader.program);
        glDeleteProgram(fill_shader.program);
        glDeleteProgram(single_color_shader.program);
        glDeleteProgram(texture_3d_shader.program);
        glDeleteVertexArrays(1,&texture_vao);
        glDeleteVertexArrays(1,&colored_polygon_vao);
        glDeleteVertexArrays(1,&fill_vao);
        glDeleteVertexArrays(1,&texture_3d_vao);
        glDeleteBuffers(1,&texture_vbo);
        glDeleteBuffers(1,&colored_polygon_vbo);
        glDeleteBuffers(1,&fill_vbo);
        glDeleteBuffers(1,&texture_3d_vbo);
    }
    void blit(prTexture image,Point location,double size,float rotation=0.0f,
    Shader& shader=texture_shader){
        glBindVertexArray(texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_vbo);
        float vtx[16] = {
            0.0                     ,0.0                      ,0.0,1.0,
            0.0                     ,(float)image->getHeight(),0.0,0.0,
            (float)image->getWidth(),0.0                      ,1.0,1.0,
            (float)image->getWidth(),(float)image->getHeight(),1.0,0.0
        };
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
        GLint imglocation = shader.getLocation("img");
        GLint poslocation = shader.getLocation("position");
        GLint sizelocation = shader.getLocation("size");
        GLint rotationlocation = shader.getLocation("rotation");
        GLint transparlocation = shader.getLocation("transparency");
        GLint brightnesslocation = shader.getLocation("brightness");
        glUseProgram(shader.program);
        glUniformHandleui64ARB(imglocation,image->getHandle());
        glUniform2f(poslocation,location.x,location.y);
        glUniform1f(sizelocation,size);
        glUniform1f(rotationlocation,rotation);
        glUniform1f(transparlocation,image.alpha);
        glUniform1f(brightnesslocation,image.brightness);
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    }
    enum class align{
        LEFT,CENTER,RIGHT,
        TOP,BOTTOM
    };
    Rect get_text_rect(Font& font,std::wstring text,Point position,float size,
                   align algn=align::LEFT){
        float textwidth=0.0;
        float topchrs=0.0;
        float bottomchrs=0.0;
        float xdelta=0.0;
        Ch_Texture ch;
        for(wchar_t chr : text){
            try{
                ch = font.loadChar(chr);
            }catch(FTError&){
                ch = font.loadChar(L'?');
            }
            textwidth += (ch.distance/64.0)*size;
            topchrs = std::max(topchrs,size*ch.yoffset);
            bottomchrs = std::min(bottomchrs,size*(ch.yoffset-ch.tex->getHeight()));
        }
        if(algn==align::CENTER){
            xdelta -= textwidth/2.0;
        }else if(algn==align::RIGHT){
            xdelta -= textwidth;
        }else if(algn!=align::LEFT){
            assert(false);
        }
        return Rect(position.x+xdelta,position.y,textwidth,topchrs-bottomchrs);
    }
    
    Rect draw_text(Font& font,std::wstring text,Point position,float size,
                   Color color={1.0,1.0,1.0,1.0},align algn=align::LEFT){
        float topchrs=0.0;
        Ch_Texture ch;
        for(wchar_t chr : text){
            ch = font.loadChar(chr);
            topchrs = std::max(topchrs,size*ch.yoffset);
        }
        glUseProgram(text_shader.program);
        glUniform4f(text_shader.getLocation("color"),color.x,color.y,color.z,color.w);
        glUniform1f(text_shader.getLocation("size"),size);
        glUniform1i(text_shader.getLocation("rotation"),0);
        glBindVertexArray(texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_vbo);
        GLuint imgloc = text_shader.getLocation("img");
        Point charpos,posytion = position;
        Rect tr = get_text_rect(font,text,position,size,algn);
        posytion.x = tr.x;
        Point sz;
        GLint poslocation = text_shader.getLocation("position");
        for(wchar_t chr : text){
            try{
                ch = font.loadChar(chr);
            }catch(FTError&){
                ch = font.loadChar(L'?');
            }
            glUniformHandleui64ARB(imgloc,ch.tex->getHandle());
            sz = Point((float)ch.tex->getWidth(),(float)ch.tex->getHeight());
            float vtx[16] = {
                0.0   ,-sz.y,0.0,0.0,
                0.0   , 0.0 ,0.0,1.0,
                sz.x  ,-sz.y,1.0,0.0,
                sz.x  , 0.0 ,1.0,1.0
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            charpos = posytion;
            charpos.x -= size*ch.xoffset;
            charpos.y += size*(ch.tex->getHeight()-ch.yoffset);
            charpos.y += topchrs;
            glUniform2f(poslocation,charpos.x,charpos.y);
            glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            posytion.x += (ch.distance/64.0)*size;
        }
        return tr;
    }
    namespace draw{
        Rect rect(Rect in,Color color,Shader& shader=single_color_shader){
            glUseProgram(shader.program);
            glBindVertexArray(colored_polygon_vao);
            glBindBuffer(GL_ARRAY_BUFFER,colored_polygon_vbo);
            float vtx[8] = {
                0.0 ,0.0,
                0.0 ,in.h,
                in.w,in.h,
                in.w,0.0
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            glUniform2f(shader.getLocation("position"),in.x,in.y);
            glUniform1f(shader.getLocation("size"),1.0);
            glUniform4f(shader.getLocation("color"),color.x,color.y,color.z,color.w);
            glDrawArrays(GL_TRIANGLE_FAN,0,4);
            return in;
        }
        Triangle triangle(Triangle in,Color color,Shader& shader=single_color_shader){
            glUseProgram(shader.program);
            glBindVertexArray(colored_polygon_vao);
            glBindBuffer(GL_ARRAY_BUFFER,colored_polygon_vbo);
            float vtx[6] = {
                in.a.x,in.a.y,
                in.b.x,in.b.y,
                in.c.x,in.c.y
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            glUniform2f(shader.getLocation("position"),0.0,0.0);
            glUniform1f(shader.getLocation("size"),1.0);
            glUniform4f(shader.getLocation("color"),color.x,color.y,color.z,color.w);
            glDrawArrays(GL_TRIANGLES,0,3);
            return in;
        }
        Line line(Line in,Color color,Shader& shader=single_color_shader){
            glUseProgram(shader.program);
            glBindVertexArray(colored_polygon_vao);
            glBindBuffer(GL_ARRAY_BUFFER,colored_polygon_vbo);
            float vtx[4] = {
                in.a.x,in.a.y,
                in.b.x,in.b.y
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            glUniform2f(shader.getLocation("position"),0.0,0.0);
            glUniform1f(shader.getLocation("size"),1.0);
            glUniform4f(shader.getLocation("color"),color.x,color.y,color.z,color.w);
            glDrawArrays(GL_LINES,0,2);
            return in;
        }
        Rect3D rect3D(pContext3D context,Rect3D in,prTexture text){
            glUseProgram(texture_3d_shader.program);
            glBindVertexArray(texture_3d_vao);
            glBindBuffer(GL_ARRAY_BUFFER,texture_3d_vbo);
            glUniformMatrix4fv(texture_3d_shader.getLocation("view"),1,GL_FALSE,glm::value_ptr(context->camera.viewmatrix()));
            glm::mat4 projmat = glm::perspective(glm::radians(context->fov),context->aspect_ratio,
                                context->near_clip,context->far_clip);
            glUniformMatrix4fv(texture_3d_shader.getLocation("projection"),1,GL_FALSE,glm::value_ptr(projmat));
            glm::mat4 model = in.modelmatrix();
            glUniformMatrix4fv(texture_3d_shader.getLocation("model"),1,GL_FALSE,glm::value_ptr(model));
            glm::vec3 bl = in.bottomleft;
            glm::vec3 br = in.bottomright;
            glm::vec3 tl = in.topleft;
            glm::vec3 tr = in.topright;
            float vtx[20] = {
                tl.x,tl.y,tl.z,1.0,1.0,
                tr.x,tr.y,tr.z,0.0,1.0,
                bl.x,bl.y,bl.z,1.0,0.0,
                br.x,br.y,br.z,0.0,0.0
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),text->getHandle());
            glUniform1f(texture_3d_shader.getLocation("alpha"),text.alpha);
            glUniform1f(texture_3d_shader.getLocation("bright"),text.brightness);
            glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            return in;
        }
        Cube cube(pContext3D context,Cube in,CubeTexture textures){
            glUseProgram(texture_3d_shader.program);
            glBindVertexArray(texture_3d_vao);
            glBindBuffer(GL_ARRAY_BUFFER,texture_3d_vbo);
            glUniformMatrix4fv(texture_3d_shader.getLocation("view"),1,GL_FALSE,glm::value_ptr(context->camera.viewmatrix()));
            glm::mat4 projmat = glm::perspective(glm::radians(context->fov),context->aspect_ratio,
                                context->near_clip,context->far_clip);
            glUniformMatrix4fv(texture_3d_shader.getLocation("projection"),1,GL_FALSE,glm::value_ptr(projmat));
            glm::mat4 model = in.modelmatrix();
            glUniformMatrix4fv(texture_3d_shader.getLocation("model"),1,GL_FALSE,glm::value_ptr(model));
            {
                float vtx[20] = {
                    0.0  ,in.h,0.0,1.0,1.0,
                    in.w,in.h,0.0,0.0,1.0,
                    0.0  ,0.0  ,0.0,1.0,0.0,
                    in.w,0.0  ,0.0,0.0,0.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.back->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.back.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.back.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            {
                float vtx[20] = {
                    0.0  ,in.h,in.l,0.0,1.0,
                    0.0  ,0.0  ,in.l,0.0,0.0,
                    in.w,in.h,in.l,1.0,1.0,
                    in.w,0.0  ,in.l,1.0,0.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.front->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.front.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.front.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            {
                float vtx[20] = {
                    0.0,0.0  ,0.0  ,0.0,0.0,
                    0.0,0.0  ,in.l,1.0,0.0,
                    0.0,in.h,0.0  ,0.0,1.0,
                    0.0,in.h,in.l,1.0,1.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.left->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.left.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.left.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            {
                float vtx[20] = {
                    in.w,0.0  ,0.0  ,1.0,0.0,
                    in.w,in.h,0.0  ,1.0,1.0,
                    in.w,0.0  ,in.l,0.0,0.0,
                    in.w,in.h,in.l,0.0,1.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.right->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.left.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.left.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            {
                float vtx[20] = {
                    0.0  ,in.h,in.l,0.0,0.0,
                    in.w,in.h,in.l,1.0,0.0,
                    0.0  ,in.h,0.0  ,0.0,1.0,
                    in.w,in.h,0.0  ,1.0,1.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.top->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.top.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.top.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            {
                float vtx[20] = {
                    0.0  ,0.0,in.l,0.0,0.0,
                    0.0  ,0.0,0.0  ,0.0,1.0,
                    in.w,0.0,in.l,1.0,0.0,
                    in.w,0.0,0.0  ,1.0,1.0
                };
                glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
                glUniformHandleui64ARB(texture_3d_shader.getLocation("tex"),textures.bottom->getHandle());
                glUniform1f(texture_3d_shader.getLocation("alpha"),textures.bottom.alpha);
                glUniform1f(texture_3d_shader.getLocation("bright"),textures.bottom.brightness);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            }
            return in;
        }
    }//namespace draw;
    namespace time{
        class Clock{
            private:
                double tick_before_last_tick;
                double last_tick;
            public:
                Clock(){
                    last_tick = tick_before_last_tick = glfwGetTime();
                }
                void tick(double fps){
                    while((glfwGetTime()-last_tick)<(1.0/fps));
                    tick_before_last_tick = last_tick;
                    last_tick = glfwGetTime();
                }
                double get_fps() const{
                    if(last_tick==tick_before_last_tick)return 0;
                    return 1.00/(last_tick-tick_before_last_tick);
                }
        };
        void delay(long ms){
            float mss = static_cast<double>(ms);
            double now = glfwGetTime();
            while(((glfwGetTime()-now)*1000.0)<mss);
        }
    }//namespace time;
}//namespace pygame;
namespace std{
    std::string to_string(pygame::Rect rct){
        return (string)"pygame.Rect("+to_string(rct.x)+","+to_string(rct.y)+","+to_string(rct.w)+","+to_string(rct.h)+")";
    }
    std::string to_string(pygame::Point pt){
        return (string)"pygame.Point("+to_string(pt.x)+","+to_string(pt.y)+")";
    }
}//namespace std;
#endif//GSDL_H
