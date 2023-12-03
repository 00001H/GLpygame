#include"gsdl.hpp"
namespace pygame{
    using namespace std::literals;
    sTexture load_texture(Window& ww,std::u8string filename){
        int w,h,color_chnls;
        unsigned char *data = stbi_load(cppp::copy_as_plain(filename).c_str(),&w,&h,&color_chnls,0);
        if(data==nullptr){
            throw pygame::error(u8"Unable to load texture: "s+filename);
        }
        sTexture v{new Texture(ww,data,w,h,(color_chnls==3)?GL_RGB:GL_RGBA,GL_RGBA)};
        if(!v){
            throw pygame::error(u8"Unable to create texture: "s+filename);
        }
        stbi_image_free(data);
        return v;
    }
    LineMetrics get_line_metrics(Font& font,const cppp::codepoints& cps){
        float textwidth=0.0f;
        float maxascent=0.0f;
        //NEGATIVE!!
        float mindescent=0.0f;
        char_tex ch=nullptr;
        for(const cppp::codepoint& chr : cps){
            try{
                ch = font.load_char(chr);
            }catch(FTError&){
                ch = font.load_char('?');
            }
            textwidth += ch->dst();
            maxascent = std::max(maxascent,ch->asc());
            mindescent = std::min(mindescent,ch->dsc());
        }
        return {textwidth,maxascent,mindescent};
    }
    glm::vec2 get_text_dims(Font& f,const cppp::codepoints& cps,float* ascender,bool use_fh){
        float wd{0.0f};
        float ht{0.0f};
        size_t l{0uz};
        cppp::tokenize<char32_t>(cps,U'\n',[&](const cppp::codepoints& s,size_t){
            LineMetrics lm{get_line_metrics(f,s)};
            if(ascender&&!l)*ascender = lm.asc();
            wd = std::max(wd,lm.w());
            ht += (use_fh?f.get_height():lm.h());
            ++l;
            return false;
        },false);
        return {wd,ht};
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
    void gl_ver(int mjr,int mnr,bool core){
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,mjr);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,mnr);
        glfwWindowHint(GLFW_OPENGL_PROFILE,core?GLFW_OPENGL_CORE_PROFILE:GLFW_OPENGL_COMPAT_PROFILE);
    }
    void Window::_handle_kpress(GLFWwindow* win,int key,int scan,int action,int mods){
        if(action==GLFW_REPEAT)return;//Ignore system repeats
        winmaps.at(win)->eventqueue.put(event::Event(((action==GLFW_PRESS) ? event::KEYDOWN : event::KEYUP),event::KeyEvent(key,scan,mods)));
        if(action==GLFW_PRESS){
            winmaps.at(win)->repeatedKey = key;
            winmaps.at(win)->repeatedScan = scan;
            winmaps.at(win)->repeatedMods = mods;
            winmaps.at(win)->effectiveRepeatedFrames = 0u;
            winmaps.at(win)->repeating = true;
        }else if(key==winmaps.at(win)->repeatedKey){
            winmaps.at(win)->repeating = false;
        }
    }
    void default_resize_fun(Window& w){
        w.restore_viewport();
    }
    void Window::init(){
        GLPY_PLOAD(text_shader,u8"2d_textured_vertex.glsl"sv,u8"text_rendering_fragment.glsl"sv);
        GLPY_PLOAD(texture_shader,u8"2d_textured_vertex.glsl"sv,u8"texture_sampling_fragment.glsl"sv);
        GLPY_PLOAD(fill_shader,u8"2d_colored_vertex.glsl"sv,u8"fill_fragment.glsl"sv);
        GLPY_PLOAD(single_color_shader,u8"2d_vertex.glsl"sv,u8"single_color_fragment.glsl"sv);
        
        GLPY_PLOAD(texture_3d_shader,u8"3d_textured_vertex.glsl"sv,u8"3d_textured_fragment.glsl"sv);
#undef GLPY_PLOAD
        DrawBuffer::batch_create(*this,{&rect_db,&texture_db,&fill_db,&colored_polygon_db,&texture_3d_db});
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

        //xyz(3)+st(2)=5/vertex
        //let's support 40 vtx
        //that's 200*float
        texture_3d_db.initialize(nullptr,200uz,GL_STREAM_DRAW);
        texture_3d_db.vtx_attribs({3uz,2uz});
    }
    void Window::rect3D_nb_nm(const Rect3D& in,const zTexture& texture){
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
    void Window::rect3D(const Context3D& context,const Rect3D& in,const zTexture& texture){
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
    void Window::cube(const Context3D& context,const Cube& in,const CubeTexture& textures){
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
    void Window::linerect(Line in,float thickness,Color color,Shader* shader){
        if(!shader)shader = &single_color_shader;
        Point din{in};
        shader->uv2("position",in.a-glm::vec2(0.5f*thickness,0.0f));
        shader->u2f("imgdims",thickness,in.length());
        shader->u2f("rotation_center",0.5f,0.0f);
        shader->u1f("rotation",std::atan2(din.y,din.x)-glm::half_pi<float>());
        shader->uv4("color",color);
        invoke_shader(4u,*shader,rect_db);
        shader->u2f("rotation_center",0.5f,0.5f);
    }
    void Window::draw_text(Font& font,std::u8string_view text,const Point& position,
                const Color& color,float size,align algn,v_align valgn){
        rect_db.bind();
        text_shader.use();
        text_shader.uv4("color",color);
        text_shader.u1f("rotation",0.0f);
        std::vector<cppp::codepoints> lines{cppp::split<char32_t>(cppp::codepoints_of(text),U'\n')};
        std::vector<LineMetrics> lms;
        float w{0.0f};
        float h{0.0f};
        for(const auto& l : lines){
            LineMetrics& lm{lms.emplace_back(get_line_metrics(font,l))};
            w = std::max(w,lm.w());
            h += lm.h();
        }
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
            LineMetrics& lm{lms[i]};
            for(const cppp::codepoint& ch : lines[i]){
                try{
                    gly = font.load_char(ch);
                }catch(FTError&){
                    gly = font.load_char('?');
                }
                text_shader.uimg("img",gly->tex(*this)->handle());
                glm::vec2 sz{gly->tex(*this)->width(),gly->tex(*this)->height()};
                Point drwpos{x+gly->xoff()*size,y};
                drwpos += position;
                text_shader.uv2("imgdims",sz*size);
                drwpos.y -= gly->asc()*size;
                if(valgn==v_align::CENTER){
                    drwpos.y += lm.dsc()*size;
                }
                if(valgn==v_align::BOTTOM){
                    drwpos.y -= lm.asc()*size;
                }
                text_shader.uv2("position",drwpos);
                gl_call(glDrawArrays,GL_TRIANGLE_STRIP,0,4);
                x += gly->dst()*size;
            }
            y += lm.h()*size;
        }
    }
    void Window::tick_repeats(){
        if(repeating){
            ++effectiveRepeatedFrames;
            if(effectiveRepeatedFrames>=repeatBegin){
                if((effectiveRepeatedFrames-repeatBegin)>=repeatExec){
                    effectiveRepeatedFrames = repeatBegin;
                    eventqueue.put(event::Event(event::KEYREPEAT,event::KeyEvent(repeatedKey,repeatedScan,repeatedMods)));
                }
            }
        }
    }
    void Window::_handle_mmotion(GLFWwindow* win,double x,double y){
        Window* self = winmaps.at(win);
        self->eventqueue.put(event::Event(event::MOUSEMOTION,self->toPygameCoords({x,y})));
    }
    void Window::blit(const zTexture& image,const Point& location,glm::vec2 size,float rotation,
    const glm::vec4& rgn,Shader* shader,bool flipv){
        if(!shader)shader = &texture_shader;
        shader->use();
        shader->uimg("img",image.handle());
        shader->uv2("position",location);
        shader->u2f("imgdims",image.width()*size.x*(rgn.z-rgn.x),image.height()*size.y*(rgn.w-rgn.y));
        shader->u1f("rotation",-rotation);
        shader->u1f("transparency",image.alpha());
        shader->u1f("brightness",image.brightness());
        shader->uv4("region",rgn);
        shader->u1ui("flipv",flipv);
        invoke_shader(4uz,*shader,rect_db);
        shader->u1ui("flipv",true);
    }
    void Shader::loadsource(std::u8string_view vsrc,std::u8string_view fsrc){
        GLint compiled{0};
        std::string vtxsrc{cppp::copy_as_plain(vsrc)};
        const char *vtxsrc_cstr = vtxsrc.c_str();
        GLuint vshad = gl_call(glCreateShader,GL_VERTEX_SHADER);
        gl_call(glShaderSource,vshad,1,&vtxsrc_cstr,nullptr);
        gl_call(glCompileShader,vshad);
        gl_call(glGetShaderiv,vshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            gl_call(glGetShaderiv,vshad,GL_INFO_LOG_LENGTH,&ln);
            std::string infolog(ln,'\0');
            gl_call(glGetShaderInfoLog,vshad,ln,nullptr,infolog.data());
            throw vshad_compilation_failed(cppp::copy_as_u8(infolog));
        }

        std::string frgsrc{cppp::copy_as_plain(fsrc)};
        const char* frgsrc_cstr = frgsrc.c_str();
        GLuint fshad = gl_call(glCreateShader,GL_FRAGMENT_SHADER);
        gl_call(glShaderSource,fshad,1,&frgsrc_cstr,nullptr);
        gl_call(glCompileShader,fshad);
        gl_call(glGetShaderiv,fshad,GL_COMPILE_STATUS,&compiled);
        if(!compiled){
            GLint ln;
            gl_call(glGetShaderiv,fshad,GL_INFO_LOG_LENGTH,&ln);
            std::string infolog(ln,'\0');
            gl_call(glGetShaderInfoLog,fshad,ln,nullptr,infolog.data());
            throw fshad_compilation_failed(cppp::copy_as_u8(infolog));
        }

        program = gl_call(glCreateProgram);
        gl_call(glAttachShader,program,vshad);
        gl_call(glAttachShader,program,fshad);
        gl_call(glLinkProgram,program);
        gl_call(glDeleteShader,vshad);
        gl_call(glDeleteShader,fshad);

        GLint linked{0};
        gl_call(glGetProgramiv,program,GL_LINK_STATUS,&linked);
        if(!linked){
            GLint ln;
            gl_call(glGetShaderiv,fshad,GL_INFO_LOG_LENGTH,&ln);
            std::string infolog(ln,'?');
            gl_call(glGetProgramInfoLog,program,ln,nullptr,infolog.data());
            throw program_linking_failed(cppp::copy_as_u8(infolog));
        }
    }
    void load_rsrc_program(Shader& s,std::u8string_view x,std::u8string_view y){
        cppp::dirpath p = x;
        cppp::dirpath q = y;
        cppp::dirpath rp = "rsrc"s/p;
        cppp::dirpath rq = "rsrc"s/q;
        s.loadfile(rp.u8string(),rq.u8string());
    }
    Texture::Texture(Window& w,
                unsigned char *data,
                GLsizei width,GLsizei height,
                GLenum imageformat,GLenum internalformat,
                GLenum minfilter,GLenum magfilter,
                bool mipmap,GLenum wrap_s,GLenum wrap_t,glm::vec4 bordercolor) : GLObject(w), _width(width), _height(height),
            internalformat(internalformat), texture(0), placeholder(false){
        gl_call(glGenTextures,1,&texture);
        gl_call(glBindTexture,GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,0,internalformat,width,height,0,imageformat,GL_UNSIGNED_BYTE,data);
        if(mipmap)gl_call(glGenerateMipmap,GL_TEXTURE_2D);
        gl_call(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrap_s);
        gl_call(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrap_t);
        const float borderc[] = {bordercolor.r,bordercolor.g,bordercolor.b,bordercolor.a};
        gl_call(glTexParameterfv,GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderc);
        gl_call(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minfilter);
        gl_call(glTexParameteri,GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,magfilter);
        texturehandle = gl_call(glGetTextureHandleARB,texture);
        resident = false;
        enable();
    }
}
