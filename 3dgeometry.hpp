#ifndef PYGAME_3DG
#define PYGAME_3DG
#include"include.hpp"
#include"errs.hpp"
namespace pygame{
    class Rect3D{
        public:
            glm::vec3 bottomleft;
            glm::vec3 bottomright;
            glm::vec3 topleft;
            glm::vec3 topright;
            Rect3D() = default;
            Rect3D(glm::vec3 bl,glm::vec3 br,glm::vec3 tl,glm::vec3 tr)
            : bottomleft(bl),bottomright(br),topleft(tl),topright(tr){}
            glm::mat4 modelmatrix() const{
                return glm::mat4(1.0);
            }
            glm::vec3 center() const{
                return (bottomleft+bottomright+topleft+topright)/4.0f;
            }
    };
    struct YPR{
        float yaw;
        float pitch;
        float roll;
        YPR() : yaw(0.0f), pitch(0.0f), roll(0.0f){}
        YPR(float y,float p,float r) : yaw(y), pitch(p), roll(r){}
        glm::mat4 modelmatrix(glm::mat4 trans) const{
            trans = glm::rotate(trans,pitch,glm::vec3(1.0f,0.0f,0.0f));
            trans = glm::rotate(trans,yaw,glm::vec3(0.0f,1.0f,0.0f));
            trans = glm::rotate(trans,roll,glm::vec3(0.0f,0.0f,-1.0f));
            return trans;
        }
        glm::mat4 viewmatrix() const{
            glm::mat4 trans{1.0f};
            trans = glm::rotate(trans,pitch,glm::vec3(1.0f,0.0f,0.0f));
            trans = glm::rotate(trans,yaw+glm::half_pi<float>(),glm::vec3(0.0f,1.0f,0.0f));
            trans = glm::rotate(trans,roll,direction());
            return trans;
        }
        glm::vec3 direction() const{
            return glm::normalize(glm::vec3(cos(yaw)*cos(pitch),-sin(pitch),sin(yaw)*cos(pitch)));
        }
        glm::vec3 ywfront() const{
            return glm::vec3(cos(yaw),0.0f,sin(yaw));
        }
        glm::vec3 ywright() const{
            return glm::vec3(-sin(yaw),0.0f,cos(yaw));
        }
        glm::vec3 right() const{
            return glm::mat3(glm::rotate(glm::mat4(1.0f),pitch,ywright()))*worldup();
        }
        glm::vec3 up() const{
            return glm::mat3(glm::rotate(glm::mat4(1.0f),pitch,ywright()))*worldup();
        }
        glm::vec3 worldup() const{
            return glm::vec3(sin(roll),cos(roll),0.0f);
        }
    };
    class Cube{
        glm::vec3 _pos;
        glm::vec3 _dims;
        YPR angles;
        glm::vec3 xo() const{
            return {_dims.x,0.0f,0.0f};
        }
        glm::vec3 yo() const{
            return {0.0f,_dims.y,0.0f};
        }
        glm::vec3 zo() const{
            return {0.0f,0.0f,_dims.z};
        }
        constexpr static glm::vec3 ORIGIN{0.0f,0.0f,0.0f};
        public:
            YPR& a(){
                return angles;
            }
            YPR a() const{
                return angles;
            }
            Cube() = default;
            Cube(const glm::vec3& spos,const glm::vec3& dims): _pos(spos), _dims(dims){}
            Rect3D front_face() const{
                return {xo()+zo(),zo(),_dims,yo()+zo()};
            }
            Rect3D back_face() const{
                return {ORIGIN,xo(),yo(),xo()+yo()};
            }
            Rect3D right_face() const{
                return {xo(),xo()+zo(),xo()+yo(),_dims};
            }
            Rect3D left_face() const{
                return {zo(),ORIGIN,yo()+zo(),yo()};
            }
            Rect3D top_face() const{
                return {yo(),xo()+yo(),yo()+zo(),_dims};
            }
            Rect3D bottom_face() const{
                return {xo(),ORIGIN,xo()+zo(),zo()};
            }
            glm::vec3& pos(){
                return _pos;
            }
            glm::vec3 pos() const{
                return _pos;
            }
            glm::vec3& dims(){
                return _dims;
            }
            glm::vec3 dims() const{
                return _dims;
            }
            glm::vec3 centerD() const{
                return _dims/2.0f;
            }
            glm::vec3 center() const{
                return _pos+centerD();
            }
            void set_center(glm::vec3 center){
                _pos = center-centerD();
            }
            glm::mat4 modelmatrix() const{
                return glm::translate(
                        angles.modelmatrix(
                            glm::translate(
                                glm::mat4(1.0f),
                                center()
                            )
                        ),
                        -centerD()
                    );
            }
    };
    struct Camera{
        glm::vec3 pos;
        YPR angles;
        Camera() : pos(0.0f), angles(){}
        Camera(glm::vec3 pos,const YPR& angles={}) : pos(pos),angles(angles){}
        glm::mat4 viewmatrix() const{
            return glm::translate(angles.viewmatrix(),-pos);
        }
    };
    class Context3D{
        public:
            Camera camera;
            float fov;
            float near_clip;
            float far_clip;
            float aspect_ratio;
            Context3D(const Camera& cam,float fov=glm::radians(70.0f),float near=0.1f,float far=100.0f,float aspect_w_over_h=16.0f/9.0f)
            : camera(cam), fov(fov), near_clip(near), far_clip(far), aspect_ratio(aspect_w_over_h){
            }
    };
}
#endif
