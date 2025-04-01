#include<opencv2/opencv.hpp>

// �ȼ��� color �Ҵ�
void assignColor(IplImage *src, IplImage *dst, int u, int v, int channel) {
	for (int y = 0; y < dst->height; y++) 
		for (int x = 0; x < dst->width; x++) {
			int x2 = x + u;
			int y2 = y + v;
			if (x2 < 0 || x2 > dst->width - 1) continue;
			if (y2 < 0 || y2 > dst->height - 1) continue;
			CvScalar f = cvGet2D(dst, y, x);
			CvScalar g = cvGet2D(src, y2, x2);
			f.val[channel] = g.val[channel];
			cvSet2D(dst, y2, x2, f);
		}
}

// �̹��� ���
IplImage *resizeImage(IplImage *img, float scaleFactor) {
	CvSize size;
	// scale factor �� �̹��� ũ�� ���
	size.width = cvRound(img->width * scaleFactor);
	size.height = cvRound(img->height * scaleFactor);

	IplImage *resizedImg = cvCreateImage(size, 8, 3);
	cvResize(img, resizedImg);
	return resizedImg;
}

// ssd ���
double getSSD(IplImage *s1, IplImage *s2, int u, int v) {
	double out = 0;
	int count = 0;
	for (int y = 0; y < s1->height; y++)
		for (int x = 0; x < s1->width; x++) {
			int x2 = x + u;
			int y2 = y + v;
			if (x2<0 || x2>s2->width - 1) continue;
			if (y2<0 || y2>s2->height - 1) continue;

			CvScalar f1 = cvGet2D(s1, y, x);
			CvScalar f2 = cvGet2D(s2, y2, x2);
			out += ((f1.val[0] - f2.val[0]) * (f1.val[0] - f2.val[0]) + 
					(f1.val[1] - f2.val[1]) * (f1.val[1] - f2.val[1]) +
					(f1.val[2] - f2.val[2]) * (f1.val[2] - f2.val[2])
				);
			count++;
		}
	out /= count;
	return out;
}

// ���� �̵��� ���
void Shift(IplImage *source, IplImage *templateImage, int* bestU, int* bestV, int range) {
	float minSSD = FLT_MAX;
	// �����̵���
	*bestU = 0;
	*bestV = 0;

	for (int v = -range; v <= range; v++) {
		for (int u = -range; u <= range; u++) {
			double diff = getSSD(source, templateImage, u, v);
			if (diff < minSSD) {
				minSSD = diff;
				*bestU = u;
				*bestV = v;
			}
		}
	}

}

int main() {
	printf("CV test\n");
	char path[100];

	// ��� �Է�
	scanf("%s", path);

	// ��� ���� �̹��� �ε�
	IplImage* source = cvLoadImage(path);

	// ��ǥ �̹��� ũ�� ����
	CvSize size;
	size.height = source->height / 3;
	size.width = source->width;

	// B, G, R ������ �迭�� �̹��� �����ϱ� ���� IplImage �迭 ����
	IplImage *src[3] = {	cvCreateImage(size, 8, 3),
							cvCreateImage(size, 8, 3),
							cvCreateImage(size, 8, 3) 
	};

	// �� �ε������� �̹��� ����
	for (int i = 0; i < 3; i++) 
		for (int y = 0; y < src[i]->height; y++) 
			for (int x = 0; x < src[i]->width; x++) {
				CvScalar f = cvGet2D(src[i], y, x);
				CvScalar g = cvGet2D(source, y + src[i]->height * i, x);
				f.val[i] = g.val[0];
				cvSet2D(src[i], y, x, f);
			}

	// ���� �̹��� ����
	IplImage* dst = cvCreateImage(size, 8, 3);
	
	// coarse-to-fine
	IplImage **cur = src;

	int bestU = 0, bestV = 0;
	int range = 10; // shift range

	for (int i = 0; i < 3; i++) {
		// downsampling
		for (float scale = 1.0; scale >= 0.1; scale -= 0.3) {
			IplImage *resizedImg = resizeImage(cur[i], scale);

			int localBestU = 0, localBestV = 0;
			Shift(resizedImg, resizedImg, &localBestU, &localBestV, range);

			bestU = localBestU;
			bestV = localBestV;

			printf("Scale %.1f Best shift (%d %d)\n", scale, bestU, bestV);

			if (scale == 0.1) {
				IplImage *dst = cvCreateImage(cvGetSize(source), 8, 3);
				assignColor(resizedImg, dst, bestU, bestV, i);
				cvShowImage("dst", dst);
				cvWaitKey(1);
				cvReleaseImage(&dst);
			}

			cur[i] = resizedImg;
		}
	}

	cvReleaseImage(&source);

	return 0;
}
