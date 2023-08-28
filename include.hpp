#ifndef GLPY_INCL_UNIH
#define GLPY_INCL_UNIH
#include<unordered_map>
#include<filesystem>
#include<sys/stat.h>
#include<stdexcept>
#include<algorithm>
#include<iostream>
#include<optional>
#include<cppp.hpp>
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
    struct Rect{
        float x;
        float y;
        float w;
        float h;
        Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f){}
        Rect(float x,float y,float w,float h) : x(x),y(y),w(w),h(h) {}
        Rect(Point pos, glm::vec2 dims) : x(pos.x), y(pos.y), w(dims.x), h(dims.y){}
        Point ltop() const{
            return {x,y};
        }
        Rect reposition(Point origin) const{
            Rect nw = *this;
            nw.x = origin.x;
            nw.y = origin.y;
            return nw;
        }
        Point center() const{
            return {x+w/2.0f,y+h/2.0f};
        }
        bool colliderect(Rect other) const{
            return ((x<(other.y+other.w))//left is lefter than other right
                  &&((x+w)>(other.x))//right is righter than other left
                  &&(y<(other.y+other.h))//top is topper than other bottom//note:bigger y = lower
                  &&((y+h)>(other.y)));//bottom is bottomer than other top
        }
        bool collidepoint(Point point) const{
            return (((point.x)<(x+w))//lefter than right
                  &&((point.x)>x)//righter than left
                  &&((point.y)<(y+h))//topper than bottom
                  &&((point.y)>y));//bottomer than top
        }
    };
}
#endif
