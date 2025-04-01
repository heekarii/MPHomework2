#include<opencv2/opencv.hpp>

void assignColor(IplImage* src, IplImage* dst) {
	CvSize size;
	size.height = src->height / 3;
	size.width = src->width;

	IplImage *src1[3] = { cvCreateImage(size, 8, 3) ,cvCreateImage(size, 8, 3) ,cvCreateImage(size, 8, 3)};

	for (int i = 0; i < 3; i++) 
		for (int y = 0; y < src1[i]->height; y++) 
			for (int x = 0; x < src1[i]->width; x++) {
				CvScalar f = cvGet2D(src1[i], y, x);
				CvScalar g = cvGet2D(src, y + dst->height * i, x);
				f.val[i] = g.val[0];
				cvSet2D(src1[i], y, x, f);
			}
	for (int y = 0; y < dst->height; y++) 
		for (int x = 0; x < dst->width; x++) {
			CvScalar f = cvGet2D(dst, y, x);
			CvScalar g1 = cvGet2D(src1[0], y, x);
			CvScalar g2 = cvGet2D(src1[1], y, x);
			CvScalar g3 = cvGet2D(src1[2], y, x);
			f.val[0] = g1.val[0];
			f.val[1] = g2.val[1];
			f.val[2] = g3.val[2];
			cvSet2D(dst, y, x, f);
		}

}

void alignImage(IplImage *src1, IplImage *src2, IplImage *src3) {

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