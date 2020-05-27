#include <opencv2\opencv.hpp>
#include <cstdio>
#include <vector>
#include <iostream>
#include <string>
#include <io.h>
#include <conio.h>

using namespace std;
using namespace cv;

bool contains(Rect first, Rect second) {
	return (first.x - 3) <= second.x && (second.x + second.width) <= (first.x + first.width + 3) && (first.y - 3) <= second.y && (second.y + second.height) <= (first.y + 3 + first.height);
}

 Mat RGBtoGRAY(Mat rgb) {
	 float GrayValue;
	 Mat mats[3];
	 split(rgb, mats);

	 for (int j = 0; j < rgb.rows; j++) {
		 for (int i = 0; i < rgb.cols; i++) {
			 GrayValue = 0.2126f * (rgb.at<Vec3b>(j, i)[2]) + 0.7152f * (rgb.at<Vec3b>(j, i)[1]) + 0.0722f * (rgb.at<Vec3b>(j, i)[0]);

			 if (GrayValue < 0.0) 
				 GrayValue = 0.0f;
			 else if (GrayValue > 255.0) 
				 GrayValue = 255.0f;

			 mats[0].at<uchar>(j, i) = (int) GrayValue;
		 }
	 }
	 return mats[0];
 }

 Mat erode_(Mat src, Mat kernel) {
	 Mat dst = src;
	 int iMin, iVal;

	 for (int j = 0; j < src.rows - 2; j++) {
		 for (int i = 0; i < src.cols - 2; i++) {
			 iMin = 0xFFF;
			 for (int x = 0; x < kernel.rows; x++) {
				 for (int y = 0; y < kernel.cols; y++) {
					 if (src.at<uchar>(y, x) != 0)
					 {
						 iVal = src.at<uchar>(j + y, i + x);
						 if (iMin > iVal)
						 {
							 iMin = iVal;
						 }
					 }

				 }

			 }
			 dst.at<uchar>(j, i) = iMin;

		 }
	 }
	return dst;
 }

 Mat morphologyEx_(Mat src, int op, Mat kernel) {
	 Mat dst = src;
	 switch (op) {
		 case MORPH_OPEN:
			 erode(src, dst, kernel);
			 dilate(src, dst, kernel);
			 break;
		 case MORPH_CLOSE:
			 dilate(src, dst, kernel);
			 dst = erode_(src, kernel);
			 break;
	 }
	 return dst;
 }

