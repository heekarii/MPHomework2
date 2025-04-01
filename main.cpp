#include <opencv2/opencv.hpp>
#include <cstdio>
#include <climits>

/// 1채널 IplImage 두 장의 SSD를 계산하는 함수
/// s1: 기준 이미지
/// s2: 정렬할 이미지
/// (u, v): s2를 (u, v)만큼 평행 이동한 뒤 s1과 비교
double getSSD_1ch(IplImage *s1, IplImage *s2, int u, int v) {
    double ssd = 0.0;
    int count = 0;

    // s1, s2 모두 1채널이라고 가정
    // s1->width, s1->height 만큼 반복
    for (int y = 0; y < s1->height; y++) {
        int y2 = y + v;
        // y2가 s2 범위 밖이면 건너뜀
        if (y2 < 0 || y2 >= s2->height)
            continue;

        // 포인터 접근
        // s1의 y행 시작 포인터
        uchar *rowPtr1 = (uchar *)(s1->imageData + y * s1->widthStep);
        // s2의 y2행 시작 포인터
        uchar *rowPtr2 = (uchar *)(s2->imageData + y2 * s2->widthStep);

        for (int x = 0; x < s1->width; x++) {
            int x2 = x + u;
            if (x2 < 0 || x2 >= s2->width)
                continue;

            int diff = rowPtr1[x] - rowPtr2[x2];
            ssd += (diff * diff);
            count++;
        }
    }

    if (count > 0) ssd /= count;
    else ssd = 1e15; // 비교 영역이 전혀 없으면 큰 값 반환

    return ssd;
}

/// (u, v) = (가로 이동, 세로 이동)에 맞춰
/// src 이미지를 dst의 특정 채널(channelIndex)에 복사하는 함수
/// 여기서는 dst는 3채널, src는 1채널로 가정
void copyChannelWithShift(IplImage *src, IplImage *dst, int u, int v, int channelIndex) {
    // dst는 BGR 3채널, src는 1채널이라고 가정
    for (int y = 0; y < src->height; y++) {
        // src 포인터
        uchar *rowPtrSrc = (uchar *)(src->imageData + y * src->widthStep);

        int Y = y + v; // 이동 후 좌표
        if (Y < 0 || Y >= dst->height)
            continue;

        // dst 포인터 (이 채널만 따로 접근해야 하므로 row별 포인터 연산)
        uchar *rowPtrDst = (uchar *)(dst->imageData + Y * dst->widthStep);

        for (int x = 0; x < src->width; x++) {
            int X = x + u;
            if (X < 0 || X >= dst->width)
                continue;

            // src는 1채널
            uchar val = rowPtrSrc[x];
            // dst는 3채널(BGR), channelIndex에만 쓰기
            rowPtrDst[3 * X + channelIndex] = val;
        }
    }
}

int main() {
    // (1) 세로로 긴 흑백 이미지를 불러옴
    //     예) 높이가 3배, 폭은 동일. depth=8, 1채널이라고 가정
    char path[256];
    std::scanf("%s", path);

    IplImage *src = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
    if (!src) {
        printf("Fail to load image\n");
        return -1;
    }

    // 높이 = 3등분
    int singleHeight = src->height / 3;
    CvSize singleSize = cvSize(src->width, singleHeight);

    // (2) top, middle, bottom 1채널 IplImage로 분리
    IplImage *top = cvCreateImage(singleSize, 8, 1);
    IplImage *middle = cvCreateImage(singleSize, 8, 1);
    IplImage *bottom = cvCreateImage(singleSize, 8, 1);

    // 복사
    // top
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrTop = (uchar *)(top->imageData + y * top->widthStep);
        uchar *rowPtrSrc = (uchar *)(src->imageData + y * src->widthStep);
        memcpy(rowPtrTop, rowPtrSrc, singleSize.width);
        // 1채널이므로 width개의 바이트 복사
    }
    // middle
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrMid = (uchar *)(middle->imageData + y * middle->widthStep);
        // src에서 y + singleHeight
        uchar *rowPtrSrc = (uchar *)(src->imageData + (y + singleHeight) * src->widthStep);
        memcpy(rowPtrMid, rowPtrSrc, singleSize.width);
    }
    // bottom
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrBot = (uchar *)(bottom->imageData + y * bottom->widthStep);
        // src에서 y + 2 * singleHeight
        uchar *rowPtrSrc = (uchar *)(src->imageData + (y + 2 * singleHeight) * src->widthStep);
        memcpy(rowPtrBot, rowPtrSrc, singleSize.width);
    }

    // (3) top을 기준으로 middle, bottom 각각 브루트포스 탐색
    //     여기서는 -30~30 범위를 예시로...
    int bestUMid = 0, bestVMid = 0;
    double minSSD_mid = 1e15;

    for (int v = -30; v <= 30; v++) {
        for (int u = -30; u <= 30; u++) {
            double ssd = getSSD_1ch(top, middle, u, v);
            if (ssd < minSSD_mid) {
                minSSD_mid = ssd;
                bestUMid = u;
                bestVMid = v;
            }
        }
    }

    int bestUBot = 0, bestVBot = 0;
    double minSSD_bot = 1e15;

    for (int v = -30; v <= 30; v++) {
        for (int u = -30; u <= 30; u++) {
            double ssd = getSSD_1ch(top, bottom, u, v);
            if (ssd < minSSD_bot) {
                minSSD_bot = ssd;
                bestUBot = u;
                bestVBot = v;
            }
        }
    }

    printf("bestUMid = %d, bestVMid = %d, minSSD_mid = %f\n", bestUMid, bestVMid, minSSD_mid);
    printf("bestUBot = %d, bestVBot = %d, minSSD_bot = %f\n", bestUBot, bestVBot, minSSD_bot);

    // (4) 최종 3채널(BGR) 이미지 생성, 크기는 singleSize
    IplImage *dst = cvCreateImage(singleSize, 8, 3);

    // 초기화 (배경을 검은색으로)
    cvZero(dst);

    // (5) top -> B 채널 (u=0, v=0)
    copyChannelWithShift(top, dst, 0, 0, 0);

    // (6) middle -> G 채널 (bestUMid, bestVMid)
    copyChannelWithShift(middle, dst, bestUMid, bestVMid, 1);

    // (7) bottom -> R 채널 (bestUBot, bestVBot)
    copyChannelWithShift(bottom, dst, bestUBot, bestVBot, 2);

    // (8) 화면 표시 / 이미지 저장
    cvShowImage("dst", dst);
    cvWaitKey(0);

    cvSaveImage("aligned_result.png", dst);

    // 메모리 해제
    cvReleaseImage(&src);
    cvReleaseImage(&top);
    cvReleaseImage(&middle);
    cvReleaseImage(&bottom);
    cvReleaseImage(&dst);

    return 0;
}
