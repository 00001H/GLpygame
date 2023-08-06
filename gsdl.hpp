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
    sTexture loadTexture2D(std::u8string filename){
        int w,h,color_chnls;
        unsigned char *data = stbi_load(cppp::copy_as_plain(filename).c_str(),&w,&h,&color_chnls,0);
        if(data==nullptr){
            throw pygame::error(u8"Unable to load texture: "s+filename);
        }
        sTexture v{new Texture(data,w,h,(color_chnls==3)?GL_RGB:GL_RGBA,GL_RGBA)};
        if(!v){
            throw pygame::error(u8"Unable to create texture: "s+filename);
        }
        stbi_image_free(data);
        return v;
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
    GLuint loadprogram(const std::u8string_view& vs,const std::u8string_view& fs){
        using namespace std;
        GLint compiled;
        std::u8string vsrc{loadStringFile(vs)};
        std::string vtxsrc{cppp::copy_as_plain(vsrc)};
        const char *vtxsrc_cstr = vtxsrc.c_str();
        GLuint vshad = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshad,1,&vtxsrc_cstr,NULL);
        glCompileShader(vshad);
        glGetShaderiv(vshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            glGetShaderiv(vshad,GL_INFO_LOG_LENGTH,&ln);
            std::string infolog(ln,'\0');
            glGetShaderInfoLog(vshad,ln,nullptr,infolog.data());
            throw vshad_compilation_failed(cppp::copy_as_u8(infolog));
        }

        std::u8string fsrc{loadStringFile(fs)};
        std::string frgsrc{cppp::copy_as_plain(fsrc)};
        const char* frgsrc_cstr = frgsrc.c_str();
        GLuint fshad = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshad,1,&frgsrc_cstr,NULL);
        glCompileShader(fshad);
        glGetShaderiv(fshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            glGetShaderiv(fshad,GL_INFO_LOG_LENGTH,&ln);
            std::string infolog(ln,'\0');
            glGetShaderInfoLog(fshad,ln,nullptr,infolog.data());
            throw fshad_compilation_failed(cppp::copy_as_u8(infolog));
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
            std::string infolog(ln,'\0');
            glGetProgramInfoLog(program,ln,nullptr,infolog.data());
            throw program_linking_failed(cppp::copy_as_u8(infolog));
        }
        glDeleteShader(vshad);
        glDeleteShader(fshad);
        return program;
    }
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
    GLuint fill_vao,fill_vbo,texture_vao,texture_vbo;
    GLuint colored_polygon_vao,colored_polygon_vbo;
    Shader texture_shader,text_shader,fill_shader,single_color_shader;
#ifndef PYGAME_NO3D
    GLuint texture_3d_vao,texture_3d_vbo;
    Shader texture_3d_shader;
#endif
    void setRenderRect(float xmax,float ymax,Shader& sh){
        sh.u1f("hsw",xmax/2.0f);
        sh.u1f("hsh",ymax/2.0f);
    }

    GLfloat simple_quad[]={//GL_TRIANGLE_FAN
        0.0f,0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,1.0f,
        1.0f,1.0f,1.0f,1.0f,
        1.0f,0.0f,1.0f,0.0f
    };
    using namespace std::literals;
    GLuint load_rsrc_program(const std::u8string_view& x,const std::u8string_view& y){
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
    void gllInit(){
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            throw error(u8"GLAD load failed! Did your forget to bind an OpenGL context?"sv);
        }
        text_shader.program = GLPY_PLOAD(u8"2d_textured_vertex.glsl"sv,u8"text_rendering_fragment.glsl"sv);
        texture_shader.program = GLPY_PLOAD(u8"2d_textured_vertex.glsl"sv,u8"texture_sampling_fragment.glsl"sv);
        fill_shader.program = GLPY_PLOAD(u8"2d_colored_vertex.glsl"sv,u8"fill_fragment.glsl"sv);
        single_color_shader.program = GLPY_PLOAD(u8"2d_vertex.glsl"sv,u8"single_color_fragment.glsl"sv);
        
#ifndef PYGAME_NO3D
        texture_3d_shader.program = GLPY_PLOAD(u8"3d_textured_vertex.glsl"sv,u8"3d_textured_fragment.glsl"sv);
#endif
#undef GLPY_PLOAD
        glGenVertexArrays(1,&texture_vao);
        glGenBuffers(1,&texture_vbo);
        glBindVertexArray(texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_vbo);
        //2(st) = 2/vertex
        //let's support 400 vtx
        //that's 800*float
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*800,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1,&fill_vao);
        glGenBuffers(1,&fill_vbo);
        glBindVertexArray(fill_vao);
        glBindBuffer(GL_ARRAY_BUFFER,fill_vbo);
        //2(xy)+3(rgb) = 5/vertex
        //let's support 200 vtx as well
        //that's 1000*float
        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*1000,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(void*)(2*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glGenVertexArrays(1,&colored_polygon_vao);
        glGenBuffers(1,&colored_polygon_vbo);
        glBindVertexArray(colored_polygon_vao);
        glBindBuffer(GL_ARRAY_BUFFER,colored_polygon_vbo);

        //2(xy) = 2/vertex
        //let's support 400 vtx
        //that's 800*float

        glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*800,nullptr,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(GLfloat),(void*)0);
        glEnableVertexAttribArray(0);