vector<Rect> detectShape(Mat img, bool joker)
{
	double max_ratio = -1.0, avg_ratio = -1.0;
	int check1, check2, row;
	int ratio = 100, min_ratio = -1;
	vector<Rect> boundRect;
	Mat img_gray, element;
	//printf("%d\n", img.total());
	img_gray = RGBtoGRAY(img);
	//cvtColor(img, img_gray, COLOR_BGR2GRAY);
	Canny(img_gray, img_gray, 100, 300, 3);
	element = getStructuringElement(MORPH_RECT, Size(joker ? 3 : 7, joker ? 3 : 7)); // 7 7
	if (joker) {
		for (int j = 0; j < element.rows; j++) {
			for (int i = 0; i < element.cols; i++) {
				printf("%3d ", element.at<uchar>(j, i));
			}
			puts("");
		}
		puts("");
		for (int j = 0; j < 20; j++) {
			for (int i = 0; i < 20; i++) {
				printf("%3d ", img_gray.at<uchar>(j, i));
			}
			puts("");
		}
	}
	imshow("o", img_gray);
	morphologyEx_(img_gray, MORPH_CLOSE, element);
	//morphologyEx(img_gray, img_gray, MORPH_CLOSE, element);
	imshow("m", img_gray);
	puts("");
	if (joker)
	for (int j = 0; j < 20; j++) {
		for (int i = 0; i < 20; i++) {
			printf("%3d ", img_gray.at<uchar>(j, i));
		}
		puts("");
	}
	vector< vector< Point> > contours;
	findContours(img_gray, contours, 3, 2);
	vector<vector<Point> > contours_poly(contours.size());
	
	for (row = 0; row < (img.rows / 4); row++) {
		check2 = check1 = -2;
		for (int i = 0; i < (img.cols / 5); i++) {
			//printf("%d %d / %d\n", row, i, img.at<Vec3b>(row, i)[0]);
			if (check2 == -2 && check1 == -2 && img.at<Vec3b>(row, i)[0] > 200) {
				check2 = check1 = -1;
			}
			else if (check1 == -1 && img.at<Vec3b>(row, i)[0] < 100)
				check1 = i;
			else if (check2 == -1 && check1 != -1 && img.at<Vec3b>(row, i)[0] > 200) {
				check2 = i;
				break;
			}
		}
		if (check1 > 0 && check2 > 0) {
			ratio = img.cols / (check2 - check1);
			if (min_ratio == -1 || min_ratio > ratio)
				min_ratio = ratio;
		}
	}
	//printf("%d\n", min_ratio);

	for (int i = 0; i < contours.size(); i++) {
		approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
		Rect appRect(boundingRect(Mat(contours_poly[i])));
		double ratio_ = (double) appRect.height / appRect.width;
		if ((!joker && (ratio_ >= 0.5 && ratio_ <= 1.5) && (appRect.area() < (img.total() / (min_ratio / 5 * 9)) && appRect.area() >= (img.total() / (min_ratio * 6)))) || (joker && (ratio_ >= 0.9 && ratio_ <= 6 && appRect.area() < (img.total() / (min_ratio * 4)) && appRect.area() >= (img.total() / (min_ratio * 8)) && appRect.y <= img.rows / 6))) {
			bool push = true;
			for (int j = 0; j < boundRect.size(); j++) {
				if (contains(boundRect.at(j), appRect)) {
					push = false;
					break;
				} else if (contains(appRect, boundRect.at(j))) {
					boundRect.at(j) = appRect;
					push = false;
					break;
				}
			}
			if (push) {
				boundRect.push_back(appRect);
			}
		}
	}
	if (boundRect.size() < 3 && joker) {
		boundRect.clear();

		for (int i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 1, true);
			Rect appRect(boundingRect(Mat(contours_poly[i])));
			double ratio_ = (double)appRect.height / appRect.width;
			if (ratio_ > 1 && ratio_ < 6 && appRect.x <= img.cols / 13 && appRect.area() <= (img.total() / (min_ratio * 4))&& appRect.area() >= (img.total() / (min_ratio * 16))) {
				//printf("%lf %d\n", ratio_, appRect.area());
				bool push = true;
				for (int j = 0; j < boundRect.size(); j++) {
					if (contains(boundRect.at(j), appRect)) {
						push = false;
						break;
					}
					else if (contains(appRect, boundRect.at(j))) {
						boundRect.at(j) = appRect;
						push = false;
						break;
					}
				}
				if (push) {
					boundRect.push_back(appRect);
				}
			}
		}
	}

	int tmp = 0, value = 0, value2 = 0, index;
	for (index = 0; index < boundRect.size() && !joker; index++) {
		value = tmp = 0;
		if (boundRect.at(index).y > (img.rows / 2))
			continue;
		for (int i = 0; i < boundRect.at(index).height; i++) {
			//printf("%d\n", img.at<Vec3b>(boundRect.at(index).y + i, boundRect.at(index).x + boundRect.at(index).width - 2)[0]);
			tmp++;
			if (img.at<Vec3b>(boundRect.at(index).y + i, boundRect.at(index).x + boundRect.at(index).width - 2)[0] < 250) {
				if (value == 0)
					value = tmp;
				value2++;
			}
		}
		//printf(" %d\n", value);
		if (value > 1)
			break;
	}
	if (value > 1 && !joker) {
		//printf("%d %d\n", value, boundRect.at(index).height);
		//printf("%d\n", value2);
		double ratio__ = (double)value / boundRect.at(index).height;
		

		if (ratio__ >= 0.59)
			printf("스페이드 ");
		else if (value2 > 5 && ratio__ >= 0.49)
			printf("클로버 ");
		else if (ratio__ > 0.4)
			printf("다이아몬드 ");
		else
			printf("하트 ");

		printf("%d\n", boundRect.size());

	}
	return boundRect;
}

int main()
{

	Mat image, drawing;
	string name;
	int result = 1, refinery_count, ratio;
	_finddata64i32_t fd;
	intptr_t handle;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	handle = _findfirst64i32("C:\\Users\\YJH\\Desktop\\트럼프 카드 이미지\\*.*", &fd);

	while (result != -1)
	{
		name = "C:\\Users\\YJH\\Desktop\\트럼프 카드 이미지\\";
		name.append(fd.name/*"red_joker.png"*/);
		image = imread(name);

		result = _findnext64i32(handle, &fd);
		if (image.empty())
			continue;

		vector<Rect> JokerBox = detectShape(image, true);
		if (JokerBox.size() >= 3) {
			bool black = false;
			for (int i = 0; i < JokerBox.size(); i++) {
				for (int y = JokerBox[i].y; y < JokerBox[i].y + JokerBox[i].height; y++) {
					for (int x = JokerBox[i].x; x < JokerBox[i].x + JokerBox[i].width; x++) {
						if (image.at<Vec3b>(y, x)[0] == 0)
							black = true;
					}
				}
				rectangle(image, JokerBox[i], Scalar(255, 0, 0), 1, 4, 0);
			}
			if (black)
				printf("블랙 ");
			else
				printf("레드 ");
			printf("조커\n");

		}
		else {
			vector<Rect> ShapeBox = detectShape(image, false);
			for (int i = 0; i < ShapeBox.size(); i++)
				rectangle(image, ShapeBox[i], Scalar(255, 0, 0), 1, 4, 0);
		}
		imshow("image", image);

		waitKey();
	}

	_findclose(handle);

	return 0;
}





