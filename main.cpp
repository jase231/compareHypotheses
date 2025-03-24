#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TLorentzVector.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <tuple>
#include <sstream>
#include <cstddef>
#include "compare_hypotheses.h"
#include <chrono>
#include "inih/INIReader.h"

using namespace std::chrono;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Please pass the configuration file.\nUsage: " << argv[0] << " <config.ini>\n";
    return 1;
  }
  // init benchmarking
  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  // read in config file
  INIReader reader(argv[1]);
  if (reader.ParseError() != 0) {
    std::cerr << "Error reading config file " << argv[1] << '\n';
    return 1;
  }
  
  // read primary and secondary configs
  std::string file1 = reader.Get("Primary", "file", "");
  std::string file2 = reader.Get("Secondary", "file", "");

  std::string tree1 = reader.Get("Primary", "tree", "");
  std::string tree2 = reader.Get("Secondary", "tree", "");

  // optional configs
  std::string out_file = reader.Get("Misc", "outfile", "placeholder");
  bool best_by_beam = reader.GetBoolean("Misc", "best_per_beam", false);
  bool preserve_combos = reader.GetBoolean("Misc", "preserve_combos", false);
  bool verbose = reader.GetBoolean("Misc", "verbose", false);

  // validate config
  if (file1.empty() || file2.empty() || tree1.empty() || tree2.empty()) {
    std::cerr << "Missing required parameters in config file.\n";
    std::cerr << "Required sections/keys:\n";
    std::cerr << "[Primary]\nfile = ...\ntree = ...\n\n";
    std::cerr << "[Secondary]\nfile = ...\ntree = ...\n";
    return 1;
}

  if (best_by_beam) {
    std::cout << "Running in best combo per beam ID mode.\n";
  } else {
    std::cout << "Running in best overall combo mode. To run in best combo per beam ID mode, pass the -bb flag.\n";
  }
  std::cout << "Pre-processing data..." << std::endl;
  compare_hypotheses c(file1, tree1, file2, tree2, best_by_beam, preserve_combos);
  c.set_verbose(verbose);
  c.set_match_by_beam(best_by_beam);
  
  c.prepare_data();
  std::cout << "Data prepared, finding matches..." << std::endl;
  c.find_matches();

  std::cout << "Number of matches: " << c.matches << std::endl;

  // benchmark the matching process
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  auto matching_duration = duration_cast<microseconds>( t2 - t1 ).count();
  std::cout << "The matching process took: " << matching_duration*1E-6 << " seconds\n";



  std::cout << "Writing to file...\n";
  c.write_to_file(out_file);

  // benchmark the writing-to-file
  high_resolution_clock::time_point t3 = high_resolution_clock::now();
  auto writeDuration = duration_cast<microseconds>( t3 - t2 ).count();
  std::cout << "The file-writing process took: " << writeDuration*1E-6 << " seconds\n";

  return 0;
}