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
    class DrawBuffer{
        GLuint vao;
        GLuint vbo;
        public:
            DrawBuffer(GLuint vao,GLuint vbo) : vao(vao), vbo(vbo){}
            DrawBuffer() : DrawBuffer(0,0){}
            void assign(GLuint ao,GLuint bo){
                vao = ao;
                vbo = bo;
            }
            void bind() const{
                glBindVertexArray(vao);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
            }
            void create(){
                glGenVertexArrays(1,&vao);
                glGenBuffers(1,&vbo);
            }
            template<size_t count>
            static void batch_create(DrawBuffer* const(&p)[count]){
                GLuint vaos[count];
                GLuint vbos[count];
                glGenVertexArrays(count,vaos);
                glGenBuffers(count,vbos);
                for(size_t i=0uz;i<count;++i){
                    p[i]->assign(vaos[i],vbos[i]);
                    glBindVertexArray(vaos[i]);
                    glBindBuffer(GL_ARRAY_BUFFER,vbos[i]);
                }
                glBindVertexArray(0);
                glBindBuffer(GL_ARRAY_BUFFER,0);
            }
            void initialize(const GLfloat* data,size_t count,GLenum usage){
                glNamedBufferData(vbo,count*sizeof(GLfloat),data,usage);
            }
            template<size_t count>
            void initialize(GLfloat const(&data)[count],GLenum usage){
                initialize(data,count,usage);
            }
            void subdata(const GLfloat* data,size_t count,size_t begin=0uz){
                glNamedBufferSubData(vbo,begin*sizeof(GLfloat),count*sizeof(GLfloat),data);
            }
            void* mmap(GLenum access){
                return glMapNamedBuffer(vbo,access);
            }
            GLfloat* fmmap(GLenum access){
                return static_cast<GLfloat*>(mmap(access));
            }
            struct broken_buffer_error : public error{using error::error;};
            void unmmap(){
                if(glUnmapNamedBuffer(vbo)==GL_FALSE){
                    throw broken_buffer_error(u8"DrawBuffer::unmap(): VBO broken(this is usually not due to a user-side problem)"sv);
                }
            }
            template<size_t count>
            void vtx_attribs(size_t const(&sizes)[count]){
                size_t va_length = 0uz;
                size_t stride = 0uz;
                for(const size_t& l : sizes){
                    stride += l*sizeof(GLfloat);
                }
                glVertexArrayVertexBuffer(vao,0,vbo,0,stride);
                for(size_t i=0uz;i<count;++i){
                    glVertexArrayAttribFormat(vao,i,sizes[i],GL_FLOAT,GL_FALSE,va_length);
                    glVertexArrayAttribBinding(vao,i,0);
                    va_length += sizes[i]*sizeof(GLfloat);
                    glEnableVertexArrayAttrib(vao,i);
                }
            }
    };
    inline DrawBuffer fill_db,texture_db,rect_db,colored_polygon_db,texture_3d_db;
    inline Shader texture_shader,text_shader,fill_shader,single_color_shader;
#ifndef PYGAME_NO3D
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
    inline void invoke_shader_nb(float* data,size_t data_cnt,size_t vert_cnt,GLenum drawmode=GL_TRIANGLE_STRIP){
        glBufferSubData(GL_ARRAY_BUFFER,0,data_cnt*sizeof(float),data);
        glDrawArrays(drawmode,0,vert_cnt);
    }
    inline void invoke_shader(float* data,size_t data_cnt,size_t vert_cnt,const Shader& shdr,DrawBuffer& db,GLenum drawmode=GL_TRIANGLE_STRIP){
        db.bind();
        shdr.use();
        invoke_shader_nb(data,data_cnt,vert_cnt,drawmode);
    }
    inline void invoke_shader(size_t vert_cnt,const Shader& shdr,DrawBuffer& db,GLenum drawmode=GL_TRIANGLE_STRIP){
        db.bind();
        shdr.use();
        glDrawArrays(drawmode,0,vert_cnt);
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
            shader.use();
            shader.u2f("position",in.x,in.y);
            shader.u2f("imgdims",in.w,in.h);
            shader.u1f("size",1.0f);
            shader.u1f("rotation",0.0f);
            shader.uv4("color",color);
            invoke_shader(4uz,shader,rect_db);
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
