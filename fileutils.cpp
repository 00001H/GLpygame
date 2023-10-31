#include"fileutils.hpp"
#include<cppp/strings/strings.hpp>
#include<fstream>
namespace pygame{
    std::u8string load_string_file(const std::u8string_view fname){
        //Do not use basic_ifstream<char8_t>: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88508
        std::basic_ifstream<char> file;
        file.open(cppp::copy_as_plain(fname));
        auto bgn = std::istreambuf_iterator<char>(file);
        auto end = std::istreambuf_iterator<char>();
        std::u8string st{bgn,end};
        file.close();
        return st;
    }
}
