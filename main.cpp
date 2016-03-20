#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "analysis.h"

void showHelp()
{
    std::cout << "Usage: program <bloodTestImagePath>" << std::endl;
}

std::ostream &operator<<(std::ostream &out, const TestAnalysis::TestInfo &testInfo)
{
    out << testInfo.conc.at(TestAnalysis::SampleType::C1) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::C2) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::C3) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::C4) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::C5) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::C6) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::QC1) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::QC2) << std::endl;
    out << testInfo.conc.at(TestAnalysis::SampleType::Tested) << std::endl;

    out << std::endl;

    out << "QC1 VAR: " << testInfo.qc1Variation << std::endl; 
    out << "QC2 VAR: " << testInfo.qc2Variation << std::endl; 
    return out;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        showHelp();
        return -1;
    }

    cv::Mat image = cv::imread(argv[1]);
    if (!image.data) {
        std::cout << "Can't open image: " << argv[1] << std::endl;
        return -1;
    }

    std::cout << TestAnalysis::detectConcentrations(image);

    return 0;
}
