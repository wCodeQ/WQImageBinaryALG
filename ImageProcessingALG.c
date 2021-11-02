//
//  imageProcessingALG.c
//  ImageDemo
//
//  Created by wq on 2018/12/24.
//  Copyright © 2018 王前. All rights reserved.
//

#include "ImageProcessingALG.h"
#include <string.h>

#define min(a,b) ((a)>(b)?(b):(a))
#define byteof(type) sizeof(type) * 8

typedef unsigned char u_char;

int length_hash_codes(long long *hash_codes) {
    long long *p = hash_codes;
    int length = 0;
    while (*p) {
        length++;
        p++;
    }
    return length;
}

long long * hash_codes(int *bit_map, int width, int height) {
    int pixel_total = width * height; //像素总数
    unsigned long sum = 0;  //总灰度
    int hash_code_length = byteof(long long);   //一个hash_code存储长度
    
    int hash_code_count = pixel_total / hash_code_length; //计算hash_code存储数量
    if (hash_code_count == 0) {
        return NULL;
    }
    
    long long *hash_codes = (long long *)malloc((hash_code_count + 1) * sizeof(long long));
    memset(hash_codes, 0, (hash_code_count + 1) * sizeof(long long));
    
    // 获取总灰度
    int *gray_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(gray_pixels, 0, pixel_total * sizeof(int));
    int *p = bit_map;
    for (u_int i = 0; i < pixel_total; i++, p++) {
        // 分离三原色及透明度
        u_char alpha = ((*p & 0xFF000000) >> 24);
        u_char red = ((*p & 0xFF0000) >> 16);
        u_char green = ((*p & 0x00FF00) >> 8);
        u_char blue = (*p & 0x0000FF);
        
        u_char gray = (red*38 + green*75 + blue*15) >> 7;
        if (alpha == 0 && gray == 0) {
            gray = 0xFF;
        }
        gray_pixels[i] = gray;
        sum += gray;
    }
    
    double avg_gray = sum / pixel_total;
    
    for (u_int count = 0; count < hash_code_count; count++) {
        long long hash_code = 0;
        u_int row = count * hash_code_length;
        for (u_int i = 0; i < hash_code_length; i++) {
            u_int index = row + i;
            if (gray_pixels[index] > avg_gray) {
                hash_code = (hash_code | (0xA000000000000000 >> i));
            }
        }
        hash_codes[count] = hash_code;
    }
    free(gray_pixels);
    
    return hash_codes;
}

int hash_difference_length(long long *hash_codes_1, long long *hash_codes_2) {
    int hash_code_length = byteof(long long);   //一个hash_code存储长度

    int hash_codes_count_1 = length_hash_codes(hash_codes_1);
    int hash_codes_count_2 = length_hash_codes(hash_codes_2);
    int difference_length = abs(hash_codes_count_1 - hash_codes_count_2) * hash_code_length;
    
    int min_hash_codes_count = min(hash_codes_count_1, hash_codes_count_2);
    
    for (u_int count = 0; count < min_hash_codes_count; count++) {
        long long hash_code_1 = hash_codes_1[count];
        long long hash_code_2 = hash_codes_2[count];
        for (u_int i = 1; i <= hash_code_length; i++) {
            if (((hash_code_1 >> (hash_code_length - i)) & 0x01) != ((hash_code_2 >> (hash_code_length - i)) & 0x01)) {
                difference_length++;
            }
        }
    }
    return difference_length;
}

// 获取灰度图
int * gray_image(int *bit_map, int width, int height) {
    double pixel_total = width * height; // 像素总数
    if (pixel_total == 0) return NULL;
    // 灰度像素点存储
    int *gray_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(gray_pixels, 0, pixel_total * sizeof(int));
    int *p = bit_map;
    for (u_int i = 0; i < pixel_total; i++, p++) {
        // 分离三原色及透明度
        u_char alpha = ((*p & 0xFF000000) >> 24);
        u_char red = ((*p & 0xFF0000) >> 16);
        u_char green = ((*p & 0x00FF00) >> 8);
        u_char blue = (*p & 0x0000FF);
        
        u_char gray = (red*38 + green*75 + blue*15) >> 7;
        if (alpha == 0 && gray == 0) {
            gray = 0xFF;
        }
        gray_pixels[i] = gray;
    }
    return gray_pixels;
}

