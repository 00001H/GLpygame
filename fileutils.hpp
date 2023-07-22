#ifndef FILEUTILS_H
#define FILEUTILS_H
#include<iostream>
#include<stdexcept>
#include<fstream>
#include<sys/stat.h>

namespace pygame{

    std::string loadStringFile(const std::string& fname){
        std::ifstream file;
        file.open(fname.c_str());
        auto bgn = std::istreambuf_iterator<char>(file);
        auto end = std::istreambuf_iterator<char>();
        std::string st{bgn,end};
        file.close();
        return st;
    }
    bool fileexists(std::string filename){
        struct stat trash;
        return stat(filename.c_str(),&trash) != -1;
    }
    
}
#endif//FILEUTILS_H
