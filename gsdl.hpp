#ifndef GLPY_GSDL_HPP
#define GLPY_GSDL_HPP
#include"include.hpp"
#include"fileutils.hpp"
#include"chlibs.hpp"
#include"texture.hpp"
#ifndef PYGAME_NO3D
#include"3dgeometry.hpp"
#endif
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
        operator glm::vec2() const{
            return b-a;
        }
        float length() const{
            return glm::distance(a,b);
        }
    };
    struct Rect{
        float x;
        float y;
        float w;
        float h;
        Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f){}
        Rect(float x,float y,float w,float h) : x(x),y(y),w(w),h(h) {}
        Rect(Point pos, glm::vec2 dims) : x(pos.x), y(pos.y), w(dims.x), h(dims.y){}
        Rect reposition(Point origin){
            Rect nw = *this;
            nw.x = origin.x;
            nw.y = origin.y;
            return nw;
        }
        Point center() const{
            return {x+w/2.0f,y+h/2.0f};
        }
        bool colliderect(Rect other) const{
            return ((x<(other.y+other.w))//left is lefter than other right
                  &&((x+w)>(other.x))//right is righter than other left
                  &&(y<(other.y+other.h))//top is topper than other bottom//note:bigger y = lower
                  &&((y+h)>(other.y)));//bottom is bottomer than other top
        }
        bool collidepoint(Point point) const{
            return (((point.x)<(x+w))//lefter than right
                  &&((point.x)>x)//righter than left
                  &&((point.y)<(y+h))//topper than bottom
                  &&((point.y)>y));//bottomer than top
        }
    };
#ifndef PYGAME_NO3D
    struct CubeTexture{
        public:
            zTexture front;
            zTexture back;
            zTexture top;
            zTexture bottom;
            zTexture left;
            zTexture right;
            CubeTexture(const zTexture& face) : 
            front(face),back(face),top(face),bottom(face),
            left(face),right(face){}
    };
