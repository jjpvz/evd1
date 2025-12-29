/*! ***************************************************************************
 *
 * \brief     Unit test functions
 * \file      main.c
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      January 2025
 *
 * \note      Successfully tested with the following software versions:
 *            - GCC 14.2.0 x86_64-w64-mingw32 (ucrt64)
 *            - CMake for Visual Studio Code 0.0.17
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "main.h"

// Uncomment to test the assignment functions only
// Comment to test all functions
#define TEST_ASSIGNMENTS_ONLY

// This function is called before each test
void setUp(void)
{
}

// This function is called after each test
void tearDown(void)
{
}

// ----------------------------------------------------------------------------
int main(void)
{
    UNITY_BEGIN();

    printf("EVDK UNIT TESTS\n\n");

    printf("CODING AND COMPRESSION\n");
    RUN_TEST(test_make_huffman_pq);
    RUN_TEST(test_make_huffman_tree);
    RUN_TEST(test_encode_image);
    RUN_TEST(test_decode_image);
#ifndef TEST_ASSIGNMENTS_ONLY
#endif
    // printf("\n");

    printf("GRAPHICS ALGORITHMS\n");
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_affineTransformation);
    RUN_TEST(test_warpPerspective);
    RUN_TEST(test_warpPerspectiveFast);
    RUN_TEST(test_zoom);
    RUN_TEST(test_zoomFactor);
#endif
    // printf("\n");

    printf("HISTOGRAM OPERATIONS\n");
    RUN_TEST(test_contrast);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_brightness);
    RUN_TEST(test_histogram);
#endif
    // printf("\n");

    printf("IMAGE FUNDAMENTALS\n");
    RUN_TEST(test_convertUyvyToUint8);
    RUN_TEST(test_scaleFast);
    RUN_TEST(test_convolveFast);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_convolve);
#endif
    // printf("\n");

    printf("MENSURATION\n");
    RUN_TEST(test_labelTwoPass);
    RUN_TEST(test_perimeter);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_area);
    RUN_TEST(test_labelIterative);
#endif
    // printf("\n");

    printf("MORPHOLOGICAL FILTERS\n");
    RUN_TEST(test_fillHolesTwoPass);
    RUN_TEST(test_removeBorderBlobsTwoPass);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_dilation);
    RUN_TEST(test_dilationGray);
    RUN_TEST(test_erosion);
    RUN_TEST(test_erosionGray);
    RUN_TEST(test_fillHolesIterative);
    RUN_TEST(test_hitmiss);
    RUN_TEST(test_removeBorderBlobsIterative);
    RUN_TEST(test_skeleton);
#endif
    // printf("\n");

    printf("NOISE\n");
#ifndef TEST_ASSIGNMENTS_ONLY
// RUN_TEST();
#endif
    // printf("\n");

    printf("NONLINEAR FILTERS\n");
    RUN_TEST(test_meanFast);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_harmonic);
    RUN_TEST(test_maximum);
    RUN_TEST(test_mean);
    RUN_TEST(test_median);
    RUN_TEST(test_midpoint);
    RUN_TEST(test_minimum);
    RUN_TEST(test_range);
#endif
    // printf("\n");

    printf("SEGMENTATION\n");
    RUN_TEST(test_threshold2Means);
    RUN_TEST(test_thresholdOtsu);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_threshold);
    RUN_TEST(test_thresholdOptimum);
    RUN_TEST(test_lineDetector);
#endif
    // printf("\n");

    printf("SPATIAL FILTERS\n");
    RUN_TEST(test_sobelFast);
#ifndef TEST_ASSIGNMENTS_ONLY
    RUN_TEST(test_gaussianFilter);
    RUN_TEST(test_laplacian);
    RUN_TEST(test_sobel);
#endif
    // printf("\n");

    printf("SPATIAL FREQUENCY FILTERS\n");
#ifndef TEST_ASSIGNMENTS_ONLY
// RUN_TEST();
#endif
    // printf("\n");

    printf("TRANSFORMS\n");
#ifndef TEST_ASSIGNMENTS_ONLY
// RUN_TEST();
#endif
    // printf("\n");

    return UNITY_END();
}

// ----------------------------------------------------------------------------
void prettyprint(const image_t *img, const char *title)
{
    printf("\n%s\n", title);

    for (int r = 0; r < img->rows; r++)
    {
        for (int c = 0; c < img->cols; c++)
        {
            // Print the pixel based on the image type
            if (img->type == IMGTYPE_UINT8)
            {
                printf("%3d, ", getUint8Pixel(img, c, r));
            }
            else if (img->type == IMGTYPE_INT16)
            {
                printf("%5d, ", getInt16Pixel(img, c, r));
            }
            else if (img->type == IMGTYPE_INT32)
            {
                printf("%5d, ", getInt32Pixel(img, c, r));
            }
            else if (img->type == IMGTYPE_FLOAT)
            {
                printf("%8.3f, ", getFloatPixel(img, c, r));
            }
            else if (img->type == IMGTYPE_UYVY)
            {
                printf("0x%04X, ", getUyvyPixel(img, c, r));
            }
            else
            {
                printf("Image type not supported\n");
                fflush(stdout);
                return;
            }
        }
        printf("\n");
        fflush(stdout);
    }

    printf("\n");
    fflush(stdout);
}
