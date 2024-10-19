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
hypothesisTree::hypothesisTree(std::string fileName, std::string tName, bool matchType) : df(ROOT::RDataFrame(tName, fileName)), treeName(tName), matchByBestPerBeam(matchType) {}

// checks whether the tree`'s event map contains a combo of event ID "eventID"
bool hypothesisTree::containsEventID(unsigned long long eventID) const {
    return (eventAsKeyMap.find(eventID) != eventAsKeyMap.end());
}
// same as containsEventID() but uses std::pair as key. to be replaced with template.
bool hypothesisTree::containsEventIDAndBeam(std::pair<unsigned long long, unsigned> pair_key) const {
  return (eventBeamAsKeyMap.find(pair_key) != eventBeamAsKeyMap.end());
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
  // fill the eventBeamAsKey map using newly created std::pair as key
  if (matchByBestPerBeam) {
    auto pair_key = std::make_pair(eventColumnData[index], beamColumnData[index]);

    eventBeamAsKeyMap[pair_key].setChiSq(chiSqColumnData[index]);
    eventBeamAsKeyMap[pair_key].setEvent(eventColumnData[index]);
    eventBeamAsKeyMap[pair_key].setRun(runColumnData[index]);
    eventBeamAsKeyMap[pair_key].setBeam(beamColumnData[index]);
    eventBeamAsKeyMap[pair_key].setNDF(ndfColumnData[index]);
    return;
  }
  // set eventAsKey map using event ID as key
  eventAsKeyMap[eventColumnData[index]].setChiSq(chiSqColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setEvent(eventColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setRun(runColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setBeam(beamColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setNDF(ndfColumnData[index]);
}

// for every row, remove duplicate rows (by shared event ID) and keep only combo lowest chisq
void hypothesisTree::filterHighChiSqEventsByBeam() {
    for (size_t i = 0; i < eventColumnData.size(); ++i) {
        if (!containsEventIDAndBeam(std::make_pair(eventColumnData[i], beamColumnData[i]))) {
            updateComboData(i);
        } else {
            if (eventBeamAsKeyMap[std::make_pair(eventColumnData[i], beamColumnData[i])].getChiSq() > chiSqColumnData[i]) {
                updateComboData(i);
            }
        }
    }
}

void hypothesisTree::filterHighChiSqEventsByEvent() {
    for (size_t i = 0; i < eventColumnData.size(); ++i) {
        if (!containsEventID(eventColumnData[i])) {
            updateComboData(i);
        } else {
            if (eventAsKeyMap[eventColumnData[i]].getChiSq() > chiSqColumnData[i]) {
                updateComboData(i);
            }
        }
    }
}

// constructor for compareHypotheses manager class. initializes two hypothesisTrees and the match counter.
compareHypotheses::compareHypotheses(std::string f1, std::string t1, std::string f2, std::string t2, bool matchType)
                                                         : tree1(f1, t1, matchType), tree2(f2, t2, matchType), matches(0), matchByBestPerBeam(matchType) {}

// iterates over tree1's events and checks them against tree2's events.
// events are logged to log_matches.txt and are stored in the matchedChiSqs map.
void compareHypotheses::findMatches() {
  std::ofstream os("log_matches.txt");

  if (!os.good()) {
    std::cerr << "Error: Could not open log file." << std::endl;
    return;
  }
  std::cout << "Number of unfiltered events in tree1: " << tree1.eventColumnData.size() << " Number of unfiltered events in tree2: " << tree2.eventColumnData.size() << std::endl;

  // matchByBestPerBeam true, match by best combo per beam ID (ie. use <unsigned long long, combo> eventBeamAsKey map)
  // TODO: implement a template to allow matching by either map type without this essentially duplicate guard if statement
  if (matchByBestPerBeam) {
    /* for (const auto& pair : tree1.eventBeamAsKeyMap) {
      eventAsKeyMap[(pair.first).first] = pair.second;
    } */

    for (const auto& pair : tree1.eventBeamAsKeyMap) {
    if (tree2.containsEventIDAndBeam(pair.first)) {
      matchedChiSqsByBeam[pair.first] = tree2.eventBeamAsKeyMap[pair.first].getChiSq() / tree2.eventBeamAsKeyMap[pair.first].getNDF();
      matches++;
      os << "Event ID: " << (pair.first).first << " found in both trees. Run IDs: " << pair.second.getRun() << ',' << tree2.eventBeamAsKeyMap[pair.first].getRun() <<
          " Beam IDs: " << pair.second.getBeamID() << ',' << tree2.eventBeamAsKeyMap[pair.first].getBeamID() << '\n';
      // implement verbose mode printing of four-vectors
    }
  }
  os.close();
  return;
  }

  // matchByBestPerBeam false, match by best overall combo (<unsigned long long, combo> eventAsKey map)
  for (const auto& pair : tree1.eventAsKeyMap) {
    if (tree2.containsEventID(pair.first)) {
      matchedChiSqs[pair.first] = tree2.eventAsKeyMap[pair.first].getChiSq() / tree2.eventAsKeyMap[pair.first].getNDF();
      matches++;
      os << "Event ID: " << pair.first << " found in both trees. Run IDs: " << pair.second.getRun() << ',' << tree2.eventAsKeyMap[pair.first].getRun() <<
          " Beam IDs: " << pair.second.getBeamID() << ',' << tree2.eventAsKeyMap[pair.first].getBeamID() << '\n';
      // implement verbose mode printing of four-vectors
    }
  }
  os.close();  
}

// load hypothesisTrees' member data from file and cut all high-chisq combos
void compareHypotheses::prepareData() {
  if (matchByBestPerBeam) {
    tree1.fillColumnVecs();
    tree1.filterHighChiSqEventsByBeam();
    tree2.fillColumnVecs();
    tree2.filterHighChiSqEventsByBeam();
    return;
  }
  tree1.fillColumnVecs();
  tree1.filterHighChiSqEventsByEvent();
  tree2.fillColumnVecs();
  tree2.filterHighChiSqEventsByEvent();

}

// writes alternative chisq values into new branch. if no match is found, placeholder chisq is written instead.
void compareHypotheses::writeToFile(std::string outFile) {
  // replace output filename placeholder (see main.cpp) with tree name from member function
  if (outFile == "placeholder") {
    outFile = tree2.getTreeName() + "_hypothesesMatched.root";
  }

  
  // write to file if in match by best per beam mode. to be replaced by generics.
  if (matchByBestPerBeam) {
      // Capture matchedChiSqs by reference
    auto& matchedChiSqsRef = matchedChiSqsByBeam;
    // helper variable for cleaner Define call
    auto newBranchName = tree2.getTreeName() + "_chisq_ndf";

    auto df3 = tree1.df.Define(newBranchName, [&matchedChiSqsRef](unsigned long long event, unsigned beam) {
      // check if event is in matchedChiSqs
      if (matchedChiSqsRef.find(std::make_pair(event, beam)) != matchedChiSqsRef.end()) {
        return matchedChiSqsRef[std::make_pair(event, beam)];
      } else {
        return static_cast<float>(185100000.0);    // using large number to indicate no match
      }
    }, {"event", "beam_beamid"});

    // write the newly defined RDataFrame to the file specified by user or the default arg
    df3.Snapshot("hypothesesMatched", outFile);
    return;
  }

  // write to file if in best overall match mode. 
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