#ifndef PYGAME_NO3D
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
#endif
    }
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
    const glm::vec4& rgn = {0.0f,0.0f,1.0f,1.0f},Shader& shader=texture_shader,bool flipv=true){
        float vtx[8] = {
            0.0f,1.0f,
            1.0f,1.0f,
            0.0f,0.0f,
            1.0f,0.0f
        };
        glUseProgram(shader.program);
        shader.uimg("img",image.handle());
        shader.uv2("position",location);
        shader.u2f("imgdims",image.width()*size,image.height()*size);
        shader.u1f("rotation",-rotation);
        shader.u1f("transparency",image.alpha());
        shader.u1f("brightness",image.brightness());
        shader.uv4("region",rgn);
        shader.u1ui("flipv",flipv);
        invoke_shader(vtx,8,4,shader,GL_TRIANGLE_STRIP,texture_vao,texture_vbo);
        shader.u1ui("flipv",true);
    }
    enum class align{
        LEFT,CENTER,RIGHT
    };
    enum class v_align{
        TOP,BASELINE,CENTER,BOTTOM
    };
    Rect get_text_rect(Font& font,const cppp::codepoints& cps,const Point& position,
                   align algn=align::LEFT,v_align valgn=v_align::TOP,
        float* masc=nullptr,float* mdsc=nullptr){
        float textwidth=0.0f;
        float maxascent=0.0f;
        float mindescent=0.0f;
        float xdelta=0.0f,ydelta=0.0f;
        char_tex ch=nullptr;
        for(const cppp::codepoint& chr : cps){
            try{
                ch = font.loadChar(chr);
            }catch(FTError&){
                ch = font.loadChar('?');
            }
            textwidth += ch->distance;
            maxascent = std::max(maxascent,ch->ascent);
            mindescent = std::min(mindescent,ch->descent);
        }
        if(algn==align::CENTER){
            xdelta -= textwidth/2.0f;
        }else if(algn==align::RIGHT){
            xdelta -= textwidth;
        }else{
            assert(algn==align::LEFT);
        }
        float textheight = maxascent-mindescent;
        if(valgn==v_align::BOTTOM){
            ydelta -= textheight;
        }else if(valgn==v_align::CENTER){
            ydelta -= textheight/2.0f;
        }else if(valgn==v_align::BASELINE){
            ydelta -= maxascent;
        }else{
            assert(valgn==v_align::TOP);
        }
        if(masc!=nullptr){
            *masc = maxascent;
        }
        if(mdsc!=nullptr){
            *mdsc = mindescent;
        }
        return Rect(position.x+xdelta,position.y+ydelta,textwidth,textheight);
    }Rect get_text_rect(Font& font,const std::u8string_view& text,const Point& position,
                   align algn=align::LEFT,v_align valgn=v_align::TOP){
        return get_text_rect(font,cppp::codepoints_of(text),position,algn,valgn);
    }
    
    Rect draw_singleline_text(Font& font,const cppp::codepoints& cps,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP){
        char_tex ch=nullptr;
        float maxasc;
        float mindsc;
        text_shader.use();
        text_shader.uv4("color",color);
        text_shader.u1f("size",1.0f);
        text_shader.u1f("rotation",0.0f);
        glBindVertexArray(texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER,texture_vbo);
        GLuint imgloc = text_shader.getLocation("img");
        Point charpos,posytion = position;
        Rect tr = get_text_rect(font,cps,position,algn,valgn,&maxasc,&mindsc);
        posytion.x = tr.x;
        if(valgn==v_align::TOP){
            posytion.y += maxasc;
        }else if(valgn==v_align::BOTTOM){
            posytion.y += mindsc;
        }else if(valgn==v_align::CENTER){
            posytion.y += tr.h/2.0f;
        }
        Point sz;
        for(const cppp::codepoint& chr : cps){
            try{
                ch = font.loadChar(chr);
            }catch(FTError&){
                ch = font.loadChar('?');
            }
            glUniformHandleui64ARB(imgloc,ch->tex->handle());
            sz = Point((float)ch->tex->width(),(float)ch->tex->height());
            text_shader.uv2("imgdims",sz);
            float vtx[8] = {
                0.0f,0.0f,
                0.0f,1.0f,
                1.0f,0.0f,
                1.0f,1.0
            };
            glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vtx),vtx);
            charpos = posytion;
            charpos.x += ch->xoffset;
            charpos.y -= ch->descent+sz.y;
            text_shader.uv2("position",charpos);
            glDrawArrays(GL_TRIANGLE_STRIP,0,4);
            posytion.x += ch->distance;
        }
        return tr;
    }
    Rect draw_singleline_text(Font& font,const std::u8string_view& text,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP){
        return draw_singleline_text(font,cppp::codepoints_of(text),position,color,algn,valgn);
    }
    Rect draw_text(Font& font,const std::u8string_view& text,const Point& position,
                   const Color& color={1.0f,1.0f,1.0f,1.0},align algn=align::LEFT,v_align valgn=v_align::TOP,bool do_render=true){
        using text_line = std::pair<cppp::codepoints,float>;
        std::vector<text_line> lines;
        float y = 0.0f;
        float ydelta = 0.0f;
        Rect bbox;
        Rect tmp;
        bool first=true;
        cppp::codepoints line;
        size_t i=0;
        cppp::codepoints cps = cppp::codepoints_of(text);
        for(const cppp::codepoint& ch : cps){
            if(ch=='\r')continue;
            if(ch!='\n'){
                line += ch;
            }
            ++i;
            if(ch=='\n'||(i>=cps.size())){
                tmp = get_text_rect(font,line,position,algn);
                lines.push_back(std::make_pair(line,y));
                if(first){
                    bbox.x = tmp.x;
                    bbox.y = tmp.y;
                    first=false;
                }
                bbox.w = glm::max(bbox.w,tmp.w);
                bbox.h = y+tmp.h;
                y += font.getHeight();
                line.clear();
            }
        }
        if(valgn==v_align::BOTTOM){
            ydelta -= bbox.h;
        }else if(valgn==v_align::CENTER){
            ydelta -= bbox.h/2.0f;
        }else if(valgn==v_align::BASELINE){
            //NEW: Allow baseline alignment

            // throw pygame::error(u8"Cannot render multiline with BASELINE align"sv);
        }else{
            assert(valgn==v_align::TOP);
        }
        bbox.y += ydelta;
        if(do_render){
            for(const text_line& ln : lines){
                draw_singleline_text(font,ln.first,position+Point(0.0f,ln.second+ydelta),color,algn,valgn==v_align::BASELINE?valgn:v_align::TOP);
            }
        }
        return bbox;
    }
    glm::vec2 getnormalright(glm::vec2 in){
        float ang = glm::atan(in.y,in.x);
        return {glm::cos(ang),glm::sin(ang)};
    }
    namespace draw{
        void rect(Rect in,Color color,Shader& shader=single_color_shader){
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
        void linerect(Line in,float thickness,Color color={1.0f,1.0f,1.0f,1.0f},Shader& shader=single_color_shader){
            float vtx[8uz] = {
                -0.5f,0.0f,
                -0.5f,1.0f,
                 0.5f,0.0f,
                 0.5f,1.0f
            };
            Point din{in};
            shader.uv2("position",in.a);
            shader.u2f("imgdims",thickness,in.length());
            shader.u2f("rotation_center",0.0f,0.0f);
            shader.u1f("size",1.0f);
            shader.u1f("rotation",std::atan2(din.y,din.x)-glm::half_pi<float>());
            shader.uv4("color",color);
            invoke_shader(vtx,8u,4u,shader);
            shader.u2f("rotation_center",0.5f,0.5f);
        }
#ifndef PYGAME_NO3D
        void rect3D_nb_nm(const Rect3D& in,const zTexture& texture){
            texture_3d_shader.uimg("tex",texture.handle());
            texture_3d_shader.u1f("alpha",texture.alpha());
            texture_3d_shader.u1f("bright",texture.brightness());
            const glm::vec3& bl = in.bottomleft;
            const glm::vec3& br = in.bottomright;
            const glm::vec3& tl = in.topleft;
            const glm::vec3& tr = in.topright;
            float vtx[20] = {
                bl.x,bl.y,bl.z,1.0f,1.0f,
                tl.x,tl.y,tl.z,1.0f,0.0f,
                br.x,br.y,br.z,0.0f,1.0f,
                tr.x,tr.y,tr.z,0.0f,0.0f
            };
            invoke_shader_nb(vtx,20u,4u);
        }
        void rect3D(const Context3D& context,const Rect3D& in,const zTexture& texture){
            texture_3d_shader.use();
            glBindVertexArray(texture_3d_vao);
            glBindBuffer(GL_ARRAY_BUFFER,texture_3d_vbo);
            texture_3d_shader.um4("view",context.camera.viewmatrix());
            glm::mat4 projmat = glm::perspective(context.fov,context.aspect_ratio,
                                context.near_clip,context.far_clip);
            texture_3d_shader.um4("projection",projmat);
            texture_3d_shader.um4("model",in.modelmatrix());
            rect3D_nb_nm(in,texture);
        }
        void cube(const Context3D& context,const Cube& in,const CubeTexture& textures){
            texture_3d_shader.use();
            glBindVertexArray(texture_3d_vao);
            glBindBuffer(GL_ARRAY_BUFFER,texture_3d_vbo);
            texture_3d_shader.um4("view",context.camera.viewmatrix());
            glm::mat4 projmat = glm::perspective(context.fov,context.aspect_ratio,
                                context.near_clip,context.far_clip);
            texture_3d_shader.um4("projection",projmat);
            texture_3d_shader.um4("model",in.modelmatrix());
            if(textures.back){
                rect3D_nb_nm(in.back_face(),textures.back);
            }
            if(textures.front){
                rect3D_nb_nm(in.front_face(),textures.front);
            }
            if(textures.left){
                rect3D_nb_nm(in.left_face(),textures.left);
            }
            if(textures.right){
                rect3D_nb_nm(in.right_face(),textures.right);
            }
            if(textures.top){
                rect3D_nb_nm(in.top_face(),textures.top);
            }
            if(textures.bottom){
                rect3D_nb_nm(in.bottom_face(),textures.bottom);
            }
        }
    #endif
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
                void measure(){
                    tick_before_last_tick = last_tick;
                    last_tick = glfwGetTime();
                }
                //Warning: Please turn off vsync before using this function!!!
                void tick(double fps){
                    double frameTime = 1.0/fps;
                    double passedTime = glfwGetTime()-last_tick;
                    if(passedTime<frameTime){
                        uint64_t mcs{static_cast<uint64_t>((frameTime-passedTime)*1000000)};
                        std::this_thread::sleep_for(std::chrono::microseconds(mcs));
                    }
                    tick_before_last_tick = last_tick;
                    last_tick = glfwGetTime();
                }
                double lastFrameTime() const{
                    return last_tick-tick_before_last_tick;
                }
                double get_fps() const{
                    if(last_tick==tick_before_last_tick)return 0;
                    return 1.00/lastFrameTime();
                }
        };
        void delay(long long ms){
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
    }//namespace time;
}//namespace pygame;
namespace std{
    string to_string(pygame::Rect rct){
        return "pygame.Rect("s+to_string(rct.x)+','+to_string(rct.y)+','+to_string(rct.w)+','+to_string(rct.h)+')';
    }
    string to_string(pygame::Point pt){
        return "pygame.Point("s+to_string(pt.x)+','+to_string(pt.y)+')';
    }
}//namespace std;
namespace cppp{
    std::u8string to_u8string(pygame::Rect rct){
        return u8"pygame.Rect("sv+to_u8string(rct.x)+u8','+to_u8string(rct.y)+u8','+to_u8string(rct.w)+u8','+to_u8string(rct.h)+u8')';
    }
    std::u8string to_u8string(pygame::Point pt){
        return u8"pygame.Point("sv+to_u8string(pt.x)+u8','+to_u8string(pt.y)+u8')';
    }
}//namespace std;
#endif//GSDL_H
