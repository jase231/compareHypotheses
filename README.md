1. Clone the repo: `git clone -b python --single-branch git@github.com:jase231/compareHypotheses.git && cd compareHypotheses`
2. Create venv: `uv venv`
3. Install dependencies: `uv pip install -r requirements.txt`
4. Create a symlink to fix bug caused by PyPI ROOT distribution build configuration:
   - `cd .venv/lib/python3.13`
   - `ln -s site-packages mock_site_packages`
   - `cd ../../../`
5. Copy `thisroot.sh` into the PyPI ROOT distribution: `cp thisroot.sh .venv/lib64/python3.13/site-packages/ROOT/bin/`
6. Execute `thisroot.sh`: `source .venv/lib64/python3.13/site-packages/ROOT/bin/thisroot.sh` (fix python version as needed)

If building wheel:
1. `python -m build`
2. Then, exit your venv, open a fresh directory, copy the wheel there
3. Create new venv
4. `uv pip install ROOT`
5. `uv pip install <comparehypotheses-etc-etc.whl>

If installing the package:
1. `uv pip install .`

To confirm proper install:
1. Check that that the executable has been built and is linked into PATH: `compare_hypotheses`
2. Check that the package is available from python: `python`
-   `from comp_hyp.test_suite import test_suite`
-   `test_suite.test_suite()`
-   To clean, call `python comp_hyp/test_suite/test_suite.py clean`