// 获取灰度直方统计
double * gray_histogram(int *gray_pixels, int width, int height) {
    double *histogram = (double *)malloc(256 * sizeof(double));
    memset(histogram, 0, 256 * sizeof(double));
    
    double pixel_total = width * height; //像素总数
    if (pixel_total == 0) {
        free(histogram);
        return NULL;
    }
    
    for (u_int i = 0; i < pixel_total; i++) {
        u_char gray = gray_pixels[i];
        histogram[gray]++;
    }
    return histogram;
}

// 根据阈值获取二值图
int * binary_image(int *gray_pixels, int width, int height, int T) {
    double pixel_total = width * height; //像素总数
    int *binary_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(binary_pixels, 0, pixel_total * sizeof(int));
    for (u_int i = 0; i < pixel_total; i++) {
        binary_pixels[i] = gray_pixels[i] <= T ? 0xFF000000:0xFFFFFFFF;
    }
    return binary_pixels;
}

// otsu算法，获取二值图
int * binary_image_with_otsu_threshold_alg(int *bit_map, int width, int height, int *T) {

    double pixel_total = width * height; //像素总数
    if (pixel_total == 0) return NULL;
    
    unsigned long sum1 = 0;  // 总灰度值
    unsigned long sumB = 0;  // 背景总灰度值
    double wB = 0.0;        // 背景像素点比例
    double wF = 0.0;        // 前景像素点比例
    double mB = 0.0;        // 背景平均灰度值
    double mF = 0.0;        // 前景平均灰度值
    double max_g = 0.0;     // 最大类间方差
    double g = 0.0;         // 类间方差
    u_char threshold = 0;    // 阈值
    double histogram[256] = {0}; //灰度直方图，下标是灰度值，保存内容是灰度值对应的像素点总数
    
    // 获取灰度直方图和总灰度
    int *gray_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(gray_pixels, 0, pixel_total * sizeof(int));
    int *p = bit_map;
    for (u_int i = 0; i < pixel_total; i++, p++) {
        // 分离三原色及透明度
        u_char alpha = ((*p & 0xFF000000) >> 24);
        u_char red = ((*p & 0xFF0000) >> 16);
        u_char green = ((*p & 0x00FF00) >> 8);
        u_char blue = (*p & 0x0000FF);
        
        u_char gray = (red*38 + green*75 + blue*15) >> 7;
        if (alpha == 0 && gray == 0) {
            gray = 0xFF;
        }
        gray_pixels[i] = gray;
        
        //计算灰度直方图分布，Histogram数组下标是灰度值，保存内容是灰度值对应像素点数
        histogram[gray]++;
        sum1 += gray;
    }
    
    // otsu算法
    for (u_int i = 0; i < 256; i++)
    {
        wB = wB + histogram[i]; // 这里不算比例，减少运算，不会影响求T
        wF = pixel_total - wB;
        if (wB == 0 || wF == 0)
        {
            continue;
        }
        sumB = sumB + i * histogram[i];
        mB = sumB / wB;
        mF = (sum1 - sumB) / wF;
        g = wB * wF * (mB - mF) * (mB - mF);
        if (g >= max_g)
        {
            threshold = i;
            max_g = g;
        }
    }
    
    for (u_int i = 0; i < pixel_total; i++) {
        gray_pixels[i] = gray_pixels[i] <= threshold ? 0xFF000000:0xFFFFFFFF;
    }
    
    if (T) {
        *T = threshold;    // otsu算法阈值
    }
    
    return gray_pixels;
}

