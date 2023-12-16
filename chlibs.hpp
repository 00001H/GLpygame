/*
TODO: Tidy up this incredibly messy code
I can't stand it anymore
*/

#ifndef CHLIBS_H
#define CHLIBS_H
#include"include.hpp"
#include<variant>
namespace pygame{
    using namespace std::literals;
    namespace _ft{
        #include<ft2build.h>
        #include FT_FREETYPE_H
        #include<freetype/ftbitmap.h>
    }
    using FT_Int32 = _ft::FT_Int32;
    class FTError : public cppp::u8_logic_error{
        using cppp::u8_logic_error::u8_logic_error;
    };
    class FTRuntimeError : public cppp::u8_runtime_error{
        using cppp::u8_runtime_error::u8_runtime_error;
    };
    class Ch_Texture{
        pygame::_ft::FT_Bitmap _buf;
        std::unordered_map<const Window*,sTexture> _tex;
        float xoffset;
        float ascent;
        //NEGATIVE!!
        float descent;
        float distance;
        friend class _Font;
        public:
            float xoff() const{
                return xoffset;
            }
            float asc() const{
                return ascent;
            }
            float dsc() const{
                return descent;
            }
            float dst() const{
                return distance;
            }
            Ch_Texture() = default;
            sTexture& tex(Window& w){
                load(w);
                return _tex.at(&w);
            }
            void load(Window&);
            Ch_Texture(const Ch_Texture&) = delete;
            Ch_Texture& operator=(const Ch_Texture&) = delete;
    };
    using char_tex = Ch_Texture*;
    class _Font{
        _ft::FT_Library& ftlib;
        _ft::FT_Face face;
        friend class Font;
        using cm_t = std::unordered_map<cppp::codepoint,Ch_Texture>;
        cm_t charmap;
        size_t _id;
        static size_t ID;
        public:
            size_t _uuid() const{
                return _id;
            }
            _Font(_ft::FT_Library& f) : ftlib(f), face(nullptr), charmap(), _id(++ID){}
            void set_dimensions(_ft::FT_UInt w,_ft::FT_UInt h){
                _ft::FT_Set_Pixel_Sizes(face,w,h);
                charmap.clear();
            }
            float get_height() const{
                return get_ascender()-get_descender();
            }
            float get_ascender() const{
                return float(face->size->metrics.ascender)/64.0f;
            }
            float get_descender() const{
                return float(face->size->metrics.descender)/64.0f;
            }
            char_tex load_char(cppp::codepoint);
            void done(){
                FT_Done_Face(face);
            }
    };
    class Font{
        std::unique_ptr<_Font> pimpl;
        public:
            Font() : pimpl(nullptr){}
            Font(_ft::FT_Library& fl,_ft::FT_Face fac) : pimpl(new _Font(fl)){
                assert(fac!=nullptr);
                pimpl->face = fac;
            }
            Font& operator=(const Font&) = delete;
            Font(const Font&) = delete;
            Font(Font&& x) : pimpl(std::move(x.pimpl)){
                x.pimpl.reset();
            }
            ~Font(){
                destroy();
            }
            float get_ascender() const{
                if(pimpl!=nullptr)return pimpl->get_ascender();
                else throw std::bad_optional_access();
            }
            float get_descender() const{
                if(pimpl!=nullptr)return pimpl->get_descender();
                else throw std::bad_optional_access();
            }
            float get_height() const{
                if(pimpl!=nullptr)return pimpl->get_height();
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
            char_tex load_char(cppp::codepoint ch){
                if(pimpl!=nullptr)return pimpl->load_char(ch);
                else throw std::bad_optional_access();
            }
    };
    class Chlib{
        mutable std::unordered_map<std::u8string, Font> fonts;
        mutable _ft::FT_Library ftlib;
        Font& _getfont(std::u8string name,std::optional<std::u8string> orfile = std::nullopt) const;
        public:
            Chlib(){
                if(_ft::FT_Init_FreeType(&ftlib)){
                    throw FTError(u8"FreeType initialization failed!"sv);
                }
            }
            Font& loadfont(const std::u8string& name,const std::u8string& filepat){
                return _getfont(name,filepat);
            }
            Font& getfont(const std::u8string& name) const{
                return _getfont(name);
            }
    };
}
namespace std{
    template<>
    class hash<pygame::Font>{
        std::size_t operator()(const pygame::_Font &fnt) const noexcept{
            return std::hash<int>{}(fnt._uuid());
        }
    };
}
#undef FT_Int32
#endif//CHLIBS_H
