#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr
        << "Give in the command line a file name to compare with expected.\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (!file.is_open()) {
    std::cerr << "Error opening file " << argv[1] << std::endl;
    return 2;
  }

  std::vector<double> data;
  std::copy(std::istream_iterator<double>(file),
            std::istream_iterator<double>(), std::back_inserter(data));

  std::string expected_filename{"expected_"};
  expected_filename += argv[1];

  std::ifstream expected_file(expected_filename);
  if (!expected_file.is_open()) {
    std::cerr << "Error opening file " << expected_filename << std::endl;
    return 2;
  }

  std::vector<double> expected;
  std::copy(std::istream_iterator<double>(expected_file),
            std::istream_iterator<double>(), std::back_inserter(expected));

  if (data.size() != expected.size()) {
    std::cerr << "Files have different number of elements.\n";
    return 3;
  }

  std::cout << expected.size() << " elements read.\n";

  for (size_t i = 0; i < data.size(); ++i) {
    if (std::fabs(data[i] - expected[i]) > 1e-5) {
      std::cerr << "Files differ at element " << i
                << "\nExpected: " << expected[i] << ", found: " << data[i]
                << std::endl;
      return 3;
    }
  }
  std::cout << "Files match!\n";

  return 0;
}
