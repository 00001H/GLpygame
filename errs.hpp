#ifndef GLPY_ERRS_HPP
#define GLPY_ERRS_HPP
#include"include.hpp"
namespace pygame{
    #define inherits : public
    class error inherits cppp::u8_logic_error{
        using cppp::u8_logic_error::u8_logic_error;
    };
    class shader_error inherits error{
        using error::error;
    };
    class vshad_compilation_failed inherits shader_error{
        using shader_error::shader_error;
    };
    class fshad_compilation_failed inherits shader_error{
        using shader_error::shader_error;
    };
    class program_linking_failed inherits shader_error{
        using shader_error::shader_error;
    };
    #undef inherits
}
#endif// ERRS
