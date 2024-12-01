# Chi-squared Hypothesis Comparison Tool

The GlueX experiment features nearly full angular coverage and can detect both neutral and charged particles. This enables reliable kinematic fitting for exclusive reactions. Since some particles decay into γ before identification, the kinematic fitter can evaluate alternative hypotheses for γ's possible progenitors. This tool helps identify events that are shared between different reconstruction hypotheses.

## Features

- Compare chi-squared values between two different reconstruction hypotheses
- Support for both best overall combo and best-per-beam-ID matching modes
- Output includes matched events with corresponding chi-square values
- Built on ROOT's RDataFrame for efficient data processing

## Requirements

- ROOT environment (tested on ROOT 6.24/04)
- C++ compiler (tested on GCC 4.8.5)
- Input files must contain a flat ROOT tree with the following required branches:
  - Event ID
  - Beam ID
  - ChiSq
  - NDF (Number of Degrees of Freedom)

## Installation

Clone the repository and compile using make:

```bash
git clone https://github.com/jase231/compareHypotheses
cd compareHypotheses
make
```

## Usage

### Basic Command Structure

```bash
./compareHypotheses [flags]
```


### Required Flags

- `--file1` / `-f1`: Primary .root file path
- `--file2` / `-f2`: Secondary .root file path
- `--tree1` / `-t1`: Name of the primary tree in file1
- `--tree2` / `-t2`: Name of the secondary tree in file2

### Optional Flags

- `--outfile`: Custom output filename (default: `<tree2>_hypothesesMatched.root`)
- `--best-per-beam` / `-bb`: Match by best combo per beam ID (default: match by best overall combo)

### Example

```bash
./compareHypotheses --file1 data1.root --file2 data2.root --tree1 pi0pippieta --tree2 pi0pippim --outfile results.root
```

## Output Format

The program generates a ROOT file containing:
- A copy of the primary tree
- New branch with matched secondary combos' χ²/NDF values
- Unmatched combos receive a placeholder value of 1.851e6

## Matching Criteria

Two matching modes are available:
1. Best Overall Combo: Selects the combo with lowest χ² per event
2. Best Per Beam ID: Selects the combo with lowest χ² for each unique event+beam ID pair; to be used alongside accidental subtraction

## Performance
The performance of the tool is limited by the ROOT library's I/O efficiency. In a test run comparing two trees with around 300,000 events each, the matching process takes 2 seconds for best overall mode and 3 seconds for best per beam matching mode. Writing the output to file using the ROOT library takes 30 seconds.

## Future Development

- [X] Benchmarking support
- [ ] Implementation of MC generated beam energy data support
- [ ] Output option for matched-only combos
- [ ] Generic implementation of matching logic
- [ ] Enhanced edge case handling for multiple equivalent matches

## Authors

Jake Serwe - Florida State University

## Credit

This tool is intended as a successor to Alex Barnes' [mergeTrees.C](https://github.com/JeffersonLab/halld_recon/tree/2ae9aa0b7569f847c54d5714af3139ec53f87e3c/src/programs/Utilities/mergeTrees)
