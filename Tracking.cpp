#include <iostream>
#include <getopt.h>
#include "opencv2/highgui.hpp"
#include "opencv2/tracking.hpp"
#include "opencv2/objdetect.hpp"


//g++ Tracking.cpp -o Tracking `pkg-config --cflags --libs opencv4`
//./Tracking -i /home/jarvis/anyway_projects/ISR_NAU/Video_test_CV_10sec.mp4 -o /home/jarvis/anyway_projects/ISR_NAU -l /home/jarvis/anyway_projects/ISR_NAU -t KCF -r 15

using namespace cv;
using namespace std;

static void CallBackMouseROI(int event, int x, int y, int flags, void* img);   //must be before struct to fill it 

struct initRoi {
	//init of
	int init;

	//initial coordinates based on EVENT_LBUTTONDOWN
	double initX;
	double initY;

	//actual coordinates
	double actualX;
	double actualY;

	//Select Rect
	cv::Rect2d roiRect;
	int available;
	//Selected Mat roi
	cv::Mat takenRoi;

} SelectedRoi;

static double XCoord = 0;
static double YCoord = 0;

int main(int argc, char **argv)
{
	string trackerTypes[6] = {"Boosting", "MIL", "KCF", "TLD", "MedianFlow", "GOTURN"};
	string trackerType = trackerTypes[2];
	char* videoFileNameIn = NULL;
	std::string videoOutput = "";
    std::string defaultRoiPercentage = "5";
    char* logFileOut = NULL;
    char logFileName[] = "roiLog.csv";
	char opt;
	int wrongArgument = 0;
	int roiType = 0;

	cv::namedWindow("tracking", cv::WINDOW_AUTOSIZE);

	while ((opt = getopt(argc, argv, "t:i:o:l:r:")) != -1) {
		if (optarg && optarg[0] == '-') {
			wrongArgument = 1;
			std::cout << "Argument starting with '-' is not allowed " << endl;
			break;
		}
		switch (opt) {
            case 't':
				trackerType = string(optarg);
				break;
			case 'i':
				videoFileNameIn = optarg;
				break;
            case 'o':
				videoOutput = string(optarg) + "outputVideo.mp4";
				break;
            case 'l':
				logFileOut = strcpy(optarg, logFileName);
				break;
			case 'r':
				defaultRoiPercentage = string(optarg);
				break;
			case '?':
				wrongArgument = 1;
				break;
			default:
				break;
		}
	}

	if (!videoFileNameIn) {
		wrongArgument = 1;
	}

	if (wrongArgument) {
		std::cout << "Wrong argument" << std::endl;
		std::cout << "Usage: -i <input_video_file> [-o output_video_path] [-l output_log_path] [-t tracker_type] [-r roi_percentage]" << endl
			  << " 'tracker_type' can be: Boosting, MIL, KCF, TLD, MedianFlow, GOTURN. Default is KCF" << endl
			  << " If 'roi percentage' is given, the program will set it. Otherwise, default 5% will be used." << endl;
		return -1;
	}

	VideoCapture video(videoFileNameIn);
	if (!video.isOpened()) {
		std::cout << "Open video failed: " << videoFileNameIn << endl;
		return -1;
	}

	int frameWidth = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));
    Size frameSize(static_cast<int>(frameWidth), static_cast<int>(frameHeight));
	int roiWidth = (frameWidth / 100) * std::stoi(defaultRoiPercentage);
	int roiHeight = (frameHeight / 100) * std::stoi(defaultRoiPercentage);
	bool drawRoiFromCenter = true;
	namedWindow("tracking", WINDOW_AUTOSIZE);
    std::cout << "frameWidth is : " << frameWidth << " frameHeight is : " << frameHeight << std::endl;
    int codec = VideoWriter::fourcc('a', 'v', 'c', '1');
    VideoWriter objVideoWriter(videoOutput, codec, 30, Size(frameSize), true);

	int i;
	for (i = 0; i < 6; i++) {
		if (trackerType == trackerTypes[i]) {
			break;
		}
	}
	if (i == 6) {
		std::cout << "Unknown tracker type. Will use KCL as default" << std::endl;
		trackerType = trackerTypes[2];
	}
	Ptr<Tracker> tracker;
	if (trackerType == trackerTypes[0])
		tracker = TrackerBoosting::create();
	if (trackerType == trackerTypes[1])
		tracker = TrackerMIL::create();
	if (trackerType == trackerTypes[2])
		tracker = TrackerKCF::create();
	if (trackerType == trackerTypes[3])
		tracker = TrackerTLD::create();
	if (trackerType == trackerTypes[4])
		tracker = TrackerMedianFlow::create();
	if (trackerType == trackerTypes[5])
		tracker = TrackerGOTURN::create();

	Mat frame;
	Rect2d roi;
	
	FILE *dataFile = fopen(logFileOut, "w");

	if (dataFile == NULL) {
		std::cout << "Open data file failed: %s" << logFileOut << std::endl;
	}
	int frameIndex = 0;

    video.read(frame);

	std::cout << "No ROI selected " << std::endl;

	roi = selectROI("tracking", frame, drawRoiFromCenter);

	if (roi.area() == 0) 
	{
		std::cout << "No ROI selected " << std::endl;
		setMouseCallback("tracking", CallBackMouseROI, 0);
		roi.x = XCoord;
		roi.y = YCoord;
		roi.width = roiWidth;
		roi.width = roiHeight;
	}

	tracker->init(frame, roi);
	fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, (int)roi.x, (int)roi.y, (int)roi.width, (int)roi.height);
	
	std::cout << "Start the tracking process from frame, press 'q' to quit " << frameIndex << std::endl;

	while (1) {
		video.read(frame);

		// If the frame is empty, break immediately.
		if (frame.empty()) {
			std::cout << "No more frame " << std::endl;
			break;
		}
		frameIndex++;

		bool ok = tracker->update(frame, roi);

		if (ok) {

			// Draw the tracked object.
			cv::rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);
			fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, (int)roi.x, (int)roi.y, (int)roi.width, (int)roi.height);
            cv::putText(frame, "roi.x:" + to_string((int)roi.x), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 255), 2);
            cv::putText(frame, "roi.y:" + to_string((int)roi.y), Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.60, Scalar(255, 255, 255), 2);
            cv::putText(frame, "roi.width:" + to_string((int)roi.width), Point(100, 110), FONT_HERSHEY_SIMPLEX, 0.60, Scalar(255, 255, 255), 2);
            cv::putText(frame, "roi.height:" + to_string((int)roi.height), Point(100, 140), FONT_HERSHEY_SIMPLEX, 0.60, Scalar(255, 255, 255), 2);

			// Display tracker type on frame.
			cv::putText(frame, trackerType + " tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 255, 0), 2);
			//write the frame into the file
            objVideoWriter.write(frame); 

            std::cout << "roi.x:" << (int)roi.x << " " << "roi.y:" << (int)roi.y << " " 
                      << "roi width:" << (int)roi.width << " " << "roi height:" << (int)roi.height << std::endl;
                     
		} else {
			cv::putText(frame, "Tracking failure", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
			objVideoWriter.write(frame); 
			fprintf(dataFile, "%d %d %d %d %d\n", frameIndex, 0, 0, 0, 0);
		}

		cv::imshow("tracking", frame);
		// Press 'q' on keyboard to quit.
		char c = (char)cv::waitKey(25);
		if (c == 'q') {
			break;
		}
	}

	if (dataFile) {
		std::fclose(dataFile);
	}
	return 0;
}

static void CallBackMouseROI(int event, int x, int y, int flags, void* img)
{
	if (event == EVENT_RBUTTONDOWN)
	{
		SelectedRoi.actualX = x;
		SelectedRoi.actualY = y;
		std::cout << "right button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
	}
	return;

	if (event == EVENT_LBUTTONDOWN)
	{
		SelectedRoi.initX = x;
		SelectedRoi.initY = y;
	}

	if (event == EVENT_LBUTTONUP)
	{
		SelectedRoi.actualX = x;
		SelectedRoi.actualY = y;
		SelectedRoi.roiRect = Rect(SelectedRoi.initX, SelectedRoi.initY, SelectedRoi.actualX - SelectedRoi.initX, SelectedRoi.actualY - SelectedRoi.initY);
	}
}


