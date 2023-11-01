#include"chlibs.hpp"
#include"gsdl.hpp"
namespace pygame{
    char_tex _Font::loadChar(Window& w,cppp::codepoint ch){
        if(!charmap.contains(&w)){
            charmap.try_emplace(&w);
            w.atdone([this,p=&w](){
                charmap.erase(p);
            });
        }
        if(charmap.at(&w).contains(ch)){
            return &charmap.at(&w).at(ch);
        }
        int x;
        if((x = _ft::FT_Load_Char(face,ch,FT_LOAD_RENDER))){
            cppp::fcerr << u8"Loadchar failed: (U+"sv << std::hex << +ch << std::dec << u8") "sv << x << std::endl;
            throw FTError(u8"Unable to load chararcter."sv);
        }
        auto glyf = face->glyph;
        auto bmap = glyf->bitmap;
        sTexture t;
        static unsigned char colorbyte = 0;//doesn't get destroyed on function exit
        if(bmap.width==0 && bmap.rows==0){
            t.reset(new Texture(w,&colorbyte,1,1,GL_RED,GL_RED,GL_NEAREST,GL_NEAREST,false));
        }else{
            t.reset(new Texture(w,bmap.buffer,bmap.width,bmap.rows,GL_RED,GL_RED,GL_LINEAR,GL_LINEAR,false));
        }
        charmap.at(&w).try_emplace(ch,std::move(t));
        Ch_Texture& chtx = charmap.at(&w).at(ch);
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
            Font font(_font_id++,face);
            fonts.emplace(name,std::move(font));
            return fonts.at(name);
        }
    }
}
