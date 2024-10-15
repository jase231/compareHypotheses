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

int main(int argc, char* argv[]) {

  // get the file and tree names from args
  std::string file1, file2, tree1, tree2;
  int setCounter = 0;
  bool verbose = false;

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
    }
  }

  // not enough parameters passed
  if (setCounter != 4) {
    std::cerr << "Missing required parameters. ";
    std::cout << "Usage: " << argv[0] << " --file1 <file1.root> --tree1 <tree1> --file2 <file2.root> --tree2 <tree2>\n";
    return 1;
  
  
  }
  std::cout << "Pre-processing data..." << std::endl;
  compareHypotheses c(file1, tree1, file2, tree2);
  c.setVerbose(verbose);

  c.prepareData();
  std::cout << "Data prepared, finding matches..." << std::endl;
  c.findMatches();

  std::cout << "Number of matches: " << c.matches << std::endl;

  std::cout << "Writing to file...\n";
  c.writeToFile("output.root");
  
  return 0;
}