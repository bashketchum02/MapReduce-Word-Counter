//parallel program
//#include <bits/chrono.h>
#include<cctype>
#include<iostream>
#include<fstream>
#include<string>
#include<algorithm>
#include<map>
#include<set>
#include<vector>
#include<sstream>
#include<chrono>
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

void tokenizer(std::string const &str, const char separator, std::map<std::string, int> &out, int min_length, int max_length, int &count) {  
    std::stringstream ss(str);
    std::string s; 
    while (std::getline(ss, s, separator)) {
        s.erase(std::remove_if(s.begin(), s.end(), 
            []( auto const& c ) -> bool {return !std::isalpha(c);}), s.end()
        );
        if((s.length() >= min_length) && (s.length() <= max_length)){
            out[s] += 1;
            count += 1;
        }
    }
} 

int main(int argc, char **argv){
    //MPI INIT
    /*int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    */
	int num_txt_files, word_length_min, word_length_max;
    char order;
    int count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout<<"Enter the number of text files: ";
    std::cin>>num_txt_files;
    std::vector<std::string> file_list(num_txt_files);
    std::map<std::string, int> words;
    for(int i = 0; i < num_txt_files; i++) {
        std::cout<<"Enter the path of text file "<<i+1<<": ";
        std::cin>>file_list[i];
    }
    std::cout<<"Enter the minimum length of words to consider: ";
    std::cin>>word_length_min;
    std::cout<<"Enter the maximum length of words to consider: ";
    std::cin>>word_length_max;
    std::cout<<"Enter 'a' for alphabetical order or 'n' for number of words order: ";
    std::cin>>order;
    long line_count = 0;
    for(int i = 0; i < num_txt_files; i++){
        std::fstream new_file;
        new_file.open(file_list[i], std::ios::in); //read mode
        if(new_file.is_open()){
            std::string tp;
            while(getline(new_file, tp)){
                tokenizer(str_tolower(tp), ' ', words, word_length_min, word_length_max, count);
            }
            new_file.close();
        }
    }
    std::cout<<count<<std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::cout<<"Time : "<<std::chrono::duration_cast<std::chrono::seconds>(end - start).count()<<" seconds"<<std::endl;
    freopen("output_log_serial.txt", "w", stdout);    
    if(order == 'a'){
        std::cout<<"Word Count Report (Alphabetical Order):"<<std::endl;
        for(auto x : words){
            std::cout<<x.first<<": "<<x.second<<std::endl;
        }
    }
    else{
        std::cout<<"Word Count Report (Number of Words Order):"<<std::endl;
        std::set<std::pair<std::string, int>, comp> answers(words.begin(), words.end());
        for(auto x : answers){
            std::cout<<x.first<<" "<<x.second<<std::endl;
        }
    }
}
