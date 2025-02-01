############################################################################################################
# This is a demo script for running the comparison tool on several different hypotheses
# with several files (runs) per hypothesis. This script presumes the following directory
# structure and naming conventions:
#
# | compareHypotheses (the executable)
# | output (directory to which output files will be saved)
# | hypotheses
# | | hypothesis1
# | | | tree_hypothesis1_flat_000001.root
# | | | ...
# | | hypothesis2
# | | | tree_hypothesis2_flat_000001.root
# | | | ...
# | | hypothesis3
# | | | tree_hypothesis3_flat_000001.root
# | | | ...
# NB: This script enumerates the number of files by looking into the primary hypothesis directory.
# All other hypotheses are assumed to have the same number of files, and all other hypotheses' directories
# are assumed to be at the same level as the primary hypothesis directory. (see the directory structure above)
# The script will need to be configured with the correct details in the section below.
# If your DSelector outputs follow a different naming convention, especially with respect
# to the run number, you will need to modify the `extract_run_num` function.
# For additional hypotheses, you will need to add additional command templates,
# making sure to run said templates and remove intermediate files as necessary.

############################################## CONFIGURATION ###############################################
# the directory containing the main hypothesis files. must be a subdirectory of main directory (see above).
DIRECTORY = "/some/directory/hypotheses/hypothesis1"
# the primary hypothesis tree name
PRIMARY_TREE = "hypothesis1"
# the secondary hypothesis tree name
SECONDARY_TREE = "hypothesis2"
# the tertiary hypothesis tree name
TERTIARY_TREE = "hypothesis3"
# the quaternary hypothesis tree name
QUATERNARY_TREE = "hypothesis4"
# the directory to which output files will be saved
OUTPUT_DIR = "/some/directory/hypotheses/output"

# command templates
# {} are placeholders for the file path and run number. if the order of the placeholders changes, the order of the arguments in the format functions must change accordingly
COMMAND_TEMPLATE_HYP1 = "./compareHypotheses -f1 {} -t1 {} -f2 {}/../{}/tree_{}_flat_{}.root -t2 {} -bb --outfile {}/intermediate1_{}.root"
COMMAND_TEMPLATE_HYP2 = "./compareHypotheses -f1 {}/intermediate1_{}.root -t1 hypothesesMatched -f2 {}/../{}/tree_{}_flat_{}.root -t2 {} -bb --outfile {}/intermediate2_{}.root"
COMMAND_TEMPLATE_HYP3 = "./compareHypotheses -f1 {}/intermediate2_{}.root -t1 hypothesesMatched -f2 {}/../{}/tree_{}_flat_{}.root -t2 {} -bb --outfile {}/final_output{}.root"
COMMAND_TEMPLATE_CLEANUP = "rm {}/intermediate1_{}.root {}/intermediate2_{}.root"
############################################################################################################

import os
import subprocess

def run_command_for_files(directory):
    # ensure the directory exists
    if not os.path.isdir(directory):
        print(f"Directory '{directory}' does not exist.")
        return

    # iterate over each file in the directory
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        run = extract_run_num(filename)

        # ensure it's a file (not a directory)
        if os.path.isfile(file_path):
            # format the commands with the file path and run
            command_hyp1 = COMMAND_TEMPLATE_HYP1.format(file_path, PRIMARY_TREE, directory, SECONDARY_TREE, SECONDARY_TREE, run, SECONDARY_TREE, OUTPUT_DIR, run)
            command_hyp2 = COMMAND_TEMPLATE_HYP2.format(OUTPUT_DIR, run, directory, TERTIARY_TREE, TERTIARY_TREE, run, TERTIARY_TREE, OUTPUT_DIR, run)
            command_hyp3 = COMMAND_TEMPLATE_HYP3.format(OUTPUT_DIR, run, directory, QUATERNARY_TREE, QUATERNARY_TREE, run, QUATERNARY_TREE, OUTPUT_DIR, run)
            command_cleanup = COMMAND_TEMPLATE_CLEANUP.format(OUTPUT_DIR, run, OUTPUT_DIR, run)
            
            '''
            # for testing, print the commands instead of running
            print(f"Running command: {command_hyp1}")
            print(f"Running command: {command_hyp2}")
            print(f"Running command: {command_hyp3}")
            print(f"Running command: {command_cleanup}")
            '''

            # run the commands
            print(f"Running command: {command_hyp1}")            
            try:
                result = subprocess.run(command_hyp1, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                print(f"Output:\n{result.stdout.decode()}")
            except subprocess.CalledProcessError as e:
                print(f"Error running command for file '{file_path}':\n{e.stderr.decode()}")
                continue
            
            print(f"Running command: {command_hyp2}")
            try:
                result = subprocess.run(command_hyp2, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                print(f"Output:\n{result.stdout.decode()}")
            except subprocess.CalledProcessError as e:
                print(f"Error running command for file '{file_path}':\n{e.stderr.decode()}")
                continue
            
            print(f"Running command: {command_hyp3}")
            try:
                result = subprocess.run(command_hyp3, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                print(f"Output:\n{result.stdout.decode()}")
            except subprocess.CalledProcessError as e:
                print(f"Error running command for file '{file_path}':\n{e.stderr.decode()}")
                continue    
            
            # remove intermediates
            print(f"Running command: {command_cleanup}")
            try:
                result = subprocess.run(command_cleanup.format(run, run), shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                print(f"Output:\n{result.stdout.decode()}")
            except subprocess.CalledProcessError as e:
                print(f"Error running command for file '{file_path}':\n{e.stderr.decode()}")
                continue

# returns the run number from filename (assumes the run number is the last 6 characters of the filename before .root)
def extract_run_num(filename):
    return filename[-11:-5]

if __name__ == "__main__":
    run_command_for_files(DIRECTORY)