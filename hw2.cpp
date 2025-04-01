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
//	// ��� �Է�
//	scanf("%s", path);
//	// ��� ���� �̹��� �ε�
//	IplImage* source = cvLoadImage(path);
//	// ��ǥ �̹��� ũ�� ����
//	CvSize size;
//	size.height = source->height / 3;
//	size.width = source->width;
//	// B, G, R ������ �迭�� �̹��� �����ϱ� ���� IplImage �迭 ����
//	IplImage *src[3] = {	cvCreateImage(size, 8, 3),
//							cvCreateImage(size, 8, 3),
//							cvCreateImage(size, 8, 3) 
//	};
//	// �� �ε������� �̹��� ����
//	for (int i = 0; i < 3; i++) 
//		for (int y = 0; y < src[i]->height; y++) 
//			for (int x = 0; x < src[i]->width; x++) {
//				CvScalar f = cvGet2D(src[i], y, x);
//				CvScalar g = cvGet2D(source, y + src[i]->height * i, x);
//				f.val[i] = g.val[0];
//				cvSet2D(src[i], y, x, f);
//			}
//
//	// ���� �̹��� ����
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
//			// diff �� ����
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
    int u; //�����̵��� (u�� x��ǥ, v�� y��ǥ)
    int v;
    float min_err; //�ּ� ����
} MINERR;

IplImage *ImageResize(IplImage *);
void ImageAlignment(IplImage *, IplImage *, MINERR *, int, int, int);

int main() {
    printf("Test CV\n");
    IplImage *src = NULL;
    // �̹��� ������ �ҷ��� ������ �ݺ��Ͽ� �Է� �޴� �۾�
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

    IplImage *colorImg[3]; //RGB ���� �̹���
    IplImage *smallColorImg[3]; //��ҵ� RGB ���� �̹���
    IplImage *dst = cvCreateImage(cvSize(w, h), 8, 3); //�����̹���

    // RGB ���� �̹��� ����
    for (int i = 0; i < 3; i++) {
        colorImg[i] = cvCreateImage(cvSize(w, h), 8, 3);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) {
                CvScalar f = cvGet2D(src, y + (h * i), x);
                cvSet2D(colorImg[i], y, x, f);
            }
    }

    // RGB �̹��� ���
    for (int i = 0; i < 3; i++)
        smallColorImg[i] = ImageResize(colorImg[i]);

    MINERR gErr = { 0, 0, FLT_MAX }, rErr = { 0, 0, FLT_MAX }; // Green�� Red�� ���� �̵��� �� �ּ� ���� �ʱ�ȭ

    // Blue �̹����� �������� Green�� Red ä���� �̹��� ����
    ImageAlignment(smallColorImg[0], smallColorImg[1], &gErr, w / 3 / 20, h / 3 / 20, 5);
    ImageAlignment(smallColorImg[0], smallColorImg[1], &gErr, 5, 5, 1);
    gErr.u *= 3; // 3�� ��ҵ� �̹����� ��ǥ���� �������� ���� �̹��� ũ��� ��ȯ
    gErr.v *= 3;
    gErr.min_err *= 3;
    ImageAlignment(colorImg[0], colorImg[1], &gErr, 3, 3, 1);

    ImageAlignment(smallColorImg[0], smallColorImg[2], &rErr, w / 3 / 20, h / 3 / 20, 5);
    ImageAlignment(smallColorImg[0], smallColorImg[2], &rErr, 5, 5, 1);
    rErr.u *= 3; // 3�� ��ҵ� �̹����� ��ǥ���� �������� ���� �̹��� ũ��� ��ȯ
    rErr.v *= 3;
    rErr.min_err *= 3;
    ImageAlignment(colorImg[0], colorImg[2], &rErr, 3, 3, 1);;

    // ���� �̹��� ����
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int x1 = x; // blue �̹��� ��ǥ
            int y1 = y;
            int x2 = x + gErr.u; // green �̹��� ��ǥ
            int y2 = y + gErr.v;
            int x3 = x + rErr.u; // red �̹��� ��ǥ
            int y3 = y + rErr.v;

            CvScalar f1 = cvGet2D(colorImg[0], y1, x1); //blueImg �ȼ���
            f1.val[1] = 0; // Green ä���� 0���� �ʱ�ȭ
            if (x2 >= 0 && x2 < w && y2 >= 0 && y2 < h) {
                CvScalar f2 = cvGet2D(colorImg[1], y2, x2); // Green �̹����� �ȼ� ���� �����ϴ� ��� �ش� ���� ������ Red ä�ο� ����
                f1.val[1] = f2.val[1];
            }
            f1.val[2] = 0; // red ä���� 0���� �ʱ�ȭ
            if (x3 >= 0 && x3 < w && y3 >= 0 && y3 < h) {
                CvScalar f3 = cvGet2D(colorImg[2], y3, x3); // Red �̹����� �ȼ� ���� �����ϴ� ��� �ش� ���� ������ Red ä�ο� ����
                f1.val[2] = f3.val[2];
            }
            // ���� �̹����� ������ �ȼ� ���� ����
            cvSet2D(dst, y, x, f1);
        }

    //���� �̹��� ǥ��
    cvShowImage("dst", dst);
    cvWaitKey();

    // �޸� ����
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
    int w = size.width / 3; //��ҵ� �̹����� ������ ���
    int h = size.height / 3;
    IplImage *dst = cvCreateImage(cvSize(w, h), 8, 3); //��ҵ� �̹��� ����

    int K = 1;
    float n = (2 * K + 1) * (2 * K + 1);
    float H[3][3] = { {1 / 16.f, 2 / 16.f, 1 / 16.f}, // ����þ� ���� ���
               {2 / 16.f, 4 / 16.f, 2 / 16.f},
               {1 / 16.f, 2 / 16.f, 1 / 16.f} };

    //�̹��� ������ ���̱� (1/3���� ���̱�) -> �ӵ� ����� ����.
    //�̹��� ����� ���̸鼭 �ػ󵵵� ���߱� ���� ����þ� ���� ���� 
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            int x1 = x * 3; //���� �̹����� �ȼ���
            int y1 = y * 3;

            //�̹����� ����ϸ鼭 ����þ� ���� ����
            CvScalar g = cvScalar(0, 0, 0);
            for (int v = -K; v <= K; v++)
                for (int u = -K; u <= K; u++) {
                    if (x1 + u >= 0 && x1 + u < size.width && y1 + v >= 0 && y1 + v < size.height) // ��ǥ������ ���� �̹��� ���ο� �ִ��� Ȯ���ϰ�, ����þ� ���͸� ����
                    {
                        CvScalar f = cvGet2D(src, y1 + v, x1 + u);
                        for (int k = 0; k < 3; k++)
                            g.val[k] += H[v + K][u + K] * f.val[k]; // ����þ� ���͸� �����Ͽ� ���ο� �ȼ� ���� ���
                    }
                }
            cvSet2D(dst, y, x, g); // ���� ���� �ȼ� ���� ��ҵ� �̹����� ����
        }

    return dst;
}

