/*
#include<opencv2/opencv.hpp>

// 픽셀에 color 할당
void assignColor(IplImage *src, IplImage *dst, int u, int v, int channel) {
	for (int y = 0; y < dst->height; y++) 
		for (int x = 0; x < dst->width; x++) {
			int x2 = x + u;
			int y2 = y + v;
			if (x2 < 0 || x2 > dst->width - 1) continue;
			if (y2 < 0 || y2 > dst->height - 1) continue;
			CvScalar f = cvGet2D(dst, y, x);
			CvScalar g = cvGet2D(src, y2, x2);
			f.val[channel] = g.val[0];
			cvSet2D(dst, y2, x2, f);
		}
}

// 이미지 축소
IplImage *resizeImage(IplImage *img, float scaleFactor) {
	CvSize size;
	// scale factor 로 이미지 크기 축소
	size.width = cvRound(img->width * scaleFactor);
	size.height = cvRound(img->height * scaleFactor);

	IplImage *resizedImg = cvCreateImage(size, 8, 3);
	cvResize(img, resizedImg, CV_INTER_NN);
	return resizedImg;
}

// ssd 계산
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
			out += ((f1.val[0] - f2.val[0]) * (f1.val[0] - f2.val[0])
				//+ 
				//	(f1.val[1] - f2.val[1]) * (f1.val[1] - f2.val[1]) +
				//	(f1.val[2] - f2.val[2]) * (f1.val[2] - f2.val[2])
				);
			count++;
		}
	out /= count;
	return out;
}

// 최적 이동량 계산
void shift(IplImage *source, IplImage *templateImage, int* bestU, int* bestV, int range) {
	float minSSD = FLT_MAX;
	// 최적이동량
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
	printf("CV test\nInput File Name: ");
	char path[100];

	// 경로 입력
	scanf("%s", path);

	// 경로 상의 이미지 로드
	IplImage* source = cvLoadImage(path);

	// 목표 이미지 크기 설정
	CvSize size;
	size.height = source->height / 3;
	size.width = source->width;

	// B, G, R 순으로 배열에 이미지 저장하기 위한 IplImage 배열 선언
	IplImage *src[3] = {	cvCreateImage(size, 8, 3),
							cvCreateImage(size, 8, 3),
							cvCreateImage(size, 8, 3) 
	};

	// 각 인덱스별로 이미지 저장
	for (int i = 0; i < 3; i++) 
		for (int y = 0; y < src[i]->height; y++) 
			for (int x = 0; x < src[i]->width; x++) {
				CvScalar f = cvScalarAll(0);
				CvScalar g = cvGet2D(source, y + src[i]->height * i, x);
				f.val[0] = g.val[0];
				cvSet2D(src[i], y, x, f);
			}
	
	// coarse-to-fine
	IplImage **cur = src;

	int bestU[3] = { 0,0,0 }, bestV[3] = { 0,0,0 };
	float scale[3] = { 0.25f, 0.5f, 1.0f }; // scale
	float range[3] = { 50, 20, 10 };

	IplImage *dst = cvCreateImage(size, 8, 3);


	//for (int level = 0; level < 3; level++) {
	//	CvSize rsize;
	//	rsize.width = cvRound(size.width * scale[level]);
	//	rsize.height = cvRound(size.height * scale[level]);
	//	IplImage *resizedsrc[3];
	//	// image resize
	//	for (int i = 0; i < 3; i++) {
	//		resizedsrc[i] = cvCreateImage(rsize, 8, 3);
	//		resizedsrc[i] = resizeImage(cur[i], scale[level]);
	//	}

	//	for (int i = 1; i < 3; i++) {
	//		int localBestU = 0, localBestV = 0;
	//		shift(resizedsrc[0], resizedsrc[i], &localBestU, &localBestV, range[level]);

	//		bestU[i] = localBestU;
	//		bestV[i] = localBestV;

	//		printf("Scale %.1f Best shift (%d, %d)\n", scale[level], bestU[i], bestV[i]);
	//	
	//	}

	//}
	for (int level = 0; level < 3; level++) {
		CvSize rsize;
		rsize.width = cvRound(size.width * scale[level]);
		rsize.height = cvRound(size.height * scale[level]);

		IplImage *resizedsrc[3];
		// (1) 이미지 축소
		for (int i = 0; i < 3; i++) {
			resizedsrc[i] = resizeImage(cur[i], scale[level]);
		}

		// (2) i=1,2만 기준(i=0)과 비교
		for (int i = 1; i < 3; i++) {
			int localBestU = 0, localBestV = 0;

			// shift(기준=resizedsrc[0], 대상=resizedsrc[i], &localBestU, &localBestV, ...)
			shift(resizedsrc[0], resizedsrc[i], &localBestU, &localBestV, range[level]);

			// "이전 레벨"에서 누적된 bestU[i], bestV[i]를
			// 이번 레벨 scale에 맞춰 스케일 업(2배 or scale[level]/scale[level-1] 등).
			if (level > 0) {
				float ratio = scale[level] / scale[level - 1];
				bestU[i] = (int)(bestU[i] * ratio);
				bestV[i] = (int)(bestV[i] * ratio);
			}

			// 그리고, 이번 레벨에서 새로 찾은 localBestU/ localBestV를
			// 원본 좌표로 환산해 누적
			bestU[i] += (int)(localBestU / scale[level]);
			bestV[i] += (int)(localBestV / scale[level]);

			printf("[level=%.2f] i=%d => local=(%d,%d), total=(%d,%d)\n",
				scale[level], i, localBestU, localBestV, bestU[i], bestV[i]);
		}

		// (3) resizedsrc[i] 메모리 해제
		for (int i = 0; i < 3; i++) {
			cvReleaseImage(&resizedsrc[i]);
		}
	}


	assignColor(src[0], dst, 0 , 0, 0);
	assignColor(src[1], dst, bestU[1], bestV[1], 1);
	assignColor(src[2], dst, bestU[2], bestV[2], 2);

	cvShowImage("dst", dst);
	cvWaitKey();

	cvReleaseImage(&source);

	return 0;
}
	*/
