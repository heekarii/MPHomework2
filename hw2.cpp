#include<opencv2/opencv.hpp>

void assignColor(IplImage* src, IplImage* dst) {
	for (int i = 0; i < 3; i++) {
		for (int y = 0; y < dst->height; y++) {
			for (int x = 0; x < dst->width; x++) {
				CvScalar f = cvGet2D(dst, y, x);
				CvScalar g = cvGet2D(src, y + dst->height * i, x);

				f.val[i] = g.val[0];

				cvSet2D(dst, y, x, f);

			}
		}
	}
}

int main() {
	char path[100];
	scanf("%s", path);

	IplImage* src = cvLoadImage(path);
	CvSize src_size = cvGetSize(src);
	src_size.height /= 3;
	IplImage* dst = cvCreateImage(src_size, 8, 3);
	assignColor(src, dst);
	cvShowImage("dst", dst);
	cvWaitKey();

	return 0;
}