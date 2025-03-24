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
#include <fstream>
#include <limits>

// constructor for the hypothesis trees. passes input directly to RDataFrame constructor.
hypothesis_tree_base::hypothesis_tree_base(std::string file_name, std::string tree_name, bool match_type)
    : df(ROOT::RDataFrame(tree_name, file_name)), 
      tree_name(tree_name),
      match_by_best_per_beam(match_type) {}


// same as containsEventID() but uses std::pair as key. to be replaced with template.
bool hypothesis_tree_base::contains_event_id(std::pair<unsigned long long, unsigned> pair_key) const {
  if (!match_by_best_per_beam) {
    return (event_as_key_map.find(pair_key.first) != event_as_key_map.end());
  }
  return (event_beam_as_key_map.find(pair_key) != event_beam_as_key_map.end());
}

// loads relevant columns from the RDataFrame
void hypothesis_tree_base::fill_column_vecs() {
  event_column_data = *df.Take<unsigned long long>("event");
  run_column_data = *df.Take<unsigned int>("run");
  beam_column_data = *df.Take<unsigned int>("beam_beamid");
  chi_sq_column_data = *df.Take<float>("kin_chisq");
  ndf_column_data = *df.Take<unsigned>("kin_ndf");
}

// fill each combo with data from the data columns
void hypothesis_tree_best_combo::update_combo_data(size_t index) {
  // set eventAsKey map using event ID as key
  event_as_key_map[event_column_data[index]].set_chi_sq(chi_sq_column_data[index]);
  event_as_key_map[event_column_data[index]].set_event(event_column_data[index]);
  event_as_key_map[event_column_data[index]].set_run(run_column_data[index]);
  event_as_key_map[event_column_data[index]].set_beam(beam_column_data[index]);
  event_as_key_map[event_column_data[index]].set_ndf(ndf_column_data[index]);
}

// set combo data for best combo per beam
void hypothesis_tree_best_per_beam::update_combo_data(size_t index) {
  // set eventBeamAsKey map using event ID and beam ID as key
  auto pair_key = std::make_pair(event_column_data[index], beam_column_data[index]);

  event_beam_as_key_map[pair_key].set_chi_sq(chi_sq_column_data[index]);
  event_beam_as_key_map[pair_key].set_event(event_column_data[index]);
  event_beam_as_key_map[pair_key].set_run(run_column_data[index]);
  event_beam_as_key_map[pair_key].set_beam(beam_column_data[index]);
  event_beam_as_key_map[pair_key].set_ndf(ndf_column_data[index]);
  return;
}

// for every row, remove duplicate rows (by shared event ID) and keep only combo lowest chisq
void hypothesis_tree_best_combo::filter_high_chi_sq_events() {
  for (size_t i = 0; i < event_column_data.size(); ++i) {
    auto pair_key = std::make_pair(event_column_data[i], 1851); // placeholder beam ID
    if (!contains_event_id(pair_key)) {
      update_combo_data(i);
    } else {
      if (event_as_key_map[event_column_data[i]].get_chi_sq() > chi_sq_column_data[i]) {
        update_combo_data(i);
      }
    }
  }
}

void hypothesis_tree_best_per_beam::filter_high_chi_sq_events() {
  for (size_t i = 0; i < event_column_data.size(); ++i) {
    auto pair_key = std::make_pair(event_column_data[i], beam_column_data[i]);
    if (!contains_event_id(pair_key)) {
        update_combo_data(i);
      } else {
        if (event_beam_as_key_map[pair_key].get_chi_sq() > chi_sq_column_data[i]) {
          update_combo_data(i);
        }
      }
    }
}

// constructor for compare_hypotheses manager class. initializes two hypothesisTrees and the match counter.
compare_hypotheses::compare_hypotheses(std::string file_1, std::string tree_1, std::string file_2, std::string tree_2, bool match_type, bool p_combos) 
    : matches(0), match_by_best_per_beam(match_type), preserve_combos(p_combos) {
  if (match_by_best_per_beam) {
    tree1 = new hypothesis_tree_best_per_beam(file_1, tree_1, match_type);
    tree2 = new hypothesis_tree_best_per_beam(file_2, tree_2, match_type);
  } else {
    tree1 = new hypothesis_tree_best_combo(file_1, tree_1, match_type);
    tree2 = new hypothesis_tree_best_combo(file_2, tree_2, match_type);
  }
}

// load hypothesisTrees' member data from file and cut all high-chisq combos
void compare_hypotheses::prepare_data() {
  tree1->fill_column_vecs();
  tree1->filter_high_chi_sq_events();
  tree2->fill_column_vecs();
  tree2->filter_high_chi_sq_events();
}

// epsilon is estimated based on smallest difference between different chisq values for the same event in dataset
bool compare_hypotheses::chi_sqs_equal(const float& a, const float& b) {
  static constexpr float epsilon = 1e-5f;
  if (std::abs(a - b) < epsilon) {
    return true;
  } else {
    return false;
  }
}


