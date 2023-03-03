#ifndef GLPYGAME_TEXTURE_HPP
#define GLPYGAME_TEXTURE_HPP
#include<glad/glad.h>
#include<glm/glm.hpp>
#include<memory>
namespace pygame{
    class Texture{
        private:
            bool resident;
            bool placeholder=false;
            GLsizei width;
            GLsizei height;
            GLenum internalformat;
            GLuint texture=0;
            GLuint64 texturehandle;
        public:
            Texture(Texture&&);//TODO
            Texture& operator =(Texture&&);//TODO
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
                glm::vec4 bordercolor={0.0,0.0,0.0,1.0}
            ) : width(width),height(height),internalformat(internalformat){
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
            void enable(){
                if(!resident){
                    glMakeTextureHandleResidentARB(texturehandle);
                    resident=true;
                }
            }
            void disable(){
                if(resident){
                    glMakeTextureHandleNonResidentARB(texturehandle);
                    resident=false;
                }
            }
            float getWidth() const{
                return width;
            }
            float getHeight() const{
                return height;
            }
            glm::vec2 getSize() const{
                return {width,height};
            }
            float scalingToFitInside(float side_length) const{
                return side_length/glm::max(getHeight(),getWidth());
            }
            auto getHandle() const{
                if(!resident){
                    throw std::logic_error("Trying to access a non-resident handle!");
                }
                return texturehandle;
            }
            auto getId() const{
                return texture;
            }
            ~Texture(){
                disable();
                glDeleteTextures(1,&texture);
            }
    };
}
#endif//GLPYGAME_TEXTURE_HPP