void ImageAlignment(IplImage *img1, IplImage *img2, MINERR *err, int du, int dv, int dt) // du, dv, dt -> �̵����� �����ϴ� �Ű�����
{
    if (du <= 0 || dv <= 0 || dt <= 0) //�̵����� ��ȿ���� Ȯ�� �����̰ų� 0�� ��� ����
        return;

    CvSize size = cvGetSize(img1);
    int w = size.width;
    int h = size.height;

    int min_u = err->u; //���������̵���
    int min_v = err->v;
    for (int v = min_v - dv; v <= min_v + dv; v += dt) {
        for (int u = min_u - du; u <= min_u + du; u += dt) {
            float e = 0.0f; //���� ���� ����
            int ct = 0; //�ȼ� ���� ���� ����
            for (int y = h / 10; y < h - (h / 10); y += 3) // �̹����� �Ϻ� ������ Ž���Ͽ� ������ ��� -> ���� �������� ������ ���� �κи� Ž���ϱ� ���� ���� ����. �� 10���� ���ڰ� �۾��� ��� ��Ȯ���� �������� ���� Ȯ���Ͽ� 10���� ����.
            {
                for (int x = w / 10; x < w - (w / 10); x += 3) {
                    int x1 = x; //img1������ ��ǥ
                    int y1 = y;
                    int x2 = x + u; //img2������ ��ǥ
                    int y2 = y + v;
                    if (x2 < 0 || x2 > w - 1) continue; //�̹��� ������ ����� ��� ó��X
                    if (y2 < 0 || y2 > h - 1) continue;
                    CvScalar f1 = cvGet2D(img1, y1, x1); //�� �̹����� RGB�� ����
                    CvScalar f2 = cvGet2D(img2, y2, x2);
                    e += (f1.val[0] - f2.val[0]) //�ȼ����� ���̸� ����Ͽ� ���� -> ����
                        * (f1.val[0] - f2.val[0]);
                    ct++; //�ȼ� ���� +1
                }
            }
            e /= ct;
            if (e < err->min_err) //�ּ� ���� ������Ʈ
            {
                err->u = u; //�����̵��� ����
                err->v = v;
                err->min_err = e;
            }
        }
    }
}