#ifndef GLPY_FILEUTILS_HPP
#define GLPY_FILEUTILS_HPP
#include<string_view>
#include<filesystem>
namespace pygame{
    std::u8string load_string_file(const std::u8string_view);
    inline bool fileexists(const std::u8string_view filename){
        return std::filesystem::exists(filename);
    }
}
#endif//FILEUTILS_H
