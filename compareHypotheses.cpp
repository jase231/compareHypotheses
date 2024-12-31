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
hypothesisTreeBase::hypothesisTreeBase(std::string fileName, std::string treeName, bool matchType)
    : df(ROOT::RDataFrame(treeName, fileName)), 
      treeName(treeName),
      matchByBestPerBeam(matchType) {}


// same as containsEventID() but uses std::pair as key. to be replaced with template.
bool hypothesisTreeBase::containsEventID(std::pair<unsigned long long, unsigned> pair_key) const {
  if (!matchByBestPerBeam) {
    return (eventAsKeyMap.find(pair_key.first) != eventAsKeyMap.end());
  }
  return (eventBeamAsKeyMap.find(pair_key) != eventBeamAsKeyMap.end());
}

// loads relevant columns from the RDataFrame
void hypothesisTreeBase::fillColumnVecs() {
    eventColumnData = *df.Take<unsigned long long>("event");
    runColumnData = *df.Take<unsigned int>("run");
    beamColumnData = *df.Take<unsigned int>("beam_beamid");
    chiSqColumnData = *df.Take<float>("kin_chisq");
    ndfColumnData = *df.Take<unsigned>("kin_ndf");
}

// fill each combo with data from the data columns
void hypothesisTreeBestCombo::updateComboData(size_t index) {
  // set eventAsKey map using event ID as key
  eventAsKeyMap[eventColumnData[index]].setChiSq(chiSqColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setEvent(eventColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setRun(runColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setBeam(beamColumnData[index]);
  eventAsKeyMap[eventColumnData[index]].setNDF(ndfColumnData[index]);
}

// set combo data for best combo per beam
void hypothesisTreeBestPerBeam::updateComboData(size_t index) {
  // set eventBeamAsKey map using event ID and beam ID as key
  auto pair_key = std::make_pair(eventColumnData[index], beamColumnData[index]);

  eventBeamAsKeyMap[pair_key].setChiSq(chiSqColumnData[index]);
  eventBeamAsKeyMap[pair_key].setEvent(eventColumnData[index]);
  eventBeamAsKeyMap[pair_key].setRun(runColumnData[index]);
  eventBeamAsKeyMap[pair_key].setBeam(beamColumnData[index]);
  eventBeamAsKeyMap[pair_key].setNDF(ndfColumnData[index]);
  return;
}

// for every row, remove duplicate rows (by shared event ID) and keep only combo lowest chisq
void hypothesisTreeBestCombo::filterHighChiSqEvents() {
  for (size_t i = 0; i < eventColumnData.size(); ++i) {
    auto pair_key = std::make_pair(eventColumnData[i], 1851); // placeholder beam ID
    if (!containsEventID(pair_key)) {
      updateComboData(i);
    } else {
      if (eventAsKeyMap[eventColumnData[i]].getChiSq() > chiSqColumnData[i]) {
        updateComboData(i);
      }
    }
  }
}

void hypothesisTreeBestPerBeam::filterHighChiSqEvents() {
    for (size_t i = 0; i < eventColumnData.size(); ++i) {
      auto pair_key = std::make_pair(eventColumnData[i], beamColumnData[i]);
      if (!containsEventID(pair_key)) {
        updateComboData(i);
      } else {
        if (eventBeamAsKeyMap[pair_key].getChiSq() > chiSqColumnData[i]) {
          updateComboData(i);
        }
      }
    }
}

// constructor for compareHypotheses manager class. initializes two hypothesisTrees and the match counter.
compareHypotheses::compareHypotheses(std::string f1, std::string t1, std::string f2, std::string t2, bool matchType, bool pCombos) : matches(0), matchByBestPerBeam(matchType), preserveCombos(pCombos) {
  if (matchByBestPerBeam) {
    tree1 = new hypothesisTreeBestPerBeam(f1, t1, matchType);
    tree2 = new hypothesisTreeBestPerBeam(f2, t2, matchType);
} else {
    tree1 = new hypothesisTreeBestCombo(f1, t1, matchType);
    tree2 = new hypothesisTreeBestCombo(f2, t2, matchType);
}
}

// load hypothesisTrees' member data from file and cut all high-chisq combos
void compareHypotheses::prepareData() {
  tree1->fillColumnVecs();
  tree1->filterHighChiSqEvents();
  tree2->fillColumnVecs();
  tree2->filterHighChiSqEvents();
}

// epsilon is estimated based on smallest difference between different chisq values for the same event in dataset
bool compareHypotheses::chisqsEqual(const float& a, const float& b) {
  static constexpr float epsilon = 1e-5f;
  if (std::abs(a - b) < epsilon) {
    return true;
  } else {
    return false;
  }
}


// iterates over tree1's events and checks them against tree2's events.
// events are logged to log_matches.txt and are stored in the matchedChiSqs map.
void compareHypotheses::findMatches() {
  std::ofstream os("log_matches.txt");

  if (!os.good()) {
    std::cerr << "Error: Could not open log file." << std::endl;
    return;
  }
  std::cout << "Number of unfiltered events in tree1: " << tree1->eventColumnData.size() << " Number of unfiltered events in tree2: " << tree2->eventColumnData.size() << std::endl;

  // matchByBestPerBeam true, match by best combo per beam ID (ie. use <unsigned long long, combo> eventBeamAsKey map)
  // TODO: implement a template to allow matching by either map type without this essentially duplicate guard if statement
  if (matchByBestPerBeam) {
    for (const auto& pair : tree1->eventBeamAsKeyMap) {
    if (tree2->containsEventID(pair.first)) {
      // check if run IDs also match
      if (tree2->eventBeamAsKeyMap[pair.first].getRun() != pair.second.getRun()) { continue; }

      matchedChiSqsByBeam[pair.first] = tree2->eventBeamAsKeyMap[pair.first].getChiSq() / tree2->eventBeamAsKeyMap[pair.first].getNDF();
      matches++;
      os << "Event ID: " << (pair.first).first << " found in both trees. Run IDs: " << pair.second.getRun() << ',' << tree2->eventBeamAsKeyMap[pair.first].getRun() <<
          " Beam IDs: " << pair.second.getBeamID() << ',' << tree2->eventBeamAsKeyMap[pair.first].getBeamID() << '\n';
      // implement verbose mode printing of four-vectors
    }
  }
  os.close();
  return;
  }

  // matchByBestPerBeam false, match by best overall combo (<unsigned long long, combo> eventAsKey map)
  for (const auto& pair : tree1->eventAsKeyMap) {
    auto key_pair = std::make_pair(pair.first, 1851); // use templates instead of placeholder
    if (tree2->containsEventID(key_pair)) {
      // check if run IDs also match
      if (tree2->eventAsKeyMap[pair.first].getRun() != pair.second.getRun()) { continue; }

      matchedChiSqs[pair.first] = tree2->eventAsKeyMap[pair.first].getChiSq() / tree2->eventAsKeyMap[pair.first].getNDF();
      matches++;
      os << "Event ID: " << pair.first << " found in both trees. Run IDs: " << pair.second.getRun() << ',' << tree2->eventAsKeyMap[pair.first].getRun() <<
          " Beam IDs: " << pair.second.getBeamID() << ',' << tree2->eventAsKeyMap[pair.first].getBeamID() << '\n';
      // implement verbose mode printing of four-vectors
    }
  }
  os.close();  
}

// writes alternative chisq values into new branch. if no match is found, placeholder chisq is written instead.
// if preserveCombos is false (which is the default), only the most probable combos from the primary tree and their matches are written.
void compareHypotheses::writeToFile(std::string outFile) {
  if (outFile == "placeholder") {
    outFile = tree2->getTreeName() + "_hypothesesMatched.root";
  }

  auto NEW_BRANCH_NAME = tree2->getTreeName() + "_chisq_ndf";
  float NO_MATCH_INDICATOR = 185100000.0f;

  if (matchByBestPerBeam) {
    auto& matchedChiSqsRef = matchedChiSqsByBeam;
    auto& tree1eventMap = tree1->eventBeamAsKeyMap;
    
    auto df3 = tree1->df
      .Define(NEW_BRANCH_NAME, [&matchedChiSqsRef, NO_MATCH_INDICATOR](unsigned long long event, unsigned beam) {
        if (matchedChiSqsRef.find(std::make_pair(event, beam)) != matchedChiSqsRef.end()) {
          return matchedChiSqsRef[std::make_pair(event, beam)];
        } else {
          return NO_MATCH_INDICATOR;    // using large number to indicate no match
        }
      }, {"event", "beam_beamid"})
      .Filter([&matchedChiSqsRef, &tree1eventMap, this](unsigned long long event, unsigned beam, float kin_chisq) {
        return chisqsEqual(kin_chisq, tree1eventMap[std::make_pair(event, beam)].getChiSq()) || preserveCombos;
      }, {"event", "beam_beamid", "kin_chisq"});

    df3.Snapshot("hypothesesMatched", outFile);
    return;
  }
  
  // matching by best overall combo
  auto& matchedChiSqsRef = matchedChiSqs;
  auto& tree1eventMap = tree1->eventAsKeyMap;

  auto df3 = tree1->df
    .Define(NEW_BRANCH_NAME, [&matchedChiSqsRef, NO_MATCH_INDICATOR](unsigned long long event) {
      // check if event is in matchedChiSqs
      if (matchedChiSqsRef.find(event) != matchedChiSqsRef.end()) {
        return matchedChiSqsRef[event];
      } else {
        return NO_MATCH_INDICATOR;    // using large number to indicate no match
      }
    }, {"event"})
    .Filter([&matchedChiSqsRef, &tree1eventMap, this](unsigned long long event, float kin_chisq) {
      return chisqsEqual(kin_chisq, tree1eventMap[event].getChiSq()) || preserveCombos;
  }, {"event", "kin_chisq"});

  df3.Snapshot("hypothesesMatched", outFile);
}