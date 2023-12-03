#include"chlibs.hpp"
#include"gsdl.hpp"
namespace pygame{
    size_t _Font::ID{0uz};
    sTexture load_c_tex(_ft::FT_Bitmap& bmp,Window& w){
        static unsigned char colorbyte = 0;//doesn't get destroyed on function exit
        if(bmp.width==0 && bmp.rows==0){
            return sTexture{new Texture(w,&colorbyte,1,1,GL_RED,GL_RED,GL_NEAREST,GL_NEAREST,false)};
        }else{
            return sTexture{new Texture(w,bmp.buffer,bmp.width,bmp.rows,GL_RED,GL_RED,GL_LINEAR,GL_LINEAR,false)};
        }
        
    }
    void Ch_Texture::load(Window& w){
        if(_tex.contains(&w))return;
        _tex.try_emplace(&w,load_c_tex(_buf,w));
    }
    char_tex _Font::load_char(cppp::codepoint ch){
        if(auto it=charmap.find(ch);it!=charmap.end()){
            return &it->second;
        }
        int x;
        if((x = _ft::FT_Load_Char(face,ch,FT_LOAD_RENDER))){
            cppp::fcerr << u8"Loadchar failed: (U+"sv << std::hex << +ch << std::dec << u8") "sv << x << std::endl;
            throw FTError(u8"Unable to load character."sv);
        }
        auto glyf = face->glyph;
        charmap.try_emplace(ch);
        Ch_Texture& chtx = charmap.at(ch);
        _ft::FT_Bitmap_Init(&chtx._buf);
        _ft::FT_Bitmap_Copy(ftlib,&glyf->bitmap,&chtx._buf);
        chtx.xoffset = float(glyf->metrics.horiBearingX)/64.0f;
        chtx.ascent = float(glyf->metrics.horiBearingY)/64.0f;
        chtx.descent = float(glyf->metrics.horiBearingY-glyf->metrics.height)/64.0f;
        chtx.distance = float(glyf->metrics.horiAdvance)/64.0f;
        return &chtx;
    }
    Font& Chlib::_getfont(std::u8string name,std::optional<std::u8string> orfile) const{
        if(fonts.contains(name)){
            return fonts.at(name);
        }else if(!orfile){
            throw FTError(u8"Requested for unknown font without file!"sv);
        }else{
            _ft::FT_Face face;
            if(_ft::FT_New_Face(ftlib,cppp::copy_as_plain(orfile.value()).c_str(),0,&face)){
                throw FTRuntimeError(u8"Cannot load face!"sv);
            }
            fonts.try_emplace(name,ftlib,face);
            return fonts.at(name);
        }
    }
}
