#include <iostream>
#include <array>
#include <limits>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{
    const size_t cSamplesPoints = 8;
    const cv::Point cAreaShift(60, 60);

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

std::map<SampleType, double> buildConcentrationBySampleMap()
{
    std::map<SampleType, double> concentrationMap;
    concentrationMap[SampleType::C1] = 0.0;
    concentrationMap[SampleType::C2] = 62.5;
    concentrationMap[SampleType::C3] = 125.0;
    concentrationMap[SampleType::C4] = 250.0;
    concentrationMap[SampleType::C5] = 500.0;
    concentrationMap[SampleType::C6] = 1000.0;
    concentrationMap[SampleType::QC1] = 156.0;
    concentrationMap[SampleType::QC2] = 750.0;
    return concentrationMap;
}

double concentrationBySampleType(SampleType sampleType)
{
    static const auto cConcMap = buildConcentrationBySampleMap();
    assert(cConcMap.count(sampleType) > 0);
    return cConcMap.at(sampleType);
}

cv::Mat buildRedMask(const cv::Mat &image)
{
    cv::Mat hsvImage;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    cv::Mat lowerRange;
    cv::Mat upperRange;
    cv::inRange(hsvImage, cv::Scalar(0, 50, 50), cv::Scalar(10, 255, 255), lowerRange);
    cv::inRange(hsvImage, cv::Scalar(160, 50, 50), cv::Scalar(179, 255, 255), upperRange);
    lowerRange += upperRange;

    const int erosion_type = cv::MORPH_ELLIPSE;
    const int erosion_size = 15;
    const cv::Mat element = cv::getStructuringElement(erosion_type,
                                                cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                                cv::Point(erosion_size, erosion_size));

    cv::erode(lowerRange, lowerRange, element);
    return lowerRange;
}

std::array<cv::Point, cSamplesPoints> findSamplesPoints(const cv::Mat &image)
{
    cv::Mat filteredImage = buildRedMask(image);

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

    const auto termCriteria = cv::TermCriteria(
            CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
            cIterations,
            cEps);

    cv::kmeans(inputData, cSamplesPoints, labels, termCriteria,
               cTrials, cv::KMEANS_PP_CENTERS, clustersCenters);

    std::array<cv::Point, cSamplesPoints> pointsCenters;

    for (size_t i = 0; i < cSamplesPoints; ++i) {
        pointsCenters[i] = clustersCenters.at<cv::Point2f>(i);
    }

    return pointsCenters;
}

std::map<SampleType, cv::Point> getMarkedPointsMap(const std::array<cv::Point, cSamplesPoints> &points)
{
    /*
     * Assume the following samples displacement
     *
     *  C1 | QC2 | C6
     *  -------------
     *  C2 | Sam | C5
     *  -------------
     *  C3 | QC1 | C4
     */

    double minDist = std::numeric_limits<double>::max();

    for (size_t i = 0; i + 1 < cSamplesPoints; ++i) {
        for (size_t j = i + 1; j < cSamplesPoints; ++j) {
            minDist = std::min(minDist, cv::norm(points[i] - points[j]));
        }
    }

    auto sortedPoints = points;
    std::sort(sortedPoints.begin(), sortedPoints.end(),
            [minDist](const cv::Point &p1, const cv::Point &p2) {
                return p1.x + minDist / 2 < p2.x || (abs(p1.x - p2.x) < minDist / 2 && p1.y < p2.y);
    });

    const std::array<SampleType, cSamplesPoints> sortedSampleTypes = {
        SampleType::C2,
        SampleType::C3,
        SampleType::QC2,
        SampleType::Tested,
        SampleType::QC1,
        SampleType::C6,
        SampleType::C5,
        SampleType::C4
    };

    std::map<SampleType, cv::Point> markedPointsMap;
    for (size_t i = 0; i < cSamplesPoints; ++i) {
        markedPointsMap[sortedSampleTypes[i]] = sortedPoints[i];
    }

    // add C1 sample position

    const cv::Point c1Point1 = markedPointsMap[SampleType::C2] +
                               (markedPointsMap[SampleType::C2] -
                                markedPointsMap[SampleType::C3]);

    const cv::Point c1Point2 = markedPointsMap[SampleType::QC2] +
                               (markedPointsMap[SampleType::QC2] -
                                markedPointsMap[SampleType::C6]);

    const cv::Point c1PointMean = 0.5 * (c1Point1 + c1Point2);
    markedPointsMap[SampleType::C1] = c1PointMean;
    return markedPointsMap;
}

std::map<SampleType, cv::Mat> getSamplesRoisMap
(
    const cv::Mat &image,
    const std::map<SampleType, cv::Point> &pointsMap
)
{
    std::map<SampleType, cv::Mat> roisMap;

    for (auto iter = pointsMap.cbegin(); iter != pointsMap.cend(); ++iter) {
        const auto &p = iter->second;
        const cv::Rect rect(p - cAreaShift, p + cAreaShift);
        roisMap[iter->first] = cv::Mat(image, rect);
    }

    return roisMap;
}

