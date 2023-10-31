#ifndef GLPYGAME_TEXTURE_HPP
#define GLPYGAME_TEXTURE_HPP
#include"include.hpp"
namespace pygame{
    using namespace std::literals;
    class Texture{
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
            Texture(Texture&& other){
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
            Texture(
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
            ) : _width(width),_height(height),internalformat(internalformat),
            texture(0), placeholder(false){
                glGenTextures(1,&texture);
                glBindTexture(GL_TEXTURE_2D,texture);
                glTexImage2D(GL_TEXTURE_2D,0,internalformat,width,height,0,imageformat,GL_UNSIGNED_BYTE,data);
                if(mipmap)glGenerateMipmap(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,wrap_s);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,wrap_t);
                const float borderc[] = {bordercolor.r,bordercolor.g,bordercolor.b,bordercolor.a};
                glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,borderc);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minfilter);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,magfilter);
                texturehandle = glGetTextureHandleARB(texture);
                resident = false;
                enable();
            }
            bool isNull() const{
                return placeholder;
            }
            void enable(){
                if(!(placeholder||resident)){
                    glMakeTextureHandleResidentARB(texturehandle);
                    resident=true;
                }
            }
            void disable(){
                if(resident&&(!placeholder)){
                    glMakeTextureHandleNonResidentARB(texturehandle);
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
                    glDeleteTextures(1,&texture);
                    placeholder = true;
                }
            }
            ~Texture(){
                destroy();
            }
    };
    using mpTexture = Texture*;
    using pTexture = const Texture*;
    using sTexture = std::unique_ptr<Texture>;
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
}
#endif//GLPYGAME_TEXTURE_HPP
