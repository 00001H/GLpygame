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
    class Window;
    struct Line{
        Point a;
        Point b;
        Line() : a(0.0f), b(0.0f){}
        Line(Point a,Point b) : a(a), b(b) {}
        operator glm::vec2() const{
            return b-a;
        }
        float length() const{
            return glm::distance(a,b);
        }
        float squared_length() const{
            return glm::dot(b-a,b-a);
        }
        pygame::Line translate(glm::vec2 d) const{
            return {a+d,b+d};
        }
        float squared_dist(Point p) const{
            return _square((b.x-a.x)*(a.y-p.y)-(b.y-a.y)*(a.x-p.x))/squared_length();
        }
        bool above_point(Point p) const{
            const bool dr{a.x<b.x};
            const Point& ax{dr?a:b};
            const Point& bx{dr?b:a};
            if(p.x<ax.x||p.x>bx.x)return false;
            return p.y > ((p.x-ax.x)/(bx.x-ax.x)*(bx.y-ax.y)+ax.y);
        }
        private:
            inline static float _square(float f){
                return f*f;
            }
    };
    class Rect{
        Point loc;
        glm::vec2 _dims;
        public:
            constexpr Rect() : loc(0.0f), _dims(0.0f){}
            constexpr Rect(float x,float y,float w,float h) : loc(x,y), _dims(w,h){}
            constexpr Rect(Point pos, glm::vec2 dims) : loc(pos), _dims(dims){}
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
            Point ltop() const{
                return loc;
            }
            Point rtop() const{
                return {right(),top()};
            }
            Point ctop() const{
                return {left()+width()/2.0f,top()};
            }
            Point lbot() const{
                return {left(),bottom()};
            }
            Point rbot() const{
                return {right(),bottom()};
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
    class Texture;
    using mpTexture = Texture*;
    using pTexture = const Texture*;
    using sTexture = std::unique_ptr<Texture>;
}
#endif