double getMeanIntensity(const cv::Mat &image)
{
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    std::vector<unsigned char> values;
    for (int i = 0; i < gray.rows; ++i) {
        for (int j = 0; j < gray.cols; ++j) {
            values.emplace_back(gray.at<uchar>(i, j));
        }
    }

    std::nth_element(values.begin(), values.begin() + values.size() / 2, values.end());
    return values[values.size() / 2];
}

double getConcentrationValue(const cv::Vec2f &model, double intensity)
{
    return std::exp(model[0] * intensity + model[1]) - 1;
}

std::vector<SampleType> getAllSampleTypes()
{
    return {
        SampleType::C1,
        SampleType::C2,
        SampleType::C3,
        SampleType::C4,
        SampleType::C5,
        SampleType::C6,
        SampleType::QC1,
        SampleType::QC2,
        SampleType::Tested,
    };
}

double getMeanIntensity(const cv::Point &origin, const cv::Mat &mask, const cv::Mat &image)
{
    cv::Rect boundingRect;
    cv::Mat cloned = mask.clone();
    static const uchar cColorValue = 1;
    cv::floodFill(cloned, origin, cv::Scalar(cColorValue), &boundingRect);

    double intensitySum = 0;
    size_t qty = 0;

    for (int r = 0; r < boundingRect.height; ++r) {
        for (int c = 0; c < boundingRect.width; ++c) {
            const int row = boundingRect.tl().y + r;
            const int col = boundingRect.tl().x + c;
            if (cloned.at<uchar>(row, col) == cColorValue) {
                intensitySum += image.at<uchar>(row, col);
                qty++;
            }
        }
    }

    return intensitySum / qty;
}

std::map<SampleType, double> buildMeanIntensitiesMap
(
    const std::map<SampleType, cv::Point> &origins,
    const cv::Mat &mask,
    const cv::Mat &image
)
{
    cv::Mat grayscale;
    cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);

    std::map<SampleType, double> intensityMap;
    for (const auto &sampleType : getAllSampleTypes()) {
        intensityMap[sampleType] = getMeanIntensity(origins.at(sampleType), mask, grayscale);
    }

    intensityMap[SampleType::C1] = 255;

    return intensityMap;
}

cv::Vec2f buildLinearLogScaleModel(const std::map<SampleType, double> &intensityMap)
{
    std::vector<cv::Point2f> points;
    for (const auto &r : intensityMap) {
        if (r.first != SampleType::Tested && r.first != SampleType::QC1 && r.first != SampleType::QC2) {
            const auto meanValue = r.second;
            const auto conc = std::log(concentrationBySampleType(r.first) + 1);
            points.emplace_back(meanValue, conc);
        }
    }

    cv::Vec4f line;
    cv::fitLine(points, line, CV_DIST_L1, 0, 0.0001, 0.0001);
    const double b = line[3] - line[1] * (line[2] / line[0]);
    const double k = line[1] / line[0];
    return cv::Vec2f(k, b);
}

std::string sampleTypeName(SampleType sampleType)
{
#define SWITCH_SAMPLE(x) case SampleType::x: return #x;
    switch (sampleType) {
        SWITCH_SAMPLE(C1);
        SWITCH_SAMPLE(C2);
        SWITCH_SAMPLE(C3);
        SWITCH_SAMPLE(C4);
        SWITCH_SAMPLE(C5);
        SWITCH_SAMPLE(C6);
        SWITCH_SAMPLE(QC1);
        SWITCH_SAMPLE(QC2);
        SWITCH_SAMPLE(Tested);
    }
#undef SWITCH_SAMPLE

    assert(false && "All cases must be handled.");
    return "";
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

    cv::GaussianBlur(image, image, cv::Size(5, 5), 0);

    const auto samplesPoints = findSamplesPoints(image);
    const auto markedPoints = getMarkedPointsMap(samplesPoints);
    const cv::Mat mask = buildRedMask(image);
    const auto meanIntensityMap = buildMeanIntensitiesMap(markedPoints, mask, image);
    const cv::Vec2f lineCoef = buildLinearLogScaleModel(meanIntensityMap);

    for (const auto &sample : getAllSampleTypes()) {
        const auto intensity = meanIntensityMap.at(sample);
        std::cout << sampleTypeName(sample) << ": "
                  << "MEAN INTENSITY = " << intensity << ", "
                  << "ESTIMATED CONCENTRATION = " << getConcentrationValue(lineCoef, intensity)
                  << std::endl;
    }

    const auto printVariation = [&lineCoef, &meanIntensityMap] (SampleType sampleType) {
        const auto actualConc =  getConcentrationValue(lineCoef, meanIntensityMap.at(sampleType));
        const auto refConc = concentrationBySampleType(sampleType);
        std::cout << "VARIATION " << sampleTypeName(sampleType) << " = "
                  << std::fabs(actualConc - refConc) / refConc * 100 << "%"
                  << std::endl;
    };

    printVariation(SampleType::QC1);
    printVariation(SampleType::QC2);

#ifdef DEBUG
    for (int i = 0; i < mask.rows; ++i) {
        for (int j = 0; j < mask.cols; ++j) {
            if (mask.at<uchar>(i, j) == 0) {
                image.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 255, 0);
            }
        }
    }

    image = cv::Mat(image, cv::Rect(cv::Point(1000, 900), cv::Size(800, 800)));
    showImage(image);
#endif

    return 0;
}
