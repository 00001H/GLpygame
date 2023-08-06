#ifndef GLPY_FILEUTILS_HPP
#define GLPY_FILEUTILS_HPP
#include<cppp.hpp>
namespace pygame{
    std::u8string loadStringFile(const std::u8string_view& fname){
        //Do not use basic_ifstream<char8_t>: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88508
        std::basic_ifstream<char> file;
        file.open(cppp::copy_as_plain(fname).c_str());
        auto bgn = std::istreambuf_iterator<char>(file);
        auto end = std::istreambuf_iterator<char>();
        std::u8string st{bgn,end};
        file.close();
        return st;
    }
    bool fileexists(const std::u8string_view& filename){
        struct stat trash;
        return stat(cppp::copy_as_plain(filename).c_str(),&trash) != -1;
    }
}
#endif//FILEUTILS_H
