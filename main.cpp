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
#include "compareHypotheses.h"
#include <chrono>

using namespace std::chrono;

int main(int argc, char* argv[]) {
  // init benchmarking
  high_resolution_clock::time_point t1 = high_resolution_clock::now();


  // get the file and tree names from args
  std::string file1, file2, tree1, tree2;
  std::string outFile = "placeholder";
  int setCounter = 0;
  bool verbose = false;
  bool bestByBeam = false;  // find best overall combo by default
  bool preserveCombos = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      std::cout << "Usage: " << argv[0] << " --file1 <file1.root> --tree1 <tree1> --file2 <file2.root> --tree2 <tree2>\n";
    } else if (strcmp(argv[i], "--file1") == 0 || strcmp(argv[i], "-f1") == 0) {
      file1 = argv[i+1];
      setCounter++;
    } else if (strcmp(argv[i], "--tree1") == 0 || strcmp(argv[i], "-t1") == 0) {
      tree1 = argv[i+1];
      setCounter++;
    } else if (strcmp(argv[i], "--file2") == 0 || strcmp(argv[i], "-f2") == 0) {
      file2 = argv[i+1];
      setCounter++;
    } else if (strcmp(argv[i], "--tree2") == 0 || strcmp(argv[i], "-t2") == 0) {
      tree2 = argv[i+1];
      setCounter++;
    } else if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
      verbose = true;
    } else if (strcmp(argv[i], "--outfile") == 0 || strcmp(argv[i], "-o") == 0) {
      // TODO: check that filename is valid
      outFile = argv[i+1];
    } else if (strcmp(argv[i], "--best-per-beam") == 0 || strcmp(argv[i], "-b") == 0) {
      bestByBeam = true;
    } else if (strcmp(argv[i], "--preserve-combos") == 0 || strcmp(argv[i], "-p") == 0) {
      preserveCombos = true;
    }
  }

  // not enough parameters passed
  if (setCounter != 4) {
    std::cerr << "Missing required parameters. ";
    std::cout << "Usage: " << argv[0] << " --file1 <file1.root> --tree1 <tree1> --file2 <file2.root> --tree2 <tree2>\n";
    return 1;
  }

  if (bestByBeam) {
    std::cout << "Running in best combo per beam ID mode.\n";
  } else {
    std::cout << "Running in best overall combo mode. To run in best combo per beam ID mode, pass the -bb flag.\n";
  }
  std::cout << "Pre-processing data..." << std::endl;
  compareHypotheses c(file1, tree1, file2, tree2, bestByBeam, preserveCombos);
  c.setVerbose(verbose);
  c.setMatchByBeam(bestByBeam);
  
  c.prepareData();
  std::cout << "Data prepared, finding matches..." << std::endl;
  c.findMatches();

  std::cout << "Number of matches: " << c.matches << std::endl;

  // benchmark the matching process
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  auto matchingDuration = duration_cast<microseconds>( t2 - t1 ).count();
  std::cout << "The matching process took: " << matchingDuration*1E-6 << " seconds\n";



  std::cout << "Writing to file...\n";
  c.writeToFile(outFile);

  // benchmark the writing-to-file
  high_resolution_clock::time_point t3 = high_resolution_clock::now();
  auto writeDuration = duration_cast<microseconds>( t3 - t2 ).count();
  std::cout << "The file-writing process took: " << writeDuration*1E-6 << " seconds\n";

  return 0;
}