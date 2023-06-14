//parallel program
#include<cctype>
#include<iostream>
#include<fstream>
#include<string>
#include<algorithm>
#include<map>
#include<set>
#include<vector>
#include<sstream>
#include<mpi.h>

#define MAX_LINES 100000000

struct comp{
    template<typename T>
    bool operator()(const T &l, const T &r) const {
        if (l.second != r.second) {
            return l.second > r.second;
        } 
        return l.first > r.first;
    }
};

std::string serializer(std::map<std::string, int> counter){
    std::string emp_str = "";
    for(auto x : counter){
        emp_str += (x.first + ": " + std::to_string(x.second) + "\n");
    }
    return emp_str;
}

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
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	int num_txt_files, word_length_min, word_length_max;
    char order;
    if(rank == 0){
        std::cout<<"Enter the number of text files: ";
        std::cin>>num_txt_files;
    }
    MPI_Bcast(&num_txt_files, 1, MPI_INT, 0, MPI_COMM_WORLD);
    std::vector<std::string> file_list(num_txt_files);
    std::map<std::string, int> words;
    //std::cout<<"Data received : "<<num_txt_files<<"on rank "<<rank<<std::endl;
    for(int i = 0; i < num_txt_files; i++) {
            char tmp_filename[100];
            if(rank == 0){
                std::cout<<"Enter the path of text file "<<i+1<<": ";
                std::cin>>tmp_filename;
            }
            MPI_Bcast(&tmp_filename, 101, MPI_CHAR, 0, MPI_COMM_WORLD);
            file_list[i] = tmp_filename;
            //std::cout<<"Data received : "<<tmp_filename<<" on rank "<<rank<<std::endl;
    }
    
    if(!rank){
        std::cout<<"Enter the minimum length of words to consider: ";
        std::cin>>word_length_min;
    }
    MPI_Bcast(&word_length_min, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(!rank){
        std::cout<<"Enter the maximum length of words to consider: ";
        std::cin>>word_length_max;
    }
    MPI_Bcast(&word_length_max, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(!rank){
        std::cout<<"Enter 'a' for alphabetical order or 'n' for number of words order: ";
        std::cin>>order;
    }
    MPI_Bcast(&order, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    long line_count = 0;
    long total_lines = 0;
    
    std::vector<std::string> rank_0_lines;
    for(int i = 0; i < num_txt_files; i++){
        std::fstream new_file;
        new_file.open(file_list[i], std::ios::in); //read mode
        if(new_file.is_open()){
            std::string tp;
            while(getline(new_file, tp)){
                if(rank == 0){
                    //MPI_Send (&length, 1, MPI_INT, line_count%size, 1, MPI_COMM_WORLD);
                    line_count += 1;
                    rank_0_lines.push_back(tp);
                    //MPI_Send(tp.c_str(), length+1, MPI_CHAR, line_count%size, 0, MPI_COMM_WORLD);
                }
            }                  
        }
        new_file.close();
    }
    MPI_Bcast(&line_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    //std::cout<<line_count<<std::endl;
    char temp[1000];
    int count = 0;
    for(int i = 0; i < line_count; i++){
        if(!rank){
            //std::cout<<i%(size-1) + 1<<std::endl;
            MPI_Send(rank_0_lines[i].c_str(), 1000, MPI_CHAR, i%(size-1)+1, 0, MPI_COMM_WORLD);
        }
        else{
            if(rank && (i%(size-1) + 1) == rank){
                MPI_Recv(&temp, 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::string inp(temp);
                tokenizer(str_tolower(inp), ' ', words, word_length_min, word_length_max, count);
            }
        }
    }
    char huge_buffer[1000000];
    //send all the buffers to rank 0 
    if(!rank){
        for(int i = 1; i < size; i++){
            MPI_Recv(&huge_buffer, 10000000, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout<<huge_buffer<<std::endl;
        }
    }
    if(rank){
        std::string buf = serializer(words);
        MPI_Send(buf.c_str(), 10000000, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
        
    //tokenizer(str_tolower(message), ' ', words, word_length_min, word_length_max);
    //freopen("output_log.txt", "w", stdout);    
    /*if(order == 'a'){
        std::cout<<"Word Count Report (Alphabetical Order):"<<std::endl;
        for(auto x : words){
            std::cout<<x.first<<": "<<x.second<<std::endl;
        }
    }
    else{
        std::cout<<"Word Count Report (Number of Words Order):"<<std::endl;
        std::set<std::pair<std::string, int>, comp> answers(words.begin(), words.end());
    }*/
    MPI_Finalize();
}
