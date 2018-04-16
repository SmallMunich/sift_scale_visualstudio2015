#include "sift_scale.h"

IplImage* create_init_img(IplImage* img, int img_dbl, double sigma)
{
	IplImage* gray = NULL, *dbl = NULL;
	IplImage* dbl_copy = NULL;
	float sig_diff = 0;

	int   kernel = 0;
	gray = convert_to_gray32(img);
	if (img_dbl)
	{
		sig_diff = sqrt(sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA * 4);
		dbl = cvCreateImage(cvSize(img->width * 2, img->height * 2),
			                IPL_DEPTH_32F, 1);
		cvResize(gray, dbl, CV_INTER_CUBIC);

		kernel = (int)2 * (1 + (sig_diff - 0.8) / 0.3);
		if (0 == kernel % 2)
			kernel = kernel - 1;

		cvSmooth(dbl, dbl, CV_GAUSSIAN, 0, 0, sig_diff, sig_diff);
		cvReleaseImage(&gray);
		return dbl;
	}
	else
	{
		sig_diff = sqrt(sigma * sigma - SIFT_INIT_SIGMA * SIFT_INIT_SIGMA);
		cvSmooth(gray, gray, CV_GAUSSIAN, 5, 5, sig_diff, sig_diff);
		return gray;
	}
}

IplImage* convert_to_gray32(IplImage* img)
{
	IplImage* gray8 = NULL, *gray32 = NULL;

	gray8 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	gray32 = cvCreateImage(cvGetSize(img), IPL_DEPTH_32F, 1);

	if (img->nChannels == 1){
		// 单通道图像
		gray8 = (IplImage*)cvClone(img);
    }
	else {
		// 三通道图像转换成为灰度图像
		cvCvtColor(img, gray8, CV_RGB2GRAY);
	}
	// 图像深度转换成为IPL_DEPTH_32F同时进行归一化...
	cvConvertScale(gray8, gray32, 1.0 / 255.0, 0);

	cvReleaseImage(&gray8);
	return gray32;
}

IplImage*** build_gauss_pyr(IplImage* base, int octvs, int intvls, double sigma)
{
	IplImage*** gauss_pyr = NULL;
	double* sig = (double*)calloc(intvls + 3, sizeof(double));
	double sig_total = 0, sig_prev = 0, k = 0;
	int i = 0, o = 0;

	gauss_pyr = (IplImage***)calloc(octvs, sizeof(IplImage**));
	for (i = 0; i < octvs; i++){
		// 初始化高斯金字塔内存...
		gauss_pyr[i] = (IplImage**)calloc(intvls + 3, sizeof(IplImage*));
    }
	/*
	   precompute Gaussian sigmas using the following formula:
	   \sigma_{total}^2 = \sigma_{i}^2 + \sigma_{i-1}^2
	*/
	sig[0] = sigma;
	k = pow(2.0, 1.0 / intvls);   // k = pow(x,y); 计算x的y次方
	for (i = 1; i < intvls + 3; i++)
	{   // 计算卷积滤波和参数...
		sig_prev = pow(k, i - 1) * sigma;
		sig_total = sig_prev * k;
		sig[i] = sqrt(sig_total * sig_total - sig_prev * sig_prev);
	}

	// 尺度空间构建过程...
	for (o = 0; o < octvs; o++)
	{
		for (i = 0; i < intvls + 3; i++)
		{
			if (o == 0 && i == 0) {
				gauss_pyr[o][i] = cvCloneImage(base);
			}
			/* base of new octvave is halved image from end of previous octave */
			else if (i == 0) {
				// 在塔间进行下采样......
				gauss_pyr[o][i] = downsample(gauss_pyr[o - 1][intvls]);
			}
			/* blur the current octave's last image to create the next one */
			else
			{   // 不同层数之间进行卷积滤波......
				gauss_pyr[o][i] = cvCreateImage(cvGetSize(gauss_pyr[o][i - 1]),
					IPL_DEPTH_32F, 1);
				cvSmooth(gauss_pyr[o][i - 1], gauss_pyr[o][i],
					CV_GAUSSIAN, 0, 0, sig[i], sig[i]);
			}
		}
	}
	free(sig);
	return gauss_pyr;
}

IplImage* downsample(IplImage* img)
{
	IplImage* smaller = cvCreateImage(cvSize(img->width / 2, img->height / 2),
		                img->depth, img->nChannels);
	// 最近邻插值模式
	cvResize(img, smaller, CV_INTER_NN);

	return smaller;
}

bool save_guass_pyrmid(IplImage ***guass_pyd, int octvs, int intvls, bool save_pyrmid)
{
	char  outputFile[100];
	int o = 0, i = 0;
	IplImage * img_path = NULL;
	int  width = 0, height = 0;

	if (!save_pyrmid)
		return false;
	// 将构建好的尺度空间图像写入保存...
	for (o = 0; o < octvs; ++o)
	{
		for (i = 0; i < intvls + 3; ++i)
		{
			width = guass_pyd[o][i]->width;
			height = guass_pyd[o][i]->height;
			img_path = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, guass_pyd[o][i]->nChannels);

			sprintf(outputFile, "..\\sift_scale\\save_scale\\gauss_blur_%d%d.bmp", o, i);

			cvConvertScale(guass_pyd[o][i], img_path, 255.0, 0);
			cvSaveImage(outputFile, img_path);
		}
	}
	return true;
}