#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "analysis.h"

void showHelp()
{
    std::cout << "Usage: program <bloodTestImagePath>" << std::endl;
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

    const auto info = TestAnalysis::detectConcentrations(image);

    return 0;
}
