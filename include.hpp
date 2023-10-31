#ifndef GLPY_INCL_UNIH
#define GLPY_INCL_UNIH
#include<unordered_map>
#include<filesystem>
#include<sys/stat.h>
#include<functional>
#include<stdexcept>
#include<algorithm>
#include<iostream>
#include<optional>
#include<cppp/strings/strings.hpp>
#include<cppp/binary/binary.hpp>
#include<fstream>
#include<string>
#include<memory>
#include<thread>
#include<vector>
#include<any>

#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include"stb_image.h"

namespace pygame{
    using namespace cppp::strconcat;
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    typedef vec2 Point;
    typedef vec4 Color;
    struct Line{
        Point a;
        Point b;
        Line(Point a,Point b) : a(a), b(b) {}
        operator glm::vec2() const{
            return b-a;
        }
        float length() const{
            return glm::distance(a,b);
        }
    };
    class Rect{
        Point loc;
        glm::vec2 _dims;
        public:
            constexpr Rect() : loc(0.0f), _dims(0.0f){}
            constexpr Rect(float x,float y,float w,float h) : loc(x,y), _dims(w,h){}
            constexpr Rect(Point pos, glm::vec2 dims) : loc(pos), _dims(dims){}
            const Point& ltop() const{
                return loc;
            }
            Rect shrink(glm::vec2 by) const{
                return {loc+by,_dims-by-by};
            }
            const Point& pos() const{
                return loc;
            }
            Point& pos(){
                return loc;
            }
            const glm::vec2& dims() const{
                return _dims;
            }
            glm::vec2& dims(){
                return _dims;
            }
            Rect reposition(Point there) const{
                Rect nw = *this;
                nw.loc = there;
                return nw;
            }
            Point center() const{
                return loc+_dims/2.0f;
            }
            const float& left() const{
                return loc.x;
            }
            float& left(){
                return loc.x;
            }
            const float& x() const{
                return loc.x;
            }
            float& x(){
                return loc.x;
            }
            const float& top() const{
                return loc.y;
            }
            float& top(){
                return loc.y;
            }
            const float& y() const{
                return loc.y;
            }
            float& y(){
                return loc.y;
            }
            const float& width() const{
                return _dims.x;
            }
            float& width(){
                return _dims.x;
            }
            const float& height() const{
                return _dims.y;
            }
            float& height(){
                return _dims.y;
            }
            float right() const{
                return left()+width();
            }
            float bottom() const{
                return top()+height();
            }
            bool colliderect(Rect other) const{
                return ((left()<other.right())//left is lefter than other right
                    &&(right()>other.left())//right is righter than other left
                    &&(top()<other.bottom())//top is topper than other bottom//note:bigger y = lower
                    &&(bottom()>other.top()));//bottom is bottomer than other top
            }
            bool collidepoint(Point point) const{
                return ((point.x<right())//lefter than right
                    &&(point.x>left())//righter than left
                    &&(point.y<bottom())//topper than bottom
                    &&(point.y>top()));//bottomer than top
            }
    };
}
#endif
