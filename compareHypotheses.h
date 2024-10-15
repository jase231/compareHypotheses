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

class combo {
  public:
    unsigned long long getEvent() const { return event; }
    unsigned int getRun() const { return run; }
    unsigned int getBeamID() const { return beam_beamid; }
    float getChiSq() const { return kin_chisq; }
    unsigned getNDF() const { return kin_ndf; }

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
    hypothesisTree(std::string fileName, std::string treeName);
    void updateComboData(size_t index);
    void filterHighChiSqEvents();
    void fillColumnVecs();
    bool containsEventID(unsigned int key) const;

    ROOT::RDataFrame df;
    std::vector<unsigned long long> eventColumnData;
    std::vector<unsigned int> runColumnData;
    std::vector<unsigned int> beamColumnData;
    std::vector<float> chiSqColumnData;
    std::vector<unsigned> ndfColumnData;
    std::map<unsigned long long, combo> events;
    std::string getTreeName() const { return treeName; }

  private:
    std::string treeName;
};

class compareHypotheses {
  private:
    hypothesisTree tree1;
    hypothesisTree tree2;
    bool verbose;
  public:
    compareHypotheses(std::string file1, std::string tree1, std::string file2, std::string tree2);
    void findMatches();
    void prepareData();
    uint matches;
    std::map<unsigned long long, float> matchedChiSqs;
    bool isVerbose() const { return verbose; }
    bool setVerbose(bool v) { verbose = v; }
    void writeToFile(std::string outFile);
};