#ifndef GLPY_ERRS_HPP
#define GLPY_ERRS_HPP
#include"include.hpp"
namespace pygame{
    #define inherits : public
    class error inherits std::exception{
        protected:
            std::string wht;
        public:
            error(std::string st) : wht(st){}
            const char* what() const noexcept{
                return wht.c_str();
            }
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
