#ifndef GLPY_GSDL_HPP
#define GLPY_GSDL_HPP
#include"include.hpp"
#include"fileutils.hpp"
#include"chlibs.hpp"
#include"3dgeometry.hpp"
namespace pygame{
    using namespace std::literals;
    class GLObject{
        private:
            Window* wn;
        protected:
            GLObject(Window& w) : wn(&w){}
            template<typename Fp,typename ...A>
            inline std::invoke_result_t<Fp,A...> gl_call(Fp,A&& ...) const;
    };
    class Texture : public GLObject{
        protected:
            bool resident;
            GLsizei _width;
            GLsizei _height;
            GLenum internalformat;
            GLuint texture;
            GLuint64 texturehandle;
            bool placeholder;
        public:
            glm::vec2 pix2tc(size_t x, size_t y){
                return {float(x)/float(_width),float(y)/float(_height)};
            }
            Texture(Texture&& other) : GLObject(std::move(other)){
                (*this) = std::move(other);
            }
            Texture& operator =(Texture&& other){
                destroy();
                placeholder = other.placeholder;
                other.placeholder = true;
                resident = other.resident;
                _width = other._width;
                _height = other._height;
                internalformat = other.internalformat;
                texture = other.texture;
                texturehandle = other.texturehandle;
                return *this;
            }
            Texture(const Texture&) = delete;
            Texture& operator =(const Texture&) = delete;
            Texture(Window& w,
                unsigned char *data=nullptr,
                GLsizei width=1,
                GLsizei height=1,
                GLenum imageformat=GL_RGBA,
                GLenum internalformat=GL_RGBA,
                GLenum minfilter=GL_NEAREST,
                GLenum magfilter=GL_NEAREST,
                bool mipmap=false,
                GLenum wrap_s=GL_CLAMP_TO_EDGE,
                GLenum wrap_t=GL_CLAMP_TO_EDGE,
                glm::vec4 bordercolor={0.0f,0.0f,0.0f,1.0f}
            );
            bool isNull() const{
                return placeholder;
            }
            void enable(){
                if(!(placeholder||resident)){
                    gl_call(glMakeTextureHandleResidentARB,texturehandle);
                    resident=true;
                }
            }
            void disable(){
                if(resident&&(!placeholder)){
                    gl_call(glMakeTextureHandleNonResidentARB,texturehandle);
                    resident=false;
                }
            }
            float width() const{
                return _width;
            }
            size_t iwidth() const{
                return _width;
            }
            float height() const{
                return _height;
            }
            size_t iheight() const{
                return _height;
            }
            glm::vec2 size() const{
                return {_width,_height};
            }
            float scale_to_fit(float side_length) const{
                return side_length/glm::max(width(),height());
            }
            auto handle() const{
                if(!resident){
                    throw cppp::u8_logic_error(u8"Trying to access a non-resident handle!"sv);
                }
                if(placeholder){
                    throw cppp::u8_logic_error(u8"Trying to use a null texture!"sv);
                }
                return texturehandle;
            }
            auto id() const{
                return texture;
            }
            void destroy(){
                if(!placeholder){
                    disable();
                    gl_call(glDeleteTextures,1,&texture);
                    placeholder = true;
                }
            }
            ~Texture(){
                destroy();
            }
    };
    //Non-owning!
    class zTexture{
        pTexture tex;
        float a;
        float b;
        public:
            zTexture() : tex(nullptr), a(1.0f), b(1.0f){}
            zTexture(const Texture& tex,float alpha=1.0f,float brightness=1.0f) : tex(&tex), a(alpha), b(brightness){}
            auto handle() const{
                return tex->handle();
            }
            float width() const{
                return tex->width();
            }
            float height() const{
                return tex->height();
            }
            float& alpha(){
                return a;
            }
            float alpha() const{
                return a;
            }
            float& brightness(){
                return b;
            }
            float brightness() const{
                return b;
            }
            explicit operator bool() const{
                return tex;
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
    sTexture load_texture(Window& w,std::u8string filename);
    using namespace std::literals;
    class Shader;
    void load_rsrc_program(Shader&,std::u8string_view,std::u8string_view);
    #ifndef GLPY_PLOQE
    #define GLPY_PLOAD load_rsrc_program
    #endif
    enum class align{
        LEFT,CENTER,RIGHT
    };
    enum class v_align{
        TOP,CENTER,BOTTOM
    };
    Rect get_text_rect(Font& font,const cppp::codepoints& cps,const Point& position,
                   align algn=align::LEFT,v_align valgn=v_align::TOP,
        float* masc=nullptr,float* mdsc=nullptr);
    inline Rect get_text_rect(Font& font,std::u8string_view text,const Point& position,
                align algn=align::LEFT,v_align valgn=v_align::TOP,
        float* masc=nullptr,float* mdsc=nullptr){
        return get_text_rect(font,cppp::codepoints_of(text),position,algn,valgn,
            masc,mdsc);
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
    void gl_ver(int mjr,int mnr,bool core=true);
    namespace event{
        enum{
            MOUSEMOTION=0xfa01,MOUSEBUTTONDOWN=0xfa02,MOUSEBUTTONUP=0xfa03,
            KEYDOWN=0xfb01,KEYUP=0xfb02,KEYREPEAT=0xfb03,TEXT=0xfb10,USEREVT=0xff00,
        };
        using EventType = decltype(MOUSEMOTION);
        struct Event{
            int type;
            std::any value;
            Event(int etype,std::any thing) : type(etype),value(thing){}
            template<typename T>
            T& v(){
                return std::any_cast<T&>(value);
            }
            template<typename T>
            const T& v() const{
                return std::any_cast<const T&>(value);
            }
        };
        class Events{
            private:
                std::vector<Event> eventlist;
            public:
                void ignore(){
                    eventlist.clear();
                }
                std::vector<Event> get(){
                    std::vector<Event> retval{std::move(eventlist)};
                    eventlist.clear();
                    return retval;
                }
                void put(Event event){
                    eventlist.push_back(event);
                }
        };
        struct MouseButtonEvent{
            Point pos;
            int btn;
            MouseButtonEvent(Point pos,int btn) : pos(pos),btn(btn) {}
        };
        struct KeyEvent{
            int glfw_key;
            uint32_t scancode;
            uint32_t mods;
            KeyEvent(int key,int scan,uint32_t mods) : glfw_key(key),scancode(scan),mods(mods) {}
            bool is_key(int key,uint32_t _orscan=-1u) const{return (((key==GLFW_KEY_UNKNOWN)||(glfw_key==GLFW_KEY_UNKNOWN))?(scancode==_orscan):(key==glfw_key));}
        };
        struct TextEvent{
            cppp::codepoint ch;
            TextEvent(cppp::codepoint c) : ch(c){}
            void apply(std::u8string& s) const{
                cppp::append_codepoint_to_string(ch,s);
            }
        };
    }
    class Shader : public GLObject{
        GLuint program=0;
        mutable std::unordered_map<std::string,GLint> locations;
        void free_sh(){
            if(program){
                gl_call(glDeleteProgram,program);
                program = 0;
            }
        }
        public:
            void set_render_rect(float xmax,float ymax){
                u1f("hsw",xmax/2.0f);
                u1f("hsh",ymax/2.0f);
            }
            Shader(Window& w) : GLObject(w){}
            ~Shader(){
                free_sh();
            }
            void loadfile(std::u8string_view vs,std::u8string_view fs){
                loadsource(load_string_file(vs),load_string_file(fs));
            }
            void loadsource(std::u8string_view vsrc,std::u8string_view fsrc);
            void update(GLuint newprg){
                free_sh();
                program = newprg;
                locations.clear();
            }
            GLint getLocation(const char* location) const{
                if(locations.count(location)==0){
                    locations.emplace(location,gl_call(glGetUniformLocation,program,location));
                }
                return locations[location];
            }
            void use() const{
                gl_call(glUseProgram,program);
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
    inline Window* gl_context=nullptr;
    class DrawBuffer : public GLObject{
        GLuint vao;
        GLuint vbo;
        public:
            DrawBuffer(Window& wn,GLuint vao,GLuint vbo) : GLObject(wn), vao(vao), vbo(vbo){}
            DrawBuffer(Window& wn) : DrawBuffer(wn,0,0){}
            void assign(GLuint ao,GLuint bo){
                vao = ao;
                vbo = bo;
            }
            void bind() const{
                gl_call(glBindVertexArray,vao);
                gl_call(glBindBuffer,GL_ARRAY_BUFFER,vbo);
            }
            void create(){
                gl_call(glGenVertexArrays,1,&vao);
                gl_call(glGenBuffers,1,&vbo);
            }
            template<size_t count>
            static void batch_create(Window& w,DrawBuffer* const(&p)[count]);
            void initialize(const GLfloat* data,size_t count,GLenum usage){
                gl_call(glNamedBufferData,vbo,count*sizeof(GLfloat),data,usage);
            }
            template<size_t count>
            void initialize(GLfloat const(&data)[count],GLenum usage){
                initialize(data,count,usage);
            }
            void subdata(const GLfloat* data,size_t count,size_t begin=0uz){
                gl_call(glNamedBufferSubData,vbo,begin*sizeof(GLfloat),count*sizeof(GLfloat),data);
            }
            void* mmap(GLenum access){
                return gl_call(glMapNamedBuffer,vbo,access);
            }
            GLfloat* fmmap(GLenum access){
                return static_cast<GLfloat*>(mmap(access));
            }
            struct broken_buffer_error : public error{using error::error;};
            void unmmap(){
                if(gl_call(glUnmapNamedBuffer,vbo)==GL_FALSE){
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
                gl_call(glVertexArrayVertexBuffer,vao,0,vbo,0,stride);
                for(size_t i=0uz;i<count;++i){
                    gl_call(glVertexArrayAttribFormat,vao,i,sizes[i],GL_FLOAT,GL_FALSE,va_length);
                    gl_call(glVertexArrayAttribBinding,vao,i,0);
                    va_length += sizes[i]*sizeof(GLfloat);
                    gl_call(glEnableVertexArrayAttrib,vao,i);
                }
            }
    };
    class window_creation_failed : public cppp::u8_logic_error{
        using cppp::u8_logic_error::u8_logic_error;
    };
    inline std::unordered_map<GLFWwindow*,Window*> winmaps;
    class Window{
        int repeatedKey=0;
        int repeatedScan=0;
        int repeatedMods=0;
        size_t effectiveRepeatedFrames=0u;
        bool repeating=false;
        size_t repeatBegin=47u;
        size_t repeatExec=12u;
        Point toPygameCoords(glm::vec2 in){
            in -= displayframe.ltop();
            return {in.x/displayframe.width()*1920.0f,in.y/displayframe.height()*1080.0f};
        }
        static Point getMousePos(GLFWwindow *win){
            double x,y;
            glfwGetCursorPos(win,&x,&y);
            Window* self = winmaps.at(win);
            return self->toPygameCoords({x,y});
        }
        static void _handle_mmotion(GLFWwindow* win,double x,double y);
        static void _handle_mbutton(GLFWwindow* win,int btn,int action,int){
            winmaps.at(win)->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::MOUSEBUTTONDOWN : event::MOUSEBUTTONUP),event::MouseButtonEvent(getMousePos(win),btn)));
        }
        static void _handle_kpress(GLFWwindow* win,int key,int scan,int action,int mods);
        static void _handle_text(GLFWwindow* win, uint_least32_t ch){
            winmaps.at(win)->eventqueue.put(event::Event(event::TEXT,event::TextEvent(ch)));
        }
        static void _handle_resize(GLFWwindow* win,int wd,int ht){
            winmaps.at(win)->tellResize(wd,ht);
        }
        static void _restore(Window& w){
            w.restore_viewport();
        }
        GLFWwindow* win;
        std::function<void(Window&)> fbcbf;
        GLFWmonitor* fullscreen_monitor;
        GLFWmonitor* current_monitor;
        uint32_t fullscreen_fps = 0;
        GLsizei sw;
        GLsizei sh;
        Rect displayframe;
        mutable bool closed=false;
        float aspect;
        std::vector<std::function<void()>> atd;
        public:
            DrawBuffer fill_db,texture_db,rect_db,colored_polygon_db,
            #ifndef PYGAME_NO3D
            texture_3d_db;
            #endif
            Shader texture_shader,text_shader,fill_shader,single_color_shader;
            #ifndef PYGAME_NO3D
            Shader texture_3d_shader;
            #endif
            template<typename F>
            void atdone(F&& f){
                atd.emplace_back(std::forward<F>(f));
            }
            void draw_text(Font& font,std::u8string_view text,const Point& position,
                        const Color& color={1.0f,1.0f,1.0f,1.0},float size=1.0f,
                        align algn=align::LEFT, v_align valgn=v_align::TOP);
            template<typename Fp,typename ...A>
            std::invoke_result_t<Fp,A...> gl_call(Fp fun,A&& ...ca){
                set_as_OpenGL_target();
                return fun(std::forward<A>(ca)...);
            }
            Rect aspected_viewport(int winwi,int winht,float aspect){
                float wwwi = std::min(float(winwi),float(winht)*aspect);
                float wcwh = wwwi/aspect;
                float wpad = (float(winwi)-wwwi)/2.0f;
                float hpad = (float(winht)-wcwh)/2.0f;
                gl_call(glViewport,int(wpad),int(hpad),int(wwwi),int(wcwh));
                return {Point(wpad,hpad),glm::vec2{wwwi,wcwh}};
            }
            inline void invoke_shader_nb(float* data,size_t data_cnt,size_t vert_cnt,GLenum drawmode=GL_TRIANGLE_STRIP){
                gl_call(glBufferSubData,GL_ARRAY_BUFFER,0,data_cnt*sizeof(float),data);
                gl_call(glDrawArrays,drawmode,0,vert_cnt);
            }
            inline void invoke_shader(float* data,size_t data_cnt,size_t vert_cnt,const Shader& shdr,DrawBuffer& db,GLenum drawmode=GL_TRIANGLE_STRIP){
                db.bind();
                shdr.use();
                invoke_shader_nb(data,data_cnt,vert_cnt,drawmode);
            }
            inline void invoke_shader(size_t vert_cnt,const Shader& shdr,DrawBuffer& db,GLenum drawmode=GL_TRIANGLE_STRIP){
                db.bind();
                shdr.use();
                gl_call(glDrawArrays,drawmode,0,vert_cnt);
            }
            void linerect(Line in,float thickness,Color color={1.0f,1.0f,1.0f,1.0f},Shader* shader=nullptr);
            #ifndef PYGAME_NO3D
            void rect3D_nb_nm(const Rect3D& in,const zTexture& texture);
            void rect3D(const Context3D& context,const Rect3D& in,const zTexture& texture);
            void cube(const Context3D& context,const Cube& in,const CubeTexture& textures);
            #endif
            void blit(const zTexture& image,const Point& location,float size=1.0f,float rotation=0.0f,
            const glm::vec4&rgn={0.0f,0.0f,1.0f,1.0f},Shader* shader=nullptr,bool flipv=true);
            void set_render_rect(float xmax,float ymax){
                texture_shader.set_render_rect(xmax,ymax);
                text_shader.set_render_rect(xmax,ymax);
                fill_shader.set_render_rect(xmax,ymax);
                single_color_shader.set_render_rect(xmax,ymax);
            }
            void rect(Rect in,Color color,float rot=0.0f,Shader* shader=nullptr){
                if(!shader)shader = &single_color_shader;
                shader->use();
                shader->uv2("position",in.ltop());
                shader->uv2("imgdims",in.dims());
                shader->u1f("rotation",rot);
                shader->uv4("color",color);
                invoke_shader(4uz,*shader,rect_db);
            }
            void configure_repeat(size_t bgn,size_t dly){
                repeatBegin = bgn;
                repeatExec = dly;
            }
            event::Events eventqueue;
            void tick_repeats();
            void init();
            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;
            Window(size_t width,size_t height,std::u8string_view title={u8""},GLFWmonitor *monitor=nullptr,GLFWwindow *share=NULL) :
            win(glfwCreateWindow(width,height,cppp::copy_as_plain(title).c_str(),monitor,share)),
            fbcbf(_restore),
            fullscreen_monitor((monitor==nullptr)?glfwGetPrimaryMonitor():monitor),
            current_monitor(monitor), fullscreen_fps(60), sw(width), sh(height),
            displayframe({0.0f,0.0f},{sw,sh}), aspect(float(width)/float(height)),
            fill_db(*this), texture_db(*this), rect_db(*this),
            colored_polygon_db(*this),
            #ifndef PYGAME_NO3D
            texture_3d_db(*this),
            #endif
            texture_shader(*this), text_shader(*this), fill_shader(*this),
            single_color_shader(*this)
            #ifndef PYGAME_NO3D
            ,texture_3d_shader(*this)
            #endif
            {
                static bool loaded=false;
                auto ogc{gl_context};
                if(!win){
                    throw window_creation_failed(u8"Window creation failed!"sv);
                }
                set_as_OpenGL_target();
                if(!loaded){
                    if(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
                        loaded = true;
                    else
                        throw error(u8"GLAD load failed! Did your forget to bind an OpenGL context?"sv);
                }
                if(ogc)ogc->set_as_OpenGL_target();
                winmaps.try_emplace(win,this);
                glfwSetCursorPosCallback(win,_handle_mmotion);
                glfwSetMouseButtonCallback(win,_handle_mbutton);
                glfwSetKeyCallback(win,_handle_kpress);
                glfwSetFramebufferSizeCallback(win,_handle_resize);
                glfwSetCharCallback(win,_handle_text);
            }
            bool isWindowed() const{
                return current_monitor==nullptr;
            }
            bool isFullscreen() const{
                return !isWindowed();
            }
            void setFullscreenMonitor(GLFWmonitor *mon){
                fullscreen_monitor = mon;
            }
            void setFullscreenFPS(int fps){//0 for automatic
                fullscreen_fps = fps;
            }
            void toggleFullscreen(){
                if(isWindowed()){
                    current_monitor = fullscreen_monitor;
                }else{
                    current_monitor = nullptr;
                }
                glfwSetWindowMonitor(win,current_monitor,0,0,sw,sh,
                ((fullscreen_fps==0)?GLFW_DONT_CARE:fullscreen_fps));
            }
            Point mouse_pos() const{
                return getMousePos(win);
            }
            GLFWwindow* glfw_handle() const{
                return win;
            }
            bool should_close() const{
                return glfwWindowShouldClose(win);
            }
            void set_as_OpenGL_target(){
                glfwMakeContextCurrent(win);
                gl_context = this;
            }
            GLsizei width() const{
                return sw;
            }
            GLsizei height() const{
                return sh;
            }
            void restore_viewport(){
                displayframe = aspected_viewport(sw,sh,aspect);
            }
            void onresize(std::function<void(Window&)> func){
                fbcbf = func;
            }
            void swap_buffers() const{
                glfwSwapBuffers(win);
            }
            void close() const{
                if(!closed)glfwDestroyWindow(win);
                closed = true;
            }
            bool get_key(int key) const{
                return glfwGetKey(win,key)==GLFW_PRESS;
            }
            bool get_mouse_button(int btn) const{
                return glfwGetMouseButton(win,btn)==GLFW_PRESS;
            }
            ~Window(){
                for(auto& e : atd){
                    e();
                }
                close();
                if(gl_context==this){
                    gl_context=nullptr;
                }
            }
        private:
            void tellResize(int wd,int ht){
                sw = wd;
                sh = ht;
                fbcbf(*this);
            }
    };
    void default_resize_fun(Window&);
    inline void quit(){
        glfwTerminate();
    }
    template<typename Fp,typename ...A>
    inline std::invoke_result_t<Fp,A...> GLObject::gl_call(Fp glf,A&& ...a) const{
        return wn->gl_call<Fp,A...>(glf,std::forward<A>(a)...);
    }
    class Renderbuffer : public GLObject{
        private:
            GLuint renderbuf=-1;
        public:
            Renderbuffer(const Renderbuffer&) = delete;
            Renderbuffer(Window& ww,GLenum fmt,size_t w,size_t h) : GLObject(ww){
                gl_call(glGenRenderbuffers,1,&renderbuf);
                gl_call(glBindRenderbuffer,GL_RENDERBUFFER,renderbuf);
                gl_call(glRenderbufferStorage,GL_RENDERBUFFER,fmt,w,h);
            }
            auto getId(){
                return renderbuf;
            }
            ~Renderbuffer(){
                gl_call(glDeleteRenderbuffers,1,&renderbuf);
            }
    };
    class Framebuffer : public GLObject{
        private:
            GLuint fbo;
            int colorattach=0;
            mutable bool once_bound=false;
            void error_unbound() const{
                throw pygame::error(u8"New framebuffers needs to be bound at least once before it is valid!"sv);
            }
        public:
            Framebuffer(Window& w) : GLObject(w){
                gl_call(glGenFramebuffers,1,&fbo);
            }
            Framebuffer(const Framebuffer&) = delete;
            ~Framebuffer(){
                gl_call(glDeleteFramebuffers,1,&fbo);
            }
            auto id() const{
                return fbo;
            }
            [[deprecated("Use snake_case instead")]] auto getId() const{
                return id();
            }
            bool is_complete(GLenum forMode=GL_FRAMEBUFFER) const{
                return gl_call(glCheckNamedFramebufferStatus,fbo,forMode)==GL_FRAMEBUFFER_COMPLETE;
            }
            [[deprecated("Use snake_case instead")]] bool isComplete(GLenum forMode=GL_FRAMEBUFFER) const{
                return is_complete(forMode);
            }
            void bind(GLenum target=GL_FRAMEBUFFER) const{
                gl_call(glBindFramebuffer,target,fbo);
                once_bound = true;
            }
            void attach_renderbuffer(GLenum target,Renderbuffer &rbuf){
                if(!once_bound)error_unbound();
                gl_call(glNamedFramebufferRenderbuffer,fbo,target,GL_RENDERBUFFER,rbuf.getId());
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
                gl_call(glNamedFramebufferTexture,fbo,attachment_point,texture.id(),0);
            }
            [[deprecated("Use snake_case instead")]] void attachTexture(const Texture& texture){
                attach_texture(texture);
            }
            void unbind(GLenum target=GL_FRAMEBUFFER) const{
                gl_call(glBindFramebuffer,target,0);
            }
    };
    template<size_t count>
    void DrawBuffer::batch_create(Window& w,DrawBuffer* const(&p)[count]){
        GLuint vaos[count];
        GLuint vbos[count];
        w.gl_call(glGenVertexArrays,count,vaos);
        w.gl_call(glGenBuffers,count,vbos);
        for(size_t i=0uz;i<count;++i){
            p[i]->assign(vaos[i],vbos[i]);
            w.gl_call(glBindVertexArray,vaos[i]);
            w.gl_call(glBindBuffer,GL_ARRAY_BUFFER,vbos[i]);
        }
        w.gl_call(glBindVertexArray,0);
        w.gl_call(glBindBuffer,GL_ARRAY_BUFFER,0);
    }
}//namespace pygame;
namespace std{
    inline string to_string(pygame::Rect rct){
        return "pygame.Rect("s+to_string(rct.left())+','+to_string(rct.top())+','+to_string(rct.width())+','+to_string(rct.height())+')';
    }
    inline string to_string(pygame::Point pt){
        return "pygame.Point("s+to_string(pt.x)+','+to_string(pt.y)+')';
    }
}//namespace std;
namespace cppp{
    inline std::u8string to_u8string(pygame::Rect rct){
        return u8"pygame.Rect("sv+to_u8string(rct.left())+u8','+to_u8string(rct.top())+u8','+to_u8string(rct.height())+u8','+to_u8string(rct.height())+u8')';
    }
    inline std::u8string to_u8string(pygame::Point pt){
        return u8"pygame.Point("sv+to_u8string(pt.x)+u8','+to_u8string(pt.y)+u8')';
    }
}//namespace cppp;
#endif
