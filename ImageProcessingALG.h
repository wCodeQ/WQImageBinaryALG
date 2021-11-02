//
//  imageProcessingALG.h
//  ImageDemo
//
//  Created by wq on 2018/12/24.
//  Copyright © 2018 王前. All rights reserved.
//

#ifndef imageProcessingALG_h
#define imageProcessingALG_h

#include <stdio.h>
#include <stdlib.h>

// 获取图片哈希指纹，传入map，width最好为64（建议64*64）
long long * hash_codes(int *bit_map, int width, int height);

// 获取哈希指纹长度
int length_hash_codes(long long *hash_codes);

// 获取两个图片哈希指纹不同长度
int hash_difference_length(long long * hash_codes_1, long long * hash_codes_2);

// 获取灰度图
int * gray_image(int *bit_map, int width, int height);

// 获取灰度直方统计
double * gray_histogram(int *gray_pixels, int width, int height);

// 根据阈值T获取二值图
int * binary_image(int *gray_pixels, int width, int height, int T);

/**
 otsu算法获取二值图

 @param bit_map 图像像素数组地址（ARGB格式）
 @param width 图像宽
 @param height 图像高
 @param T 存储计算得出的阈值
 @return 二值图像素数组地址
 */
int * binary_image_with_otsu_threshold_alg(int *bit_map, int width, int height, int *T);

/**
 平均灰度算法获取二值图
 
 @param bit_map 图像像素数组地址（ARGB格式）
 @param width 图像宽
 @param height 图像高
 @param T 存储计算得出的阈值
 @return 二值图像素数组地址
 */
int * binary_image_with_average_gray_threshold_alg(int *bit_map, int width, int height, int *T);

/**
 双峰平均值算法获取二值图

 @param bit_map 图像像素数组地址（ARGB格式）
 @param width 图像宽
 @param height 图像高
 @param T 存储计算得出的阈值
 @return 二值图像素数组地址
 */
int * binary_image_with_average_peak_threshold_alg(int *bit_map, int width, int height, int *T);

#endif /* imageProcessingALG_h */
