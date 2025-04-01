//#include<opencv2/opencv.hpp>
//
//void assignColor(IplImage *src, IplImage *dst, int u, int v, int channel) {
//	for (int y = 0; y < dst->height; y++) 
//		for (int x = 0; x < dst->width; x++) {
//			int x2 = x + u;
//			int y2 = y + v;
//			if (x2 < 0 || x2 > dst->width - 1) continue;
//			if (y2 < 0 || y2 > dst->height - 1) continue;
//			CvScalar f = cvGet2D(dst, y, x);
//			CvScalar g = cvGet2D(src, y2, x2);
//			f.val[channel] = g.val[channel];
//			cvSet2D(dst, y2, x2, f);
//		}
//}
//
//double getSSD(IplImage *s1, IplImage *s2, int u, int v, int channel1, int channel2) {
//	double out = 0;
//	int count = 0;
//	for (int y = 0; y < s1->height; y++)
//		for (int x = 0; x < s1->width; x++) {
//			int x2 = x + u;
//			int y2 = y + v;
//			if (x2<0 || x2>s2->width - 1) continue;
//			if (y2<0 || y2>s2->height - 1) continue;
//
//			CvScalar f1 = cvGet2D(s1, y, x);
//			CvScalar f2 = cvGet2D(s2, y2, x2);
//			out += (f1.val[channel1] - f2.val[channel2]) * (f1.val[channel1] - f2.val[channel2]);
//			count++;
//		}
//	out /= count;
//	return out;
//}
//
//int main() {
//	printf("CV test\n");
//	char path[100];
//	// 경로 입력
//	scanf("%s", path);
//	// 경로 상의 이미지 로드
//	IplImage* source = cvLoadImage(path);
//	// 목표 이미지 크기 설정
//	CvSize size;
//	size.height = source->height / 3;
//	size.width = source->width;
//	// B, G, R 순으로 배열에 이미지 저장하기 위한 IplImage 배열 선언
//	IplImage *src[3] = {	cvCreateImage(size, 8, 3),
//							cvCreateImage(size, 8, 3),
//							cvCreateImage(size, 8, 3) 
//	};
//	// 각 인덱스별로 이미지 저장
//	for (int i = 0; i < 3; i++) 
//		for (int y = 0; y < src[i]->height; y++) 
//			for (int x = 0; x < src[i]->width; x++) {
//				CvScalar f = cvGet2D(src[i], y, x);
//				CvScalar g = cvGet2D(source, y + src[i]->height * i, x);
//				f.val[i] = g.val[0];
//				cvSet2D(src[i], y, x, f);
//			}
//
//	// 최종 이미지 선언
//	IplImage* dst = cvCreateImage(size, 8, 3);
//	
//	assignColor(src[0], dst, 0, 0, 0);
//
//	float min_diff1 = FLT_MAX;
//	float min_diff2 = FLT_MAX;
//
//	int bestU1 = 0, bestV1 = 0;
//	int bestU2 = 0, bestV2 = 0;
//
//	for (int v = -5; v < 5; v++) {
//		for (int u = -5; u < 5; u++) {
//			// diff 값 갱신
//			double diff1 = getSSD(src[0], src[1], u, v, 0, 1);
//			double diff2 = getSSD(src[0], src[2], u, v, 0, 2);
//
//			if (min_diff1 > diff1) {
//				min_diff1 = diff1;
//				bestU1 = u;
//				bestV1 = v;
//			}
//			if (min_diff2 > diff2) {
//				min_diff2 = diff2;
//
//				bestU2 = u;
//				bestV2 = v;
//
//			}
//
//			printf("u=%d v=%d min_diff1=%f min_diff2=%f diff1=%f diff2=%f\n", u, v, min_diff1, min_diff2, diff1, diff2);
//		}
//	}
//	
//	assignColor(src[1], dst, bestU1, bestV1, 1);
//	assignColor(src[2], dst, bestU2, bestV2, 2);
//	cvShowImage("dst", dst);
//	cvWaitKey();
//
//	return 0;
//}



#include<opencv2/opencv.hpp>

typedef struct _MINERR {
    int u; //최적이동량 (u가 x좌표, v가 y좌표)
    int v;
    float min_err; //최소 오차
} MINERR;

