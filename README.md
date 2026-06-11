1. Clone the repo: `git clone -b python --single-branch git@github.com:jase231/compareHypotheses.git && cd compareHypotheses`
2. Create venv: `uv venv`
3. Install dependencies: `uv pip install -r requirements.txt`
4. Create a symlink to fix bug caused by PyPI ROOT distribution build configuration:
   - `cd .venv/lib/python3.13`
   - `ln -s site-packages mock_site_packages`
   - `cd ../../../`
6. Copy `thisroot.sh` into the PyPI ROOT distribution: `cp thisroot.sh .venv/lib64/python3.13/site-packages/ROOT/bin/`
7. Execute `thisroot.sh`: `source .venv/lib64/python3.13/site-packages/ROOT/bin/thisroot.sh` (fix python version as needed)
8. Install the package: `uv pip install .`
9. Check that that the executable has been built and is linked into PATH: `compare_hypotheses`
10. Check that the package is available from python: `python`
-   `from comp_hyp.test_suite import test_suite`
-   `test_suite.test_suite()`
-   To clean, call `python comp_hyp/test_suite/test_suite.py clean`
