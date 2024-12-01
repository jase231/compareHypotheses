#include <string>
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
#include <chrono>

struct combo {
  public:
    // getters for member data
    unsigned long long getEvent() const { return event; }
    unsigned int getRun() const { return run; }
    unsigned int getBeamID() const { return beam_beamid; }
    float getChiSq() const { return kin_chisq; }
    unsigned getNDF() const { return kin_ndf; }

    // setters for member data
    void setChiSq(float chiSq) { kin_chisq = chiSq; }
    void setEvent(unsigned long long e) { event = e; }
    void setRun(unsigned int r) { run = r; }
    void setBeam(unsigned int b) { beam_beamid = b; }
    void setNDF(unsigned n) { kin_ndf = n; }
  private:
    unsigned long long event;
    unsigned int run;
    unsigned int beam_beamid;
    float kin_chisq;
    unsigned kin_ndf;
};

class hypothesisTree {
  public:
    hypothesisTree(std::string fileName, std::string treeName, bool matchType);

    // data preperation functions
    void updateComboData(size_t index);
    void filterHighChiSqEventsByBeam();
    void filterHighChiSqEventsByEvent();
    void fillColumnVecs();

    // check whether matching criteria is already mapped in their respective combo maps
    bool containsEventID(unsigned long long) const;
    bool containsEventIDAndBeam(std::pair<unsigned long long, unsigned>) const;

    // helper for getting tree name
    std::string getTreeName() const { return treeName; }
    
    // RDataFrame
    ROOT::RDataFrame df;

    // ColumnData vectors
    std::vector<unsigned long long> eventColumnData;
    std::vector<unsigned int> runColumnData;
    std::vector<unsigned int> beamColumnData;
    std::vector<float> chiSqColumnData;
    std::vector<unsigned> ndfColumnData;

    // combo maps
    std::map<std::pair<unsigned long long, unsigned>, combo> eventBeamAsKeyMap;
    std::map<unsigned long long, combo> eventAsKeyMap;
    

  private:
    std::string treeName;
    bool matchByBestPerBeam;  // whether matching by best combo per beam is used
};

class compareHypotheses {
  private:
    hypothesisTree tree1;
    hypothesisTree tree2;
    bool verbose;
    bool matchByBestPerBeam;  // whether matching by best combo per beam is used
  public:
    compareHypotheses(std::string file1, std::string tree1, std::string file2, std::string tree2, bool matchType);
    
    // calls each tree's data preperation functions
    void prepareData();

    // performs combo matching between each tree's combo map
    void findMatches();

    // outputs into a new branch in a clone of the primary RDataFrame
    void writeToFile(std::string outFile);

    // counter for number of matches
    uint matches;

    // maps for storing chisq of matched combos from secondary tree
    std::map<std::pair<unsigned long long, unsigned>, float> matchedChiSqsByBeam;
    std::map<unsigned long long, float> matchedChiSqs;

    // helpers and member data setters
    bool isVerbose() const { return verbose; }
    void setVerbose(bool v) { verbose = v; }
    void setMatchByBeam(bool b) { matchByBestPerBeam = b; }
};