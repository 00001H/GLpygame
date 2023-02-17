#ifndef PYGAME_3DG
#define PYGAME_3DG
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<memory>
#include<errs.hpp>
namespace pygame{
    class Cube;
    class Rect3D;
    class Cube{
        glm::vec3 _pos;
        public:
            float w;//x-wise
            float h;//y-wise
            float l;//z-wise
            float yaw=0.0;
            float pitch=0.0;
            float roll=0.0;
            Cube() = default;
            Cube(glm::vec3 pos,float w,float h,float l)
            : _pos(pos),w(w),h(h),l(l){
            }
            glm::vec3 centerD() const{
                return glm::vec3(w/2.0,h/2.0,l/2.0);
            }
            glm::vec3 center() const{
                return _pos+centerD();
            }
            glm::vec3& pos(){
                return _pos;
            }
            void set_center(glm::vec3 center){
                _pos = center-centerD();
            }
            virtual glm::mat4 modelmatrix() const{
                glm::mat4 trans(1.0);
                trans = glm::translate(trans,center());
                trans = glm::rotate(trans,glm::radians(yaw),glm::vec3(0.0,1.0,0.0));
                trans = glm::rotate(trans,glm::radians(pitch),glm::vec3(1.0,0.0,0.0));
                trans = glm::rotate(trans,glm::radians(roll),glm::vec3(0.0,0.0,-1.0));
                trans = glm::translate(trans,-centerD());
                return trans;
            }
    };
    class Rect3D{
        public:
            glm::vec3 bottomleft;
            glm::vec3 bottomright;
            glm::vec3 topleft;
            glm::vec3 topright;
            Rect3D() = default;
            Rect3D(glm::vec3 bl,glm::vec3 br,glm::vec3 tl,glm::vec3 tr)
            : bottomleft(bl),bottomright(br),topleft(tl),topright(tr){}
            virtual glm::mat4 modelmatrix() const{
                return glm::mat4(1.0);
            }
            virtual glm::vec3 center() const{
                return (bottomleft+bottomright+topleft+topright)/4.0f;
            }
    };
    class Camera{
        public:
            glm::vec3 pos;
            float yaw,pitch,roll;
            Camera() = default;
            Camera(glm::vec3 pos,float yaw=0.0,float pitch=0.0,float roll=0.0) : pos(pos),yaw(yaw),pitch(pitch),roll(roll)
            {}
            glm::vec3 worldup() const{
                float rroll = glm::radians(roll);
                return glm::vec3(sin(rroll),cos(rroll),0);
            }
            glm::mat4 viewmatrix() const{
                glm::mat4 trans = glm::mat4(1.0);
                trans = glm::rotate(trans,-glm::radians(pitch),glm::vec3(1.0,0.0,0.0));
                trans = glm::rotate(trans,glm::radians(yaw+90),glm::vec3(0.0,1.0,0.0));
                trans = glm::rotate(trans,-glm::radians(roll),direction());
                trans = glm::translate(trans,-pos);
                return trans;
            }
            glm::vec3 direction() const{
                float ryaw = glm::radians(yaw);
                float rpitch = glm::radians(pitch);
                return glm::normalize(glm::vec3(cos(ryaw)*cos(rpitch),sin(rpitch),sin(ryaw)*cos(rpitch)));
            }
            glm::vec3 xzfront() const{
                float ryaw = glm::radians(yaw);
                return glm::vec3(cos(ryaw),0,sin(ryaw));
            }
            glm::vec3 xzright() const{
                float ryaw = glm::radians(yaw);
                return glm::vec3(-sin(ryaw),0,cos(ryaw));
            }
            glm::vec3 right() const{
                return glm::cross(direction(),up());
            }
            glm::vec3 up() const{
                return glm::mat3(glm::rotate(glm::mat4(1.0),glm::radians(pitch),xzright()))*worldup();
            }
    };
    class Context3D{
        public:
            Camera camera;
            float fov;
            float near_clip;
            float far_clip;
            float aspect_ratio;
            Context3D(float fov=70.0f,float near=0.1f,float far=100.0f,float screenw=1920.0f,float screenh=1080.0f)
            : fov(fov), near_clip(near), far_clip(far), aspect_ratio(screenw/screenh){
            }
            void init_camera(glm::vec3 pos={0.0f,1.0f,0.0f},float yaw=0.0f,float pitch=0.0f,float roll=0.0f){
                camera = Camera(pos,yaw,pitch,roll);
            }
    };
    typedef std::shared_ptr<Context3D> pContext3D;
    pContext3D mkC3d(float fov=70.0f,float near=0.1f,float far=100.0f,float screenw=1920.0f,float screenh=1080.0f){
        return std::make_shared<Context3D>(fov,near,far,screenw,screenh);
    }
}
#endif