IplImage *ImageResize(IplImage *);
void ImageAlignment(IplImage *, IplImage *, MINERR *, int, int, int);

int main() {
    printf("Test CV\n");
    IplImage *src = NULL;
    // 이미지 파일을 불러올 때까지 반복하여 입력 받는 작업
    while (1) {
        char filename[100];
        printf("Input File Name: ");
        scanf("%s", &filename);
        src = cvLoadImage((const char *)filename);
        if (src != NULL)
            break;
    }

    CvSize size = cvGetSize(src);
    int w = size.width;
    int h = size.height / 3;

    IplImage *colorImg[3]; //RGB 추출 이미지
    IplImage *smallColorImg[3]; //축소될 RGB 추출 이미지
    IplImage *dst = cvCreateImage(cvSize(w, h), 8, 3); //최종이미지

    // RGB 추출 이미지 생성
    for (int i = 0; i < 3; i++) {
        colorImg[i] = cvCreateImage(cvSize(w, h), 8, 3);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                CvScalar f = cvGet2D(src, y + (h * i), x);
                cvSet2D(colorImg[i], y, x, f);
            }
    }

    // RGB 이미지 축소
    for (int i = 0; i < 3; i++)
        smallColorImg[i] = ImageResize(colorImg[i]);

    MINERR gErr = { 0, 0, FLT_MAX }, rErr = { 0, 0, FLT_MAX }; // Green과 Red의 최적 이동량 및 최소 오차 초기화

    // Blue 이미지를 기준으로 Green과 Red 채널의 이미지 정렬
    ImageAlignment(smallColorImg[0], smallColorImg[1], &gErr, w / 3 / 20, h / 3 / 20, 5);
    ImageAlignment(smallColorImg[0], smallColorImg[1], &gErr, 5, 5, 1);
    gErr.u *= 3; // 3배 축소된 이미지의 좌표값과 에러값을 원본 이미지 크기로 변환
    gErr.v *= 3;
    gErr.min_err *= 3;
    ImageAlignment(colorImg[0], colorImg[1], &gErr, 3, 3, 1);

    ImageAlignment(smallColorImg[0], smallColorImg[2], &rErr, w / 3 / 20, h / 3 / 20, 5);
    ImageAlignment(smallColorImg[0], smallColorImg[2], &rErr, 5, 5, 1);
    rErr.u *= 3; // 3배 축소된 이미지의 좌표값과 에러값을 원본 이미지 크기로 변환
    rErr.v *= 3;
    rErr.min_err *= 3;
    ImageAlignment(colorImg[0], colorImg[2], &rErr, 3, 3, 1);;

    // 최종 이미지 생성
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int x1 = x; // blue 이미지 좌표
            int y1 = y;
            int x2 = x + gErr.u; // green 이미지 좌표
            int y2 = y + gErr.v;
            int x3 = x + rErr.u; // red 이미지 좌표
            int y3 = y + rErr.v;

            CvScalar f1 = cvGet2D(colorImg[0], y1, x1); //blueImg 픽셀값
            f1.val[1] = 0; // Green 채널을 0으로 초기화
            if (x2 >= 0 && x2 < w && y2 >= 0 && y2 < h) {
                CvScalar f2 = cvGet2D(colorImg[1], y2, x2); // Green 이미지의 픽셀 값이 존재하는 경우 해당 값을 가져와 Red 채널에 대입
                f1.val[1] = f2.val[1];
            }
            f1.val[2] = 0; // red 채널을 0으로 초기화
            if (x3 >= 0 && x3 < w && y3 >= 0 && y3 < h) {
                CvScalar f3 = cvGet2D(colorImg[2], y3, x3); // Red 이미지의 픽셀 값이 존재하는 경우 해당 값을 가져와 Red 채널에 대입
                f1.val[2] = f3.val[2];
            }
            // 최종 이미지에 설정된 픽셀 값을 설정
            cvSet2D(dst, y, x, f1);
        }

    //최종 이미지 표시
    cvShowImage("dst", dst);
    cvWaitKey();

    // 메모리 해제
    cvReleaseImage(&src);
    cvReleaseImage(&dst);
    for (int i = 0; i < 3; i++) {
        cvReleaseImage(&colorImg[i]);
        cvReleaseImage(&smallColorImg[i]);
    }

    return 0;
}

