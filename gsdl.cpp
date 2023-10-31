#include"gsdl.hpp"
namespace pygame{
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
    GLuint loadpsource(std::u8string_view vsrc,std::u8string_view fsrc){
        GLint compiled;
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
    GLuint loadprogram(std::u8string_view vs,std::u8string_view fs){
        return loadpsource(load_string_file(vs),load_string_file(fs));
    }
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
        DrawBuffer::batch_create({&rect_db,&texture_db,&fill_db,&colored_polygon_db,&texture_3d_db});
        rect_db.initialize({
            0.0f,1.0f,
            1.0f,1.0f,
            0.0f,0.0f,
            1.0f,0.0f
        },GL_STATIC_DRAW);
        rect_db.vtx_attribs({2uz});
        
        //2(st) = 2/vertex
        //let's support 400 vtx
        //that's 800*float
        texture_db.initialize(nullptr,800uz,GL_STREAM_DRAW);
        texture_db.vtx_attribs({2uz});
        
        //2(xy)+3(rgb) = 5/vertex
        //let's support 200 vtx
        //that's 1000*float
        fill_db.initialize(nullptr,1000uz,GL_STREAM_DRAW);
        fill_db.vtx_attribs({2uz,3uz});
        
        //2(xy) = 2/vertex
        //let's support 400 vtx
        //that's 800*float
        colored_polygon_db.initialize(nullptr,800uz,GL_STREAM_DRAW);
        colored_polygon_db.vtx_attribs({2uz});

#ifndef PYGAME_NO3D
        //xyz(3)+st(2)=5/vertex
        //let's support 40 vtx
        //that's 200*float
        texture_3d_db.initialize(nullptr,200uz,GL_STREAM_DRAW);
        texture_3d_db.vtx_attribs({3uz,2uz});
#endif
    }
    void blit(const zTexture& image,const Point& location,float size,float rotation,
    const glm::vec4& rgn,Shader& shader,bool flipv){
        glUseProgram(shader.program);
        shader.uimg("img",image.handle());
        shader.uv2("position",location);
        shader.u2f("imgdims",image.width()*size,image.height()*size);
        shader.u1f("rotation",-rotation);
        shader.u1f("transparency",image.alpha());
        shader.u1f("brightness",image.brightness());
        shader.uv4("region",rgn);
        shader.u1ui("flipv",flipv);
        invoke_shader(4uz,shader,rect_db);
        shader.u1ui("flipv",true);
    }
    class LineMetrics{
        float width;
        float max_ascent;
        //NEGATIVE!!
        float min_descent;
        public:
            constexpr LineMetrics(float w,float ma,float md) : width(w), max_ascent(ma),
            min_descent(md){}
            float w() const{
                return width;
            }
            float h() const{
                return max_ascent-min_descent;
            }
            float asc() const{
                return max_ascent;
            }
            float adsc() const{
                return -min_descent;
            }
            float dsc() const{
                return min_descent;
            }
    };
    LineMetrics get_line_metrics(Font& font,const cppp::codepoints& cps){
        float textwidth=0.0f;
        float maxascent=0.0f;
        //NEGATIVE!!
        float mindescent=0.0f;
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
        return {textwidth,maxascent,mindescent};
    }
    void draw_text(Font& font,std::u8string_view text,const Point& position,
                   const Color& color,float size,align algn,v_align valgn){
        rect_db.bind();
        text_shader.use();
        text_shader.uv4("color",color);
        text_shader.u1f("rotation",0.0f);
        std::vector<cppp::codepoints> lines{cppp::split<char32_t>(cppp::codepoints_of(text),U'\n')};
        std::vector<LineMetrics> lms;
        float w{0.0f};
        for(const auto& l : lines){
            LineMetrics& lm{lms.emplace_back(get_line_metrics(font,l))};
            w = std::max(w,lm.w());
        }
        const float h{font.get_height()*lines.size()};
        float y{0.0f};
        if(valgn==v_align::BOTTOM){
            y += h;
        }else if(valgn==v_align::CENTER){
            y += h/2.0f;
        }else{
            assert(valgn==v_align::TOP);
        }
        y *= size;

        float xdelta{0.0f};
        if(algn==align::CENTER){
            xdelta -= w/2.0f;
        }else if(algn==align::RIGHT){
            xdelta -= w;
        }else{
            assert(algn==align::LEFT);
        }
        xdelta *= size;
        cppp::codepoints line;
        cppp::codepoints cps = cppp::codepoints_of(text);
        cps.push_back(U'\n');
        char_tex gly{nullptr};
        float x;
        for(size_t i=0uz;i<lines.size();++i){
            x = xdelta;
            LineMetrics lm{get_line_metrics(font,lines[i])};
            for(const cppp::codepoint& ch : lines[i]){
                try{
                    gly = font.loadChar(ch);
                }catch(FTError&){
                    gly = font.loadChar('?');
                }
                text_shader.uimg("img",gly->tex->handle());
                glm::vec2 sz{gly->tex->width(),gly->tex->height()};
                Point drwpos{x+gly->xoffset*size,y};
                drwpos += position;
                text_shader.uv2("imgdims",sz*size);
                drwpos.y -= gly->ascent*size;
                if(valgn==v_align::CENTER){
                    drwpos.y -= lm.asc()*size/2.0f;
                }
                if(valgn==v_align::BOTTOM){
                    drwpos.y -= lm.asc()*size;
                }
                text_shader.uv2("position",drwpos);
                glDrawArrays(GL_TRIANGLE_STRIP,0,4);
                x += gly->distance*size;
            }
            y += font.get_height()*size;
        }
    }
    namespace draw{
        void linerect(Line in,float thickness,Color color,Shader& shader){
            Point din{in};
            shader.uv2("position",in.a-glm::vec2(0.5f*thickness,0.0f));
            shader.u2f("imgdims",thickness,in.length());
            shader.u2f("rotation_center",0.5f,0.0f);
            shader.u1f("rotation",std::atan2(din.y,din.x)-glm::half_pi<float>());
            shader.uv4("color",color);
            invoke_shader(4u,shader,rect_db);
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
        //WARNING: reassigns the 3D buffer on each call! Not very fast.
        void rect3D(const Context3D& context,const Rect3D& in,const zTexture& texture){
            texture_3d_shader.use();
            texture_3d_shader.um4("view",context.camera.viewmatrix());
            glm::mat4 projmat = glm::perspective(context.fov,context.aspect_ratio,
                                context.near_clip,context.far_clip);
            texture_3d_shader.um4("projection",projmat);
            texture_3d_shader.um4("model",in.modelmatrix());
            texture_3d_db.bind();
            rect3D_nb_nm(in,texture);
        }
        //WARNING: reassigns the 3D buffer on each call! Not very fast.
        void cube(const Context3D& context,const Cube& in,const CubeTexture& textures){
            texture_3d_shader.use();
            texture_3d_shader.um4("view",context.camera.viewmatrix());
            glm::mat4 projmat = glm::perspective(context.fov,context.aspect_ratio,
                                context.near_clip,context.far_clip);
            texture_3d_shader.um4("projection",projmat);
            texture_3d_shader.um4("model",in.modelmatrix());
            texture_3d_db.bind();
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
    }
    namespace time{
        void Clock::tick(double fps){
            double frameTime = 1.0/fps;
            double passedTime = glfwGetTime()-last_tick;
            if(passedTime<frameTime){
                uint64_t mcs{static_cast<uint64_t>((frameTime-passedTime)*1000000)};
                std::this_thread::sleep_for(std::chrono::microseconds(mcs));
            }
            tick_before_last_tick = last_tick;
            last_tick = glfwGetTime();
        }
    }
}
