#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

size_t count_lines(std::string filepath) {
  // file must be full path (no missing extensions)

  // make sure input exists
  if ( ! std::filesystem::exists( filepath ) ) {
    std::cerr << "Error: input file doesn't exist: " << filepath << "\n";
    return 0;
  }
  
  // create input filestream
  std::ifstream file( filepath );

  // count with iterators!
  size_t num_lines = std::count(
				std::istreambuf_iterator<char>(file),
				std::istreambuf_iterator<char>(),
				'\n'
				);
  
  // increment count once more if file didn't end with newline
  file.unget();
  if ( file.get() != '\n' )
    num_lines++;
  
  return num_lines;
}