// iterates over tree1's events and checks them against tree2's events.
// events are logged to log_matches.txt and are stored in the matched_chi_sqs map.
void compare_hypotheses::find_matches() {
  std::ofstream os("log_matches.txt");

  if (!os.good()) {
    std::cerr << "Error: Could not open log file." << std::endl;
    return;
  }
  std::cout << "Number of unfiltered events in tree1: " << tree1->event_column_data.size() 
            << " Number of unfiltered events in tree2: " << tree2->event_column_data.size() << std::endl;

  // match_by_best_per_beam true, match by best combo per beam ID
  if (match_by_best_per_beam) {
    for (const auto& pair : tree1->event_beam_as_key_map) {
      // get iterator
      auto tree2_it = tree2->event_beam_as_key_map.find(pair.first);
      
      // check if key exists
      if (tree2_it != tree2->event_beam_as_key_map.end()) {
        const combo& tree2_combo = tree2_it->second;
        
        // run ID match check
        if (tree2_combo.get_run() != pair.second.get_run()) { continue; }

        // store the match
        matched_chi_sqs_by_beam[pair.first] = tree2_combo.get_chi_sq() / tree2_combo.get_ndf();
        matches++;
        
        os << "Event ID: " << (pair.first).first 
           << " found in both trees. Run IDs: " << pair.second.get_run() << ',' << tree2_combo.get_run()
           << " Beam IDs: " << pair.second.get_beam_id() << ',' << tree2_combo.get_beam_id() << '\n';
      }
    }
    os.close();
    return;
  }

  // match_by_best_per_beam false, match by best overall combo
  for (const auto& pair : tree1->event_as_key_map) {
    // get iterator
    auto tree2_it = tree2->event_as_key_map.find(pair.first);
    
    if (tree2_it != tree2->event_as_key_map.end()) {
      const combo& tree2_combo = tree2_it->second;
      
      // run ID match check
      if (tree2_combo.get_run() != pair.second.get_run()) { continue; }

      // store the match
      matched_chi_sqs[pair.first] = tree2_combo.get_chi_sq() / tree2_combo.get_ndf();
      matches++;
      
      os << "Event ID: " << pair.first 
         << " found in both trees. Run IDs: " << pair.second.get_run() << ',' << tree2_combo.get_run()
         << " Beam IDs: " << pair.second.get_beam_id() << ',' << tree2_combo.get_beam_id() << '\n';
    }
  }
  os.close();  
}

// writes alternative chisq values into new branch. if no match is found, placeholder chisq is written instead.
// if preserve_combos is false (which is the default), only the most probable combos from the primary tree and their matches are written.
void compare_hypotheses::write_to_file(std::string out_file) {
  if (out_file == "placeholder") {
    out_file = tree2->get_tree_name() + "_hypothesesMatched.root";
  }

  auto NEW_BRANCH_NAME = tree2->get_tree_name() + "_chisq_ndf";
  float NO_MATCH_INDICATOR = 185100000.0f;

  if (match_by_best_per_beam) {
    auto& matched_chi_sqs_ref = matched_chi_sqs_by_beam;
    auto& tree1_event_map = tree1->event_beam_as_key_map;
    
    auto df3 = tree1->df
      // create new branch for the matched chisq values, fill it with the matched chisq values or placeholder if no match is found
      .Define(NEW_BRANCH_NAME, [&matched_chi_sqs_ref, NO_MATCH_INDICATOR](unsigned long long event, unsigned beam) {
        if (matched_chi_sqs_ref.find(std::make_pair(event, beam)) != matched_chi_sqs_ref.end()) {
          return matched_chi_sqs_ref[std::make_pair(event, beam)];
        } else {
          return NO_MATCH_INDICATOR;    // using large number to indicate no match
        }
      }, {"event", "beam_beamid"})
      // preserve only the lowest chisq combo per event ID & beam ID if preserveCombos is false
      .Filter([&matched_chi_sqs_ref, &tree1_event_map, this](unsigned long long event, unsigned beam, float kin_chisq) {
        return chi_sqs_equal(kin_chisq, tree1_event_map[std::make_pair(event, beam)].get_chi_sq()) || preserve_combos;
      }, {"event", "beam_beamid", "kin_chisq"});

    df3.Snapshot("hypothesesMatched", out_file);
    return;
  }
  
  // matching by best overall combo
  auto& matched_chi_sqs_ref = matched_chi_sqs;
  auto& tree1_event_map = tree1->event_as_key_map;

  auto df3 = tree1->df
    // create new branch for the matched chisq values, fill it with the matched chisq values or placeholder if no match is found
    .Define(NEW_BRANCH_NAME, [&matched_chi_sqs_ref, NO_MATCH_INDICATOR](unsigned long long event) {
      if (matched_chi_sqs_ref.find(event) != matched_chi_sqs_ref.end()) {
        return matched_chi_sqs_ref[event];
      } else {
        return NO_MATCH_INDICATOR;    // using large number to indicate no match
      }
    }, {"event"})
    // preserve only the lowest chisq combo per event ID if preserveCombos is false
    .Filter([&matched_chi_sqs_ref, &tree1_event_map, this](unsigned long long event, float kin_chisq) {
      return chi_sqs_equal(kin_chisq, tree1_event_map[event].get_chi_sq()) || preserve_combos;
  }, {"event", "kin_chisq"});

  df3.Snapshot("hypothesesMatched", out_file);
}