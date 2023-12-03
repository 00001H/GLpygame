#include"include.hpp"
#include"color.hpp"
namespace pygame{
    inline namespace constants{
        constexpr float SW = 1920.0f;
        constexpr float SH = 1080.0f;
        constexpr float HSW = SW/2.0f;
        constexpr float HSH = SH/2.0f;
        constexpr float half_pi = glm::half_pi<float>();
        constexpr float pi = std::numbers::pi_v<float>;
        //https://xkcd.com/1292
        constexpr float pau = static_cast<float>(3.141592653589793l*1.5l);
        constexpr float tau = glm::two_pi<float>();
        const Point SCRCNTR = {HSW,HSH};
        const glm::vec2 SCRDIMS = {SW,SH};
        using namespace ::pygame::color::colcon;
    }
}
