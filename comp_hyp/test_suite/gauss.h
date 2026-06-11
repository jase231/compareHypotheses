#include <random>
                        
class gauss_rng {
public:
    gauss_rng(double mean = 0, double dev = 5) : generator(std::random_device{}()), normalizer(mean, dev) {}
    double operator()() { return normalizer(generator); }

private:
    std::mt19937 generator;
    std::normal_distribution<double> normalizer;
};
