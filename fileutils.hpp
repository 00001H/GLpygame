#ifndef GLPY_FILEUTILS_HPP
#define GLPY_FILEUTILS_HPP
#include<filesystem>
#include<cppp.hpp>
namespace pygame{
    std::u8string loadStringFile(const std::u8string_view);
    inline bool fileexists(const std::u8string_view filename){
        return std::filesystem::exists(filename);
    }
}
#endif//FILEUTILS_H
