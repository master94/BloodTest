#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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

int main(int argc, char **argv)
{
    if (argc != 2) {
        showHelp();
        return -1;
    }

    cv::Mat image = cv::imread(argv[1]);
    cv::resize(image, image, cv::Size(image.cols / 4, image.rows / 4));
    showImage(image);

    return 0;
}
