#ifndef PYGAME_COLOR_HPP
#define PYGAME_COLOR_HPP
#include"gsdl.hpp"
namespace pygame{
    namespace color{
        inline namespace colcon{
            #define defcolor const constexpr pygame::Color
            defcolor WHITE(1.0f,1.0f,1.0f,1.0f);
            defcolor BLACK(0.0f,0.0f,0.0f,1.0f);
            defcolor RED(1.0f,0.0f,0.0f,1.0f);
            defcolor GREEN(0.0f,1.0f,0.0f,1.0f);
            defcolor BLUE(0.0f,0.0f,1.0f,1.0f);
            defcolor YELLOW(1.0f,1.0f,0.0f,1.0f);
            defcolor PURPLE(1.0f,0.0f,1.0f,1.0f);
            defcolor CYAN(0.0f,1.0f,1.0f,1.0f);
            defcolor TRANSPARENT(0.0f,0.0f,0.0f,0.0f);
            #undef defcolor
        }
        namespace{
            inline constexpr vec3 rgb(vec4 v){
                return vec3(v.r,v.g,v.b);
            }
        }
        inline constexpr pygame::Color blend(pygame::Color src,pygame::Color dst){
            return pygame::Color(rgb(src)*src.a+rgb(dst)*dst.a*(1-src.a),1-((1-src.a)*(1-dst.a)));
        }
    }
}
#endif//PYGAME_COLOR_HPP
