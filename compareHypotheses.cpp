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
#include <fstream>
#include <limits>

// constructor for the hypothesis trees. passes input directly to RDataFrame constructor.
hypothesisTree::hypothesisTree(std::string fileName, std::string tName) : df(ROOT::RDataFrame(tName, fileName)), treeName(tName) {}

// checks whether the tree's event map contains a combo of event ID "eventID"
bool hypothesisTree::containsEventID(unsigned int eventID) const {
    return (events.find(eventID) != events.end());
}

// loads relevant columns from the RDataFrame
void hypothesisTree::fillColumnVecs() {
    eventColumnData = *df.Take<unsigned long long>("event");
    runColumnData = *df.Take<unsigned int>("run");
    beamColumnData = *df.Take<unsigned int>("beam_beamid");
    chiSqColumnData = *df.Take<float>("kin_chisq");
    ndfColumnData = *df.Take<unsigned>("kin_ndf");
}

// fill each combo with data from the data columns
void hypothesisTree::updateComboData(size_t index) {
  events[eventColumnData[index]].setChiSq(chiSqColumnData[index]);
  events[eventColumnData[index]].setEvent(eventColumnData[index]);
  events[eventColumnData[index]].setRun(runColumnData[index]);
  events[eventColumnData[index]].setBeam(beamColumnData[index]);
  events[eventColumnData[index]].setNDF(ndfColumnData[index]);
}

// for every row, remove duplicate rows (by shared event ID) and keep only combo lowest chisq
void hypothesisTree::filterHighChiSqEvents() {
    for (size_t i = 0; i < eventColumnData.size(); ++i) {
        if (!containsEventID(eventColumnData[i])) {
            updateComboData(i);
        } else {
            if (events[eventColumnData[i]].getChiSq() > chiSqColumnData[i]) {
                updateComboData(i);
            }
        }
    }
}
// constructor for compareHypotheses manager class. initializes two hypothesisTrees and the match counter.
compareHypotheses::compareHypotheses(std::string f1, std::string t1, std::string f2, std::string t2)
                                                         : tree1(f1, t1), tree2(f2, t2), matches(0) {}

// iterates over tree1's events and checks them against tree2's events.
// events are logged to log_matches.txt and are stored in the matchedChiSqs map.
void compareHypotheses::findMatches() {
  std::ofstream os("log_matches.txt");

  if (!os.good()) {
    std::cerr << "Error: Could not open log file." << std::endl;
    return;
  }
  std::cout << "Number of unfiltered events in tree1: " << tree1.eventColumnData.size() << " Number of unfiltered events in tree2: " << tree2.eventColumnData.size() << std::endl;

  for (const auto& pair : tree1.events) {
    if (tree2.containsEventID(pair.first)) {
      if (tree2.events[pair.first].getRun() != pair.second.getRun() || 
          tree2.events[pair.first].getBeamID() != pair.second.getBeamID()) {
        continue;
      }
      matchedChiSqs[pair.first] = tree2.events[pair.first].getChiSq() / tree2.events[pair.first].getNDF();
      matches++;
      os << "Event ID: " << pair.first << " found in both trees. Run IDs: " << pair.second.getRun() << ',' << tree2.events[pair.first].getRun() <<
          " Beam IDs: " << pair.second.getBeamID() << ',' << tree2.events[pair.first].getBeamID() << '\n';
      // implement verbose mode printing of four-vectors
    }
  }
  os.close();  
}

// load hypothesisTrees' member data from file and cut all high-chisq combos
void compareHypotheses::prepareData() {
  tree1.fillColumnVecs();
  tree1.filterHighChiSqEvents();
  tree2.fillColumnVecs();
  tree2.filterHighChiSqEvents();
}

// writes alternative chisq values into new branch. if no match is found, placeholder chisq is written instead.
void compareHypotheses::writeToFile(std::string outFile) {
  // replace output filename placeholder (see main.cpp) with tree name from member function
  if (outFile == "placeholder") {
    outFile = tree2.getTreeName() + "_hypothesesMatched.root";
  }

  // Capture matchedChiSqs by reference
  auto& matchedChiSqsRef = matchedChiSqs;
  // helper variable for cleaner Define call
  auto newBranchName = tree2.getTreeName() + "_chisq_ndf";

  auto df3 = tree1.df.Define(newBranchName, [&matchedChiSqsRef](unsigned long long event) {
    // check if event is in matchedChiSqs
    if (matchedChiSqsRef.find(event) != matchedChiSqsRef.end()) {
      return matchedChiSqsRef[event];
    } else {
      return static_cast<float>(185100000.0);    // using large number to indicate no match
    }
  }, {"event"});

  // write the newly defined RDataFrame to the file specified by user or the default arg
  df3.Snapshot("hypothesesMatched", outFile);
}