#endif
    sTexture loadTexture2D(std::u8string filename);
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
            mutable bool once_bound=false;
            void error_unbound() const{
                throw pygame::error(u8"New framebuffers needs to be bound at least once before it is valid!"sv);
            }
        public:
            Framebuffer(){
                glGenFramebuffers(1,&fbo);
            }
            Framebuffer(const Framebuffer&) = delete;
            ~Framebuffer(){
                glDeleteFramebuffers(1,&fbo);
            }
            auto id() const{
                return fbo;
            }
            [[deprecated("Use snake_case instead")]] auto getId() const{
                return id();
            }
            bool is_complete(GLenum forMode=GL_FRAMEBUFFER) const{
                return glCheckNamedFramebufferStatus(fbo,forMode)==GL_FRAMEBUFFER_COMPLETE;
            }
            [[deprecated("Use snake_case instead")]] bool isComplete(GLenum forMode=GL_FRAMEBUFFER) const{
                return is_complete(forMode);
            }
            void bind(GLenum target=GL_FRAMEBUFFER) const{
                glBindFramebuffer(target,fbo);
                once_bound = true;
            }
            void attach_renderbuffer(GLenum target,Renderbuffer &rbuf){
                if(!once_bound)error_unbound();
                glNamedFramebufferRenderbuffer(fbo,target,GL_RENDERBUFFER,rbuf.getId());
            }
            [[deprecated("Use snake_case instead")]] void attachRenderbuf(GLenum target,Renderbuffer &rbuf){
                attach_renderbuffer(target,rbuf);
            }
            void attach_texture(const Texture& texture){
                if(!once_bound)error_unbound();
                int attachment_point = GL_COLOR_ATTACHMENT0+(colorattach++);
                if(colorattach>GL_MAX_COLOR_ATTACHMENTS){
                    throw pygame::error(u8"Too many color attachments for framebuffer "s+cppp::to_u8string(fbo)+u8'!');
                }
                glNamedFramebufferTexture(fbo,attachment_point,texture.id(),0);
            }
            [[deprecated("Use snake_case instead")]] void attachTexture(const Texture& texture){
                attach_texture(texture);
            }
            static void unbind(GLenum target=GL_FRAMEBUFFER){
                glBindFramebuffer(target,0);
            }
    };
    GLuint loadprogram(const std::u8string_view& vs,const std::u8string_view& fs);
    struct Shader{
        GLuint program=-1;
        mutable std::unordered_map<std::string,GLint> locations;
        GLint getLocation(const char* location) const{
            if(locations.count(location)==0){
                locations.emplace(location,glGetUniformLocation(program,location));
            }
            return locations[location];
        }
        void use() const{
            glUseProgram(program);
        }
        void u1f(const char* var, float x) const{
            glProgramUniform1f(program,getLocation(var),x);
        }
        void u1ftime(const char* var="time", float mul=1.0f) const{
            u1f(var,glfwGetTime()*mul);
        }
        void u1i(const char* var, GLint x) const{
            glProgramUniform1i(program,getLocation(var),x);
        }
        void u1ui(const char* var, GLuint x) const{
            glProgramUniform1ui(program,getLocation(var),x);
        }
        void u2f(const char* var, GLfloat x, GLfloat y) const{
            glProgramUniform2f(program,getLocation(var),x,y);
        }
        void uv2(const char* var, glm::vec2 v2) const{
            glProgramUniform2fv(program,getLocation(var),1,glm::value_ptr(v2));
        }
        void uv3(const char* var, glm::vec3 v3) const{
            glProgramUniform3fv(program,getLocation(var),1,glm::value_ptr(v3));
        }
        void um3(const char* var, glm::mat3 m3) const{
            glProgramUniformMatrix3fv(program,getLocation(var),1,GL_FALSE,glm::value_ptr(m3));
        }
        void u4f(const char* var, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const{
            glProgramUniform4f(program,getLocation(var),x,y,z,w);
        }
        void uv4(const char* var, glm::vec4 v4) const{
            glProgramUniform4fv(program,getLocation(var),1,glm::value_ptr(v4));
        }
        void um4(const char* var, glm::mat4 m4) const{
            glProgramUniformMatrix4fv(program,getLocation(var),1,GL_FALSE,glm::value_ptr(m4));
        }
        void uimg(const char* var, GLuint64 hndl) const{
            glProgramUniformHandleui64ARB(program,getLocation(var),hndl);
        }
    };
    inline GLuint fill_vao,fill_vbo,texture_vao,texture_vbo;
    inline GLuint colored_polygon_vao,colored_polygon_vbo;
    inline Shader texture_shader,text_shader,fill_shader,single_color_shader;
#ifndef PYGAME_NO3D
    inline GLuint texture_3d_vao,texture_3d_vbo;
    inline Shader texture_3d_shader;
#endif
    inline void setRenderRect(float xmax,float ymax,Shader& sh){
        sh.u1f("hsw",xmax/2.0f);
        sh.u1f("hsh",ymax/2.0f);
    }

    using namespace std::literals;
    inline GLuint load_rsrc_program(const std::u8string_view& x,const std::u8string_view& y){
        cppp::dirpath p = x;
        cppp::dirpath q = y;
        cppp::dirpath rp = "rsrc"s/p;
        cppp::dirpath rq = "rsrc"s/q;
        std::u8string pstr = rp.u8string();
        std::u8string qstr = rq.u8string();
        return loadprogram(pstr.c_str(),qstr.c_str());
    }
    #ifndef GLPY_PLOQE
    #define GLPY_PLOAD load_rsrc_program
    #endif
    void gllInit();
    inline void invoke_shader_nb(float* data,unsigned int data_cnt,unsigned int vert_cnt,GLenum drawmode=GL_TRIANGLE_STRIP){
        glBufferSubData(GL_ARRAY_BUFFER,0,data_cnt*sizeof(float),data);
        glDrawArrays(drawmode,0,vert_cnt);
    }
    inline void invoke_shader(float* data,unsigned int data_cnt,unsigned int vert_cnt,const Shader& shdr,GLenum drawmode=GL_TRIANGLE_STRIP, GLuint vao=colored_polygon_vao, GLuint vbo=colored_polygon_vbo){
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        shdr.use();
        invoke_shader_nb(data,data_cnt,vert_cnt,drawmode);
    }
    void blit(const zTexture& image,const Point& location,float size=1.0f,float rotation=0.0f,
    const glm::vec4&rgn={0.0f,0.0f,1.0f,1.0f},Shader& shader=texture_shader,bool flipv=true);
    enum class align{
        LEFT,CENTER,RIGHT
    };
    enum class v_align{
        TOP,BASELINE,CENTER,BOTTOM
    };
    Rect get_text_rect(Font& font,const cppp::codepoints& cps,const Point& position,
                   align algn=align::LEFT,v_align valgn=v_align::TOP,
        float* masc=nullptr,float* mdsc=nullptr);
    inline Rect get_text_rect(Font& font,const std::u8string_view& text,const Point& position,
                   align algn=align::LEFT,v_align valgn=v_align::TOP){
        return get_text_rect(font,cppp::codepoints_of(text),position,algn,valgn);
    }
    Rect draw_singleline_text(Font& font,const cppp::codepoints& cps,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP);
    inline Rect draw_singleline_text(Font& font,const std::u8string_view& text,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP){
        return draw_singleline_text(font,cppp::codepoints_of(text),position,color,algn,valgn);
    }
    Rect draw_text(Font& font,const std::u8string_view& text,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP,bool do_render=true);
    namespace draw{
        inline void rect(Rect in,Color color,Shader& shader=single_color_shader){
            float vtx[8] = {
                0.0f,0.0f,
                0.0f,1.0f,
                1.0f,0.0f,
                1.0f,1.0f
            };
            shader.use();
            shader.u2f("position",in.x,in.y);
            shader.u2f("imgdims",in.w,in.h);
            shader.u1f("size",1.0f);
            shader.u1f("rotation",0.0f);
            shader.uv4("color",color);
            invoke_shader(vtx,8u,4u,shader);
        }
        void linerect(Line in,float thickness,Color color={1.0f,1.0f,1.0f,1.0f},Shader& shader=single_color_shader);
#ifndef PYGAME_NO3D
        void rect3D_nb_nm(const Rect3D& in,const zTexture& texture);
        void rect3D(const Context3D& context,const Rect3D& in,const zTexture& texture);
        void cube(const Context3D& context,const Cube& in,const CubeTexture& textures);
#endif
    }
    namespace time{
        class Clock{
            private:
                double tick_before_last_tick;
                double last_tick;
            public:
                Clock(){
                    last_tick = tick_before_last_tick = glfwGetTime();
                }
                void measure(){
                    tick_before_last_tick = last_tick;
                    last_tick = glfwGetTime();
                }
                //Warning: Please turn off vsync before using this function!!!
                void tick(double fps);
                double last_frame_time() const{
                    return last_tick-tick_before_last_tick;
                }
                double get_fps() const{
                    if(last_tick==tick_before_last_tick)return 0;
                    return 1.00/last_frame_time();
                }
        };
        inline void delay(long long ms){
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
    }//namespace time;
}//namespace pygame;
namespace std{
    inline string to_string(pygame::Rect rct){
        return "pygame.Rect("s+to_string(rct.x)+','+to_string(rct.y)+','+to_string(rct.w)+','+to_string(rct.h)+')';
    }
    inline string to_string(pygame::Point pt){
        return "pygame.Point("s+to_string(pt.x)+','+to_string(pt.y)+')';
    }
}//namespace std;
namespace cppp{
    inline std::u8string to_u8string(pygame::Rect rct){
        return u8"pygame.Rect("sv+to_u8string(rct.x)+u8','+to_u8string(rct.y)+u8','+to_u8string(rct.w)+u8','+to_u8string(rct.h)+u8')';
    }
    inline std::u8string to_u8string(pygame::Point pt){
        return u8"pygame.Point("sv+to_u8string(pt.x)+u8','+to_u8string(pt.y)+u8')';
    }
}//namespace std;
#endif//GSDL_H