// 平均灰度值算法
int * binary_image_with_average_gray_threshold_alg(int *bit_map, int width, int height, int *T) {
    
    double pixel_total = width * height; // 像素总数
    if (pixel_total == 0) return NULL;
    
    unsigned long sum = 0;  // 总灰度
    u_char threshold = 0;    // 阈值


    int *gray_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(gray_pixels, 0, pixel_total * sizeof(int));
    int *p = bit_map;
    for (u_int i = 0; i < pixel_total; i++, p++) {
        // 分离三原色及透明度
        u_char alpha = ((*p & 0xFF000000) >> 24);
        u_char red = ((*p & 0xFF0000) >> 16);
        u_char green = ((*p & 0x00FF00) >> 8);
        u_char blue = (*p & 0x0000FF);
        
        u_char gray = (red*38 + green*75 + blue*15) >> 7;
        if (alpha == 0 && gray == 0) {
            gray = 0xFF;
        }
        gray_pixels[i] = gray;
        sum += gray;
    }
    // 计算平均灰度
    threshold = sum / pixel_total;
    
    for (u_int i = 0; i < pixel_total; i++) {
        gray_pixels[i] = gray_pixels[i] <= threshold ? 0xFF000000:0xFFFFFFFF;
    }
    
    if (T) {
        *T = threshold;
    }
    
    return gray_pixels;
}

// 判断是否是双峰直方图
int is_double_peak(double *histogram) {
    // 判断直方图是存在双峰
    int peak_count = 0;
    for (int i = 1; i < 255; i++) {
        if (histogram[i - 1] < histogram[i] && histogram[i + 1] < histogram[i]) {
            peak_count++;
            if (peak_count > 2) return 0;
        }
    }
    return peak_count == 2;
}
// 双峰平均值算法
int * binary_image_with_average_peak_threshold_alg(int *bit_map, int width, int height, int *T) {
    double pixel_total = width * height; // 像素总数
    if (pixel_total == 0) return NULL;
    
    // 灰度直方图，下标是灰度值，保存内容是灰度值对应的像素点总数
    double histogram1[256] = {0};
    double histogram2[256] = {0}; // 求均值的过程会破坏前面的数据，因此需要两份数据
    u_char threshold = 0;    // 阈值

    // 获取灰度直方图
    int *gray_pixels = (int *)malloc(pixel_total * sizeof(int));
    memset(gray_pixels, 0, pixel_total * sizeof(int));
    int *p = bit_map;
    for (u_int i = 0; i < pixel_total; i++, p++) {
        // 分离三原色及透明度
        u_char alpha = ((*p & 0xFF000000) >> 24);
        u_char red = ((*p & 0xFF0000) >> 16);
        u_char green = ((*p & 0x00FF00) >> 8);
        u_char blue = (*p & 0x0000FF);
        
        u_char gray = (red*38 + green*75 + blue*15) >> 7;
        if (alpha == 0 && gray == 0) {
            gray = 0xFF;
        }
        gray_pixels[i] = gray;
        
        // 计算灰度直方图分布，Histogram数组下标是灰度值，保存内容是灰度值对应像素点数
        histogram1[gray]++;
        histogram2[gray]++;
    }
    
    // 如果不是双峰，则通过三点求均值来平滑直方图
    int times = 0;
    while (!is_double_peak(histogram2)) {
        times++;
        if (times > 1000) {
            free(gray_image);
            return NULL;                          // 似乎直方图无法平滑为双峰的，返回错误代码
        }
        histogram2[0] = (histogram1[0] + histogram1[0] + histogram1[1]) / 3;                   // 第一点
        for (int i = 1; i < 255; i++) {
            histogram2[i] = (histogram1[i - 1] + histogram1[i] + histogram1[i + 1]) / 3;       // 中间的点
        }
        histogram2[255] = (histogram1[254] + histogram1[255] + histogram1[255]) / 3;           // 最后一点
        memcpy(histogram1, histogram2, 256 * sizeof(double));                                  // 备份数据，为下一次迭代做准备
    }
    
    // 求阈值T
    int peak[2] = {0};
    for (int i = 1, y = 0; i < 255; i++) {
        if (histogram2[i - 1] < histogram2[i] && histogram2[i + 1] < histogram2[i]) {
            peak[y++] = i;
        }
    }
    threshold = (peak[0] + peak[1]) / 2;
    
    for (u_int i = 0; i < pixel_total; i++) {
        gray_pixels[i] = gray_pixels[i] <= threshold ? 0xFF000000:0xFFFFFFFF;
    }
    
    if (T) {
        *T = threshold;
    }
    
    return gray_pixels;
}
