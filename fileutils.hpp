#ifndef FILEUTILS_H
#define FILEUTILS_H
#include<iostream>
#include<stdexcept>
#include<fstream>
#include<sys/stat.h>

namespace pygame{

    std::string loadStringFile(const std::string fname){
        std::ifstream file;
        file.open(fname.c_str());
        std::string st;
        char ch;
        if(!file){
            std::cerr << fname << std::endl;
            throw std::logic_error("Error: file does not exist");
        }
        while(!file.eof()){
            file >> std::noskipws >> ch;
            st += ch;
        }
        file.close();
        return st;
    }
    bool fileexists(std::string filename){
        struct stat trash;
        return stat(filename.c_str(),&trash) != -1;
    }
    
}
#endif//FILEUTILS_H