IplImage *ImageResize(IplImage *src) {
    CvSize size = cvGetSize(src);
    int w = size.width / 3; //축소된 이미지의 사이즈 계산
    int h = size.height / 3;
    IplImage *dst = cvCreateImage(cvSize(w, h), 8, 3); //축소된 이미지 생성

    int K = 1;
    float n = (2 * K + 1) * (2 * K + 1);
    float H[3][3] = { {1 / 16.f, 2 / 16.f, 1 / 16.f}, // 가우시안 필터 행렬
               {2 / 16.f, 4 / 16.f, 2 / 16.f},
               {1 / 16.f, 2 / 16.f, 1 / 16.f} };

    //이미지 사이즈 줄이기 (1/3으로 줄이기) -> 속도 향상을 위함.
    //이미지 사이즈를 줄이면서 해상도도 낮추기 위해 가우시안 필터 적용 
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int x1 = x * 3; //원본 이미지의 픽셀값
            int y1 = y * 3;

            //이미지를 축소하면서 가우시안 필터 적용
            CvScalar g = cvScalar(0, 0, 0);
            for (int v = -K; v <= K; v++)
                for (int u = -K; u <= K; u++) {
                    if (x1 + u >= 0 && x1 + u < size.width && y1 + v >= 0 && y1 + v < size.height) // 좌표값들이 원본 이미지 내부에 있는지 확인하고, 가우시안 필터를 적용
                    {
                        CvScalar f = cvGet2D(src, y1 + v, x1 + u);
                        for (int k = 0; k < 3; k++)
                            g.val[k] += H[v + K][u + K] * f.val[k]; // 가우시안 필터를 적용하여 새로운 픽셀 값을 계산
                    }
                }
            cvSet2D(dst, y, x, g); // 새로 계산된 픽셀 값을 축소된 이미지에 입힘
        }

    return dst;
}

void ImageAlignment(IplImage *img1, IplImage *img2, MINERR *err, int du, int dv, int dt) // du, dv, dt -> 이동량을 결정하는 매개변수
{
    if (du <= 0 || dv <= 0 || dt <= 0) //이동량이 유효한지 확인 음수이거나 0일 경우 리턴
        return;

    CvSize size = cvGetSize(img1);
    int w = size.width;
    int h = size.height;

    int min_u = err->u; //현재최적이동량
    int min_v = err->v;
    for (int v = min_v - dv; v <= min_v + dv; v += dt) {
        for (int u = min_u - du; u <= min_u + du; u += dt) {
            float e = 0.0f; //오차 저장 변수
            int ct = 0; //픽셀 개수 저장 변수
            for (int y = h / 10; y < h - (h / 10); y += 3) // 이미지의 일부 영역을 탐색하여 오차를 계산 -> 필터 프레임을 제외한 사진 부분만 탐색하기 위해 영역 제한. 단 10보다 숫자가 작아질 경우 정확도가 낮아지는 것을 확인하여 10으로 선정.
            {
                for (int x = w / 10; x < w - (w / 10); x += 3) {
                    int x1 = x; //img1에서의 좌표
                    int y1 = y;
                    int x2 = x + u; //img2에서의 좌표
                    int y2 = y + v;
                    if (x2 < 0 || x2 > w - 1) continue; //이미지 범위를 벗어나는 경우 처리X
                    if (y2 < 0 || y2 > h - 1) continue;
                    CvScalar f1 = cvGet2D(img1, y1, x1); //각 이미지의 RGB값 추출
                    CvScalar f2 = cvGet2D(img2, y2, x2);
                    e += (f1.val[0] - f2.val[0]) //픽셀값의 차이를 계산하여 제곱 -> 오차
                        * (f1.val[0] - f2.val[0]);
                    ct++; //픽셀 개수 +1
                }
            }
            e /= ct;
            if (e < err->min_err) //최소 오차 업데이트
            {
                err->u = u; //최적이동량 갱신
                err->v = v;
                err->min_err = e;
            }
        }
    }
}