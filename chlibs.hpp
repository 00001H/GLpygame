#ifndef CHLIBS_H
#define CHLIBS_H
#include<iostream>
#include<string>
#include<stdexcept>
#include<unordered_map>
#include<glad/glad.h>
#include<texture.hpp>
#include<optional>
#include<filesystem>
#include<cppp.hpp>
#include<memory>
namespace _ft{
    #include<ft2build.h>
    #include FT_FREETYPE_H
}
#define FT_Int32 _ft::FT_Int32
#define FT_RENDER_MODE_LIGHT _ft::FT_RENDER_MODE_LIGHT
namespace pygame{
    class FTError:public std::logic_error{
        using std::logic_error::logic_error;
    };
    struct Ch_Texture{
        Texture* tex;
        int xoffset;
        int yoffset;
        int distance;
    };
    class _Font{
        public:
            _ft::FT_Face face;
            int _id;
            std::unordered_map<wchar_t,Ch_Texture> charmap;
            void set_dimensions(_ft::FT_UInt w,_ft::FT_UInt h){
                _ft::FT_Set_Pixel_Sizes(face,w,h);
            }
            Ch_Texture loadChar(wchar_t ch){
                if(charmap.count(ch)){
                    return charmap[ch];
                }
                int x;
                if((x = _ft::FT_Load_Char(face,ch,FT_LOAD_RENDER|FT_LOAD_TARGET_LIGHT))){
                    std::wcerr << L"Loadchar failed:" << ch << L" " << x << std::endl;
                    throw FTError("Unable to load chararcter.");
                }
                auto glyf = face->glyph;
                auto bmap = glyf->bitmap;
                Texture* t;
                unsigned char colorbyte = 0;
                if(bmap.width==0 && bmap.rows==0){
                    t = new Texture(&colorbyte,1,1,GL_RED,GL_RED,GL_NEAREST,GL_NEAREST,false);
                }else{
                    t = new Texture(bmap.buffer,bmap.width,bmap.rows,GL_RED,GL_RED,GL_LINEAR,GL_LINEAR,false);
                }
                Ch_Texture chtx;
                chtx.tex = t;
                chtx.xoffset = face->glyph->bitmap_left;
                chtx.yoffset = face->glyph->bitmap_top;
                chtx.distance = face->glyph->advance.x;
                charmap.emplace(ch,chtx);
                return chtx;
            }
            void done(){
                FT_Done_Face(face);
                for(const auto& [k,v] : charmap){
                    delete v.tex;
                }
            }
    };
    class Font{
        std::unique_ptr<_Font> pimpl;
        public:
            Font() : pimpl(nullptr){}
            Font(size_t id,_ft::FT_Face fac) : pimpl(new _Font()){
                assert(fac!=nullptr);
                pimpl->_id = id;
                pimpl->face = fac;
            }
            Font(const Font&) = delete;
            Font(Font&& x) : pimpl(std::move(std::move(x).pimpl)){}
            ~Font(){
                destroy();
            }
            void set_dimensions(_ft::FT_UInt w,_ft::FT_UInt h){
                if(pimpl!=nullptr)pimpl->set_dimensions(w,h);
                else throw std::bad_optional_access();
            }
            void destroy(){
                if(pimpl!=nullptr){
                    pimpl->done();
                }
            }
            Ch_Texture loadChar(wchar_t ch){
                if(pimpl!=nullptr)return pimpl->loadChar(ch);
                else throw std::bad_optional_access();
            }
    };
    class Chlib{
        private:
            size_t _font_id=0;
        public:
            _ft::FT_Library ftlib;
            cppp::Dict<std::wstring, Font> fonts;
            Chlib(){
                if(_ft::FT_Init_FreeType(&ftlib)){
                    throw FTError("FreeType initialization failed!");
                }
            }
            Font& loadfont(std::wstring name,std::wstring filepat){
                std::filesystem::path x(filepat);
                return getfont(name,x.string().c_str());
            }
            Font& getfont(std::wstring name,const char *orfile=nullptr){
                if(fonts.has(name)){
                    return fonts.get(name);
                }else if(orfile==nullptr){
                    throw FTError("Requested for unknown font without file!");
                }else{
                    _ft::FT_Face face;
                    if(_ft::FT_New_Face(ftlib,orfile,0,&face)){
                        throw FTError("Cannot load face!");
                    }
                    Font font(_font_id++,face);
                    fonts.moveInto(name,std::move(font));
                    return fonts.get(name);
                }
            }
    };
}
namespace std{
    template<>
    class hash<pygame::Font>{
        std::size_t operator()(const pygame::_Font &fnt) const noexcept{
            return std::hash<int>{}(fnt._id);
        }
    };
}
#undef FT_Int32
#undef FT_RENDER_MODE_LIGHT
#endif//CHLIBS_H
