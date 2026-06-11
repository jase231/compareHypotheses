import ROOT
import os
import sys

def _gauss_plot():
    # create a RDataFrame and define a branch with random values
    df = ROOT.RDataFrame(10000)
    ROOT.gRandom.SetSeed(1)
    df_rand = df.Define("rng", "normal_dist_gen();")

    # book histogram and write it to png
    hist = df_rand.Histo1D("rng")
    c = ROOT.TCanvas("rng_hist", "rng_hist", 800, 400)
    hist.Draw()
    c.SaveAs("rng_normal_hist.png")
    return "rng_normal_hist.png"

def test_suite():
    # disable info print spam
    ROOT.gErrorIgnoreLevel = ROOT.kWarning


    # include the C++ components
    _here = os.path.dirname(os.path.abspath(__file__))
    ROOT.gInterpreter.AddIncludePath(_here)
    ROOT.gInterpreter.Declare('#include "string_define_filter.h"')
    ROOT.gInterpreter.Declare('#include "rnode.h"')
    ROOT.gInterpreter.Declare('#include "build_rntuple.h"') 
    ROOT.gInterpreter.Declare('#include "angularconv.h"')
    ROOT.gInterpreter.Declare('#include "gauss.h"')
    ROOT.gInterpreter.Declare('#include "legendre.h"')

    # construct the gaussian generator class
    ROOT.gInterpreter.ProcessLine('gauss_rng normal_dist_gen{};')
    
    string_d_f_result = ROOT.string_define_filter()
    rnode_result = ROOT.rnode()
    rntuple_root_file = ROOT.build_rntuple()
    angularconv_plot_path = ROOT.angularconv()
    gauss_plot_path = _gauss_plot()
    legendre_path = "legendre.png"
    ROOT.legendre(legendre_path)

    if len(sys.argv) > 1:
        if sys.argv[1] == "clean":
            os.remove(angularconv_plot_path)
            os.remove(rntuple_root_file)
            os.remove(gauss_plot_path)
            os.remove(legendre_path)
            quit()

    # use python to obtain info from the rntuple root file
    reader = ROOT.RNTupleReader.Open("F", rntuple_root_file)
    
    print(string_d_f_result)
    print(rnode_result)
    print(f"Number of entries in generated RNTuple: {reader.GetNEntries()}")
    print(f"Wrote angular convolution model plot to {angularconv_plot_path}")
    print(f"Wrote normal distribution plot to {gauss_plot_path}")
    print(f"Wrote plot of legendre polynomials to {legendre_path}")
    


if __name__ == "__main__":
    test_suite()
