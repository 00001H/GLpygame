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
    class FTError : public cppp::u8_logic_error{
        using cppp::u8_logic_error::u8_logic_error;
    };
    class FTRuntimeError : public cppp::u8_runtime_error{
        using cppp::u8_runtime_error::u8_runtime_error;
    };
    struct Ch_Texture{
        sTexture tex;
        float xoffset;
        float ascent;
        //NEGATIVE!!
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
            char_tex loadChar(cppp::codepoint ch);
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
            float get_height() const{
                if(pimpl!=nullptr)return pimpl->getHeight();
                else throw std::bad_optional_access();
            }
            float getHeight() const{
                return get_height();
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
        mutable std::unordered_map<std::u8string, Font> fonts;
        mutable size_t _font_id=0;
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
            return std::hash<int>{}(fnt._id);
        }
    };
}
#undef FT_Int32
#endif//CHLIBS_H
