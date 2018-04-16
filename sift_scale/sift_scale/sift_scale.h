#pragma once
/** -- @file
-- This Project's Code is Part of Rob Hess SIFT Code.
-- Functions for detecting SIFT Scale Space.

-- For more information, refer to:

-- Lowe, D.  Distinctive image features from scale-invariant keypoints.
   <EM>International Journal of Computer Vision, 60</EM>, 2 (2004),
   pp.91--110.

-- Copyright (C) 2006  Rob Hess <hess@eecs.oregonstate.edu>
   Note: The SIFT algorithm is patented in the United States and cannot be
   used in commercial products without a license from the University of
   British Columbia.  For more information, refer to the file LICENSE.ubc
   that accompanied this distribution.

-- @version 1.1.1-20070330
*/
#include <opencv2\opencv.hpp>

/* 对输入图像进行高斯模糊去除噪声 */
#define SIFT_INIT_SIGMA 0.5

/** 是否双倍图像大小构建尺度空间 */
#define SIFT_IMG_DBL 1

/** 高斯模糊初始化尺度系数 */
#define SIFT_SIGMA 1.6

/** 默认每一塔中的层数 */
#define SIFT_INTVLS 3

// 对输入图像进行下采样
IplImage* downsample(IplImage*);

// 建立高斯金字塔尺度空间
IplImage*** build_gauss_pyr(IplImage*, int, int, double);

// 创建初始化图像
IplImage* create_init_img(IplImage* img, int img_dbl, double sigma);

// 将图像转换为灰度32F模式下
IplImage* convert_to_gray32(IplImage* img);

// 保存构建尺度空间的高斯金子塔图像
bool save_guass_pyrmid(IplImage ***guass_pyd, int octvs, int intvls, bool save_pyrmid=false);