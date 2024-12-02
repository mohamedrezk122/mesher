#include <iostream>

#define LOG(x) logvar(#x, x, __LINE__, __FILE__) 

template <typename T> 
void logvar(std::string var, T value, int line, std::string file=""){
    std::size_t idx = file.find_last_of("/\\");
    file = file.substr(idx+1);
    std::cout 
            << "[" << file << ":" << line << "] "
            << var << " = " << value
            << std::endl; 
}