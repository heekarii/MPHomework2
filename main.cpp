#include <opencv2/opencv.hpp>
#include <cstdio>
#include <climits>

/// 1ä�� IplImage �� ���� SSD�� ����ϴ� �Լ�
/// s1: ���� �̹���
/// s2: ������ �̹���
/// (u, v): s2�� (u, v)��ŭ ���� �̵��� �� s1�� ��
double getSSD_1ch(IplImage *s1, IplImage *s2, int u, int v) {
    double ssd = 0.0;
    int count = 0;

    // s1, s2 ��� 1ä���̶�� ����
    // s1->width, s1->height ��ŭ �ݺ�
    for (int y = 0; y < s1->height; y++) {
        int y2 = y + v;
        // y2�� s2 ���� ���̸� �ǳʶ�
        if (y2 < 0 || y2 >= s2->height)
            continue;

        // ������ ����
        // s1�� y�� ���� ������
        uchar *rowPtr1 = (uchar *)(s1->imageData + y * s1->widthStep);
        // s2�� y2�� ���� ������
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
    else ssd = 1e15; // �� ������ ���� ������ ū �� ��ȯ

    return ssd;
}

/// (u, v) = (���� �̵�, ���� �̵�)�� ����
/// src �̹����� dst�� Ư�� ä��(channelIndex)�� �����ϴ� �Լ�
/// ���⼭�� dst�� 3ä��, src�� 1ä�η� ����
void copyChannelWithShift(IplImage *src, IplImage *dst, int u, int v, int channelIndex) {
    // dst�� BGR 3ä��, src�� 1ä���̶�� ����
    for (int y = 0; y < src->height; y++) {
        // src ������
        uchar *rowPtrSrc = (uchar *)(src->imageData + y * src->widthStep);

        int Y = y + v; // �̵� �� ��ǥ
        if (Y < 0 || Y >= dst->height)
            continue;

        // dst ������ (�� ä�θ� ���� �����ؾ� �ϹǷ� row�� ������ ����)
        uchar *rowPtrDst = (uchar *)(dst->imageData + Y * dst->widthStep);

        for (int x = 0; x < src->width; x++) {
            int X = x + u;
            if (X < 0 || X >= dst->width)
                continue;

            // src�� 1ä��
            uchar val = rowPtrSrc[x];
            // dst�� 3ä��(BGR), channelIndex���� ����
            rowPtrDst[3 * X + channelIndex] = val;
        }
    }
}

int main() {
    // (1) ���η� �� ��� �̹����� �ҷ���
    //     ��) ���̰� 3��, ���� ����. depth=8, 1ä���̶�� ����
    char path[256];
    std::scanf("%s", path);

    IplImage *src = cvLoadImage(path, CV_LOAD_IMAGE_GRAYSCALE);
    if (!src) {
        printf("Fail to load image\n");
        return -1;
    }

    // ���� = 3���
    int singleHeight = src->height / 3;
    CvSize singleSize = cvSize(src->width, singleHeight);

    // (2) top, middle, bottom 1ä�� IplImage�� �и�
    IplImage *top = cvCreateImage(singleSize, 8, 1);
    IplImage *middle = cvCreateImage(singleSize, 8, 1);
    IplImage *bottom = cvCreateImage(singleSize, 8, 1);

    // ����
    // top
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrTop = (uchar *)(top->imageData + y * top->widthStep);
        uchar *rowPtrSrc = (uchar *)(src->imageData + y * src->widthStep);
        memcpy(rowPtrTop, rowPtrSrc, singleSize.width);
        // 1ä���̹Ƿ� width���� ����Ʈ ����
    }
    // middle
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrMid = (uchar *)(middle->imageData + y * middle->widthStep);
        // src���� y + singleHeight
        uchar *rowPtrSrc = (uchar *)(src->imageData + (y + singleHeight) * src->widthStep);
        memcpy(rowPtrMid, rowPtrSrc, singleSize.width);
    }
    // bottom
    for (int y = 0; y < singleHeight; y++) {
        uchar *rowPtrBot = (uchar *)(bottom->imageData + y * bottom->widthStep);
        // src���� y + 2 * singleHeight
        uchar *rowPtrSrc = (uchar *)(src->imageData + (y + 2 * singleHeight) * src->widthStep);
        memcpy(rowPtrBot, rowPtrSrc, singleSize.width);
    }

    // (3) top�� �������� middle, bottom ���� ���Ʈ���� Ž��
    //     ���⼭�� -30~30 ������ ���÷�...
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

    // (4) ���� 3ä��(BGR) �̹��� ����, ũ��� singleSize
    IplImage *dst = cvCreateImage(singleSize, 8, 3);

    // �ʱ�ȭ (����� ����������)
    cvZero(dst);

    // (5) top -> B ä�� (u=0, v=0)
    copyChannelWithShift(top, dst, 0, 0, 0);

    // (6) middle -> G ä�� (bestUMid, bestVMid)
    copyChannelWithShift(middle, dst, bestUMid, bestVMid, 1);

    // (7) bottom -> R ä�� (bestUBot, bestVBot)
    copyChannelWithShift(bottom, dst, bestUBot, bestVBot, 2);

    // (8) ȭ�� ǥ�� / �̹��� ����
    cvShowImage("dst", dst);
    cvWaitKey(0);

    cvSaveImage("aligned_result.png", dst);

    // �޸� ����
    cvReleaseImage(&src);
    cvReleaseImage(&top);
    cvReleaseImage(&middle);
    cvReleaseImage(&bottom);
    cvReleaseImage(&dst);

    return 0;
}
