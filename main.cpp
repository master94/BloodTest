#include <iostream>
#include <array>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{
    const size_t cSamplesPoints = 8;
}

void showHelp()
{
    std::cout << "Usage: program <bloodTestImagePath>" << std::endl;
}

void showImage(const cv::Mat &image)
{
    static const char *cWindowName = "Image";
    cv::namedWindow(cWindowName, cv::WINDOW_AUTOSIZE);
    cv::imshow(cWindowName, image);
    cv::waitKey(0);
}

std::array<cv::Point, cSamplesPoints> findSamplesPoints(const cv::Mat &image)
{
    cv::Mat hsvImage;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    cv::Mat lowerRange;
    cv::Mat upperRange;
    cv::inRange(hsvImage, cv::Scalar(0, 50, 50), cv::Scalar(10, 255, 255), lowerRange);
    cv::inRange(hsvImage, cv::Scalar(160, 50, 50), cv::Scalar(179, 255, 255), upperRange);
    cv::Mat filteredImage = lowerRange + upperRange;

    std::vector<cv::Point> nonZeroPoints;
    cv::findNonZero(filteredImage, nonZeroPoints);

    cv::Mat inputData(nonZeroPoints.size(), 2, CV_32F);
    for (size_t i = 0; i < nonZeroPoints.size(); ++i) {
        inputData.at<float>(i, 0) = nonZeroPoints[i].x;
        inputData.at<float>(i, 1) = nonZeroPoints[i].y;
    }

    cv::Mat labels;
    cv::Mat clustersCenters;

    static const int cIterations = 10000;
    static const float cEps = 0.001;
    static const int cTrials = 5;

    const auto termCriteria = cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, cIterations, cEps);
    cv::kmeans(inputData, cSamplesPoints, labels, termCriteria, cTrials, cv::KMEANS_PP_CENTERS, clustersCenters);

    std::array<cv::Point, cSamplesPoints> pointsCenters;

    for (size_t i = 0; i < cSamplesPoints; ++i) {
        pointsCenters[i] = clustersCenters.at<cv::Point2f>(i);
    }

    return pointsCenters;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        showHelp();
        return -1;
    }

    cv::Mat image = cv::imread(argv[1]);

    const auto samplesPoints = findSamplesPoints(image);

    for (int i = 0; i < samplesPoints.size(); ++i) {
        const auto p = samplesPoints[i];
        cv::line(image, p, p, CV_RGB(0, 255, 0), 50);
    }

    cv::resize(image, image, cv::Size(image.cols / 4, image.rows / 4));
    showImage(image);

    return 0;
}
