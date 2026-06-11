import os
import subprocess
import sys

def _comp_hyp_wrapper():
    # find the executable
    bindir = os.path.join(os.path.dirname(__file__), "bin/")
    exe = os.path.join(bindir, "compare_hypotheses")
    # pass arguments through to the executable
    args = [exe] + sys.argv[1:]
    # run the executable and return the status code
    out = subprocess.run(args)
    return out.returncode

