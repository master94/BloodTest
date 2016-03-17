#ifndef ANALYSIS_H
#define ANALYSIS_H

namespace TestAnalysis
{

enum class SampleType
{
    C1,
    C2,
    C3,
    C4,
    C5,
    C6,
    QC1,
    QC2,
    Tested
};

struct TestInfo
{
    std::map<SampleType, double> conc;
    double qc1Variation;
    double qc2Variation;
};

TestInfo detectConcentrations(const cv::Mat &input);

}

#endif // ANALYSIS_H
