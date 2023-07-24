#ifndef CHLIBS_H
#define CHLIBS_H
#include"include.hpp"
#include"texture.hpp"
namespace pygame{
    namespace _ft{
        #include<ft2build.h>
        #include FT_FREETYPE_H
    }
    using FT_Int32 = _ft::FT_Int32;
    class FTError : public std::logic_error{
        using std::logic_error::logic_error;
    };
    class FTRuntimeError : public std::logic_error{
        using std::logic_error::logic_error;
    };
    struct Ch_Texture{
        sTexture tex;
        float xoffset;
        float ascent;
        float descent;
        float distance;
        Ch_Texture(sTexture&& t) : tex(std::move(t)){}
        Ch_Texture(const Ch_Texture&) = delete;
        Ch_Texture& operator=(const Ch_Texture&) = delete;
    };
    using char_tex = const Ch_Texture*;
    class _Font{
        public:
            _ft::FT_Face face;
            int _id;
            std::unordered_map<cppp::codepoint,Ch_Texture> charmap;
            void set_dimensions(_ft::FT_UInt w,_ft::FT_UInt h){
                _ft::FT_Set_Pixel_Sizes(face,w,h);
                charmap.clear();
            }
            float getHeight() const{
                return float(face->size->metrics.height)/64.0f;
            }
            char_tex loadChar(cppp::codepoint ch){
                if(charmap.contains(ch)){
                    return &charmap.at(ch);
                }
                int x;
                if((x = _ft::FT_Load_Char(face,ch,FT_LOAD_RENDER))){
                    std::cerr << "Loadchar failed: (U+" << std::hex << +ch << std::dec << ") " << x << std::endl;
                    throw FTError("Unable to load chararcter.");
                }
                auto glyf = face->glyph;
                auto bmap = glyf->bitmap;
                sTexture t;
                static unsigned char colorbyte = 0;//doesn't get destroyed on function exit
                if(bmap.width==0 && bmap.rows==0){
                    t.reset(new Texture(&colorbyte,1,1,GL_RED,GL_RED,GL_NEAREST,GL_NEAREST,false));
                }else{
                    t.reset(new Texture(bmap.buffer,bmap.width,bmap.rows,GL_RED,GL_RED,GL_LINEAR,GL_LINEAR,false));
                }
                charmap.try_emplace(ch,std::move(t));
                Ch_Texture& chtx = charmap.at(ch);
                chtx.xoffset = float(glyf->metrics.horiBearingX)/64.0f;
                chtx.ascent = float(glyf->metrics.horiBearingY)/64.0f;
                chtx.descent = float(glyf->metrics.horiBearingY-glyf->metrics.height)/64.0f;
                chtx.distance = float(glyf->metrics.horiAdvance)/64.0f;
                return &chtx;
            }
            void done(){
                FT_Done_Face(face);
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
            Font& operator=(const Font&) = delete;
            Font(const Font&) = delete;
            Font(Font&& x) : pimpl(std::move(x.pimpl)){
                x.pimpl = nullptr;
            }
            ~Font(){
                destroy();
            }
            float getHeight() const{
                if(pimpl!=nullptr)return pimpl->getHeight();
                else throw std::bad_optional_access();
            }
            void set_dimensions(_ft::FT_UInt w,_ft::FT_UInt h){
                if(pimpl!=nullptr)pimpl->set_dimensions(w,h);
                else throw std::bad_optional_access();
            }
            void destroy(){
                if(pimpl!=nullptr){
                    pimpl->done();
                    pimpl = nullptr;
                }
            }
            char_tex loadChar(cppp::codepoint ch){
                if(pimpl!=nullptr)return pimpl->loadChar(ch);
                else throw std::bad_optional_access();
            }
    };
    class Chlib{
        mutable std::unordered_map<std::string, Font> fonts;
        mutable size_t _font_id=0;
        mutable _ft::FT_Library ftlib;
        Font& _getfont(std::string name,const char *orfile=nullptr) const{
            if(fonts.contains(name)){
                return fonts.at(name);
            }else if(orfile==nullptr){
                throw FTError("Requested for unknown font without file!");
            }else{
                _ft::FT_Face face;
                if(_ft::FT_New_Face(ftlib,orfile,0,&face)){
                    throw FTRuntimeError("Cannot load face!");
                }
                Font font(_font_id++,face);
                fonts.emplace(name,std::move(font));
                return fonts.at(name);
            }
        }
        public:
            Chlib(){
                if(_ft::FT_Init_FreeType(&ftlib)){
                    throw FTError("FreeType initialization failed!");
                }
            }
            Font& loadfont(std::string name,std::string filepat){
                return _getfont(name,filepat.c_str());
            }
            Font& getfont(std::string name) const{
                return _getfont(name);
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
#endif//CHLIBS_H
