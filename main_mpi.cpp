// parallel program
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <mpi.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// comparator struct - it helps sort the set of words
// Chew Wei Ren - 1181203505
struct comp {
  template <typename T> bool operator()(const T &l, const T &r) const {
    if (l.second != r.second) {
      return l.second > r.second;
    }
    return l.first > r.first;
  }
};

// serializes a map into a string
// Chew Wei Ren - 1181203505
std::string serializer(std::map<std::string, int> counter) {
  std::string emp_str = "";
  for (auto x : counter) {
    emp_str += (x.first + ':' + std::to_string(x.second) + '\n');
  }
  // std::cout<<emp_str<<std::endl;
  return emp_str;
}

// split takes a word and a count and returns them from a string of format
// word:count
// Chew Wei Ren - 1181203505
std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (getline(ss, item, delim)) {
    result.push_back(item);
  }

  return result;
}

// takes a string of format
// word:count
// and transforms into a map
//  Reynard Kok - 1181203212
void deserializer(std::string buffer, std::map<std::string, int> &out,
                  int &word_count, int min_len, int max_len) {
  std::stringstream ss(buffer);
  std::string s;
  while (std::getline(ss, s)) {
    std::vector<std::string> tokens = split(s, ':');
    if (tokens.size() == 2 && (tokens[0].length() >= min_len) &&
        (tokens[0].length() <= max_len)) {
      out[tokens[0]] += std::stoi(tokens[1]);
      word_count += 1;
    }
  }
}

// makes a string lowercase
// Reynard Kok - 1181203212
std::string str_tolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}

// this one takes a line of text and seperates words also cleans any non
// alphabetic characters
// Reynard Kok - 1181203212
void tokenizer(std::string const &str, const char separator,
               std::map<std::string, int> &out, int min_length, int max_length,
               int &count) {
  std::stringstream ss(str);
  std::string s;
  while (std::getline(ss, s, separator)) {
    s.erase(
        std::remove_if(s.begin(), s.end(),
                       [](auto const &c) -> bool { return !std::isalpha(c); }),
        s.end());
    if ((s.length() >= min_length) && (s.length() <= max_length)) {
      out[s] += 1;
      count += 1;
    }
  }
}

int main(int argc, char **argv) {
  // MPI INIT
  // Farhan Narodi - 1201302096
  int rank, size;
  double start, end;
  int word_counter = 0;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int num_txt_files, word_length_min, word_length_max;
  char order;
  start = MPI_Wtime();

  // Farhan Narodi - 1201302096
  // User input
  if (rank == 0) {
    std::cout << "Enter the number of text files: ";
    std::cin >> num_txt_files;
  }

  std::vector<std::string> file_list(num_txt_files);
  std::map<std::string, int> words;
  for (int i = 0; i < num_txt_files; i++) {
    if (rank == 0) {
      std::cout << "Enter the path of text file " << i + 1 << ": ";
      std::cin >> file_list[i];
    }
  }

  if (!rank) {
    std::cout << "Enter the minimum length of words to consider: ";
    std::cin >> word_length_min;
  }
  MPI_Bcast(&word_length_min, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (!rank) {
    std::cout << "Enter the maximum length of words to consider: ";
    std::cin >> word_length_max;
  }
  MPI_Bcast(&word_length_max, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (!rank) {
    std::cout << "Enter 'a' for alphabetical order or 'n' for number of words "
                 "order: ";
    std::cin >> order;
  }
  MPI_Bcast(&order, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

  long line_count = 0;
  long total_lines = 0;

  // Farhan Narodi - 1201302096
  // read the files on rank 0 and count the number of lines, store the lines in
  // a vector
  std::vector<std::string> rank_0_lines;
  for (int i = 0; i < num_txt_files; i++) {
    if (!rank) {
      std::fstream new_file;
      new_file.open(file_list[i], std::ios::in);
      if (new_file.is_open()) {
        std::string tp;
        while (getline(new_file, tp)) {
          line_count += 1;
          rank_0_lines.push_back(tp);
        }
      }
      new_file.close();
    }
  }

  // Farhan Narodi - 1201302096
  // broadcast the line count to other ranks from rank 0
  MPI_Bcast(&line_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Anirban Bala Pranto - 1181202317
  int count = 0;
  for (int i = 0; i < line_count; i++) {
    if (!rank) {
      // Anirban Bala Pranto - 1181202317
      // if rank 0, we read a line, we send the line length to another rank
      // along with the line, we use round robin technique to send the lines
      int line_len = rank_0_lines[i].length();
      MPI_Send(&line_len, 1, MPI_INT, i % (size - 1) + 1, 0, MPI_COMM_WORLD);
      MPI_Send(rank_0_lines[i].c_str(), line_len + 1, MPI_CHAR,
               i % (size - 1) + 1, 0, MPI_COMM_WORLD);
    } else {
      if (rank && (i % (size - 1) + 1) == rank) {
        // if rank is not 0, we receive a line lenght and the line of text from
        // rank 0 Anirban Bala Pranto - 1181202317
        int line_len;
        MPI_Recv(&line_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        char temp[line_len + 1];
        MPI_Recv(temp, line_len + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        std::string inp(temp);
        // we tokenize the line into words and locally count the words into a
        // hashmap Anirban Bala Pranto - 1181202317
        tokenizer(str_tolower(inp), ' ', words, word_length_min,
                  word_length_max, count);
      }
    }
  }

  int buffer_sz;
  // the received words being serialized by each rank
  // Anirban Bala Pranto - 1181202317
  std::string buf = serializer(words);

  // if we are in rank zero we receive a serialized map from other ranks, we go
  // on a for loop and check for a string data from each rank Anirban Bala
  // Pranto - 1181202317
  if (!rank) {
    for (int i = 1; i < size; i++) {
      MPI_Recv(&buffer_sz, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      char buffer[buffer_sz + 1];
      MPI_Recv(&buffer, buffer_sz + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      std::string s(buffer);
      // we deserialize the received words counts into a single map and combine
      // the counts Anirban Bala Pranto - 1181202317
      deserializer(s, words, word_counter, word_length_min, word_length_max);
    }
  } else {
    buffer_sz = buf.length();
    // if we are not in rank 0, we simply send the serialized word count to rank
    // 0 Anirban Bala Pranto - 1181202317
    MPI_Send(&buffer_sz, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(buf.c_str(), buffer_sz + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  end = MPI_Wtime();

  // we measure the runtime of the parallelization
  // Anirban Bala Pranto - 1181202317
  double time = (end - start) * MPI_Wtick();
  printf("Execution time for rank %d : %.9f seconds\n", rank, time);

  // Reynard - 1181203212
  if (!rank) {
    freopen("output_log_parallel.txt", "w", stdout);
    if (order == 'a') {
      // output the word count in alphabetical order
      // Chew Wei Ren - 1181203505
      std::cout << "Word Count Report (Alphabetical Order):" << std::endl;
      for (auto x : words) {
        std::cout << x.first << ": " << x.second << std::endl;
      }
    } else {
      // output the word in word count order, use the set and comparator
      // function to sort Reynard - 1181203212

      std::cout << "Word Count Report (Number of Words Order):" << std::endl;
      std::set<std::pair<std::string, int>, comp> answers(words.begin(),
                                                          words.end());
      for (auto x : answers) {
        std::cout << x.first << ": " << x.second << std::endl;
      }
    }
  }
  MPI_Finalize();
}
