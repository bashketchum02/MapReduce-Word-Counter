//serial program
#include<cctype>
#include<iostream>
#include<fstream>
#include<string>
#include<algorithm>
#include<map>
#include<set>
#include<sstream>
//#include<mpi.h>

struct comp{
    template<typename T>
    bool operator()(const T &l, const T &r) const {
        if (l.second != r.second) {
            return l.second > r.second;
        } 
        return l.first > r.first;
    }
};

std::string str_tolower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), 
        [](unsigned char c){ return std::tolower(c); } // correct
    );
    return s;
}

void tokenizer(std::string const &str, const char separator, std::map<std::string, int> &out, int &count) {  
    std::stringstream ss(str);
    std::string s; 
    while (std::getline(ss, s, separator)) {
        s.erase(std::remove_if(s.begin(), s.end(), 
            []( auto const& c ) -> bool {return !std::isalpha(c);}), s.end()
        );
        if(s.length() >= 3 && s.length() <= 20){
            out[s] += 1;
            count += 1;
        }
    }
} 

int main(){
    std::fstream new_file;
    int count = 0;
    std::map<std::string, int> words;
    new_file.open("pg100.txt", std::ios::in); //read mode
    if(new_file.is_open()){
        std::string tp;
        while(getline(new_file, tp)){
            tokenizer(str_tolower(tp), ' ', words, count);
        }
        new_file.close();
    }
    std::cout<<count<<std::endl;
}
