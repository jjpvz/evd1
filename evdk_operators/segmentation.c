/*! ***************************************************************************
 *
 * \brief     Segmentation operations
 * \file      segmentation.c
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      October 2024
 *
 * \see       Myler, H. R., & Weeks, A. R. (2009). The pocket handbook of
 *            image processing algorithms in C. Prentice Hall Press.
 * \see       Morse, B. (2000). Lecture 4: Thresholding. Retrieved June 17,
 *            2020, from
 *            http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/MORSE/threshold.pdf
 * \see       Gonzales, R. C., & Woods, R. E. (2002). Digital image
 *            processing.
 *
 * \copyright 2024 HAN University of Applied Sciences. All Rights Reserved.
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
#include "histogram_operations.h"
#include "image_fundamentals.h"
#include "segmentation.h"

/*!
 * \brief Separates object from background
 *
 * The source image is a graylevel image and the destination is a binary image.
 * Thresholding is defined as
 * \n
 * p_dst(x,y) = 1 if \p min <= p_src(x,y) <= \p max \n
 * p_dst(x,y) = 0 otherwise
 * \n
 * The object values are set to 1.
 * The background values are set to 0.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  min Minimum graylevel that is part of an object
 * \param[in]  max Maximum graylevel that is part of an object
 */
void threshold(const image_t *src, image_t *dst,
               const uint8_pixel_t min, const uint8_pixel_t max)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");

    // Verify image consistency
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    uint32_t i = src->rows * src->cols;
    uint8_pixel_t *s = (uint8_pixel_t *)src->data;
    uint8_pixel_t *d = (uint8_pixel_t *)dst->data;

    // Loop all pixels and set to 1 if the pixel is within thresholding window
    while (i-- > 0)
    {
        uint8_pixel_t pixel = *s++;
        *d++ = ((pixel >= min) && (pixel <= max)) ? 1 : 0;
    }
}

/*!
 * \brief Automatic thresholding by finding the valley between the two peaks
 *
 * The function assumes that the image’s histogram has two predominant peaks.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  b   Return the bright or the BRIGHTNESS_DARK areas in the source image as
 *                 object. Must be of type ::eBrightness
 */
void thresholdOptimum(const image_t *src, image_t *dst, const eBrightness b)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");

    // Verify image consistency
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    int32_t y;

    // Create histogram
    uint32_t hist1[256];
    float hist2[256];
    histogram(src, hist1);

    // Apply 31x1 mean filter on the histogram
    for (y = 0; y < 256; y++)
    {
        uint32_t sum = 0.0f;
        uint32_t cnt = 0;

        for (int32_t x = -15; x <= 15; ++x)
        {
            // Is the calculated index valid?
            int32_t index = y + x;

            if ((index >= 0) && (index <= 255))
            {
                ++cnt;
                sum += hist1[index];
            }
        }

        hist2[y] = (float)sum / (float)cnt;
    }

    uint32_t flag = 0;
    uint8_pixel_t t = 0;

    // Find the first non-zero value in the histogram,
    // skipping the very first value
    for (y = 1; y < 256; y++)
    {
        if (hist2[y] != 0.0f)
            break;
    }

    // Find the valley between the two peaks,
    // skipping the final value
    while ((flag == 0) && (y <= 254))
    {
        if ((hist2[y - 1] >= hist2[y]) && (hist2[y] < hist2[y + 1]))
        {
            flag = 1;
            t = y;
        }

        ++y;
    }

    // Threshold the image
    if (b == BRIGHTNESS_DARK)
        threshold(src, dst, 0, t);
    else
        threshold(src, dst, t, 255);
}

/*!
 * \brief Automatic thresholding by minimizing the overlap of two clusters of
 *        pixels
 *
 * Uses the iterative K-means algorithm to minimize the overlap between the
 * graylevel object and background.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  b   Return the bright or the BRIGHTNESS_DARK areas in the source image as
 *                 object. Must be of type ::eBrightness
 */
void threshold2Means(const image_t *src, image_t *dst, const eBrightness b)
{
    // calculate the histogram
    uint32_t hist[256] = {0};

    histogram(src, hist);

    // initialize mean intensity to the global mean intensity
    uint32_t total = src->rows * src->cols;

    int T = 0;

    for (uint32_t i = 0; i < 256; i++)
    {
        T += i * hist[i];
    }

    T /= total;

    int T_new = T;

    // Iterative 2-means algorithm
    do
    {
        T = T_new;

        uint32_t sum_left = 0;
        uint32_t count_left = 0;
        uint32_t sum_right = 0;
        uint32_t count_right = 0;

        // Split into two groups based on current threshold T
        for (uint32_t i = 0; i < 256; i++)
        {
            if (i <= T)
            {
                sum_left += i * hist[i];
                count_left += hist[i];
            }
            else
            {
                sum_right += i * hist[i];
                count_right += hist[i];
            }
        }

        // Prevent division by zero
        if (count_left == 0 || count_right == 0)
        {
            break;
        }

        // Calculate mean value of all pixels to the left of T (mean left)
        int mean_left = sum_left / count_left;

        // Calculate mean value of all pixels to the right of T (mean right)
        int mean_right = sum_right / count_right;

        // Move T to the mean of mean left and mean right
        T_new = ((mean_left + mean_right) + 1) / 2;

    } while (T_new != T); // Repeat as long as T changes

    // Threshold the image
    if (b == BRIGHTNESS_DARK)
    {
        threshold(src, dst, 0, T_new);
    }
    else
    {
        threshold(src, dst, T_new, 255);
    }
}

/*!
 * \brief Automatic thresholding using Otsu's method
 *
 * Otsu's method assumes that the histogram shows two clusters and that these
 * clusters are normal distributions. The threshold with the two ‘best’ normal
 * distributions gives the optimum threshold. The two ‘best’ normal
 * distributions have the lowest sum of variances. Or, as stated by Otsu:
 * "The Between Class Variance (BCV) is as high as possible".
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  b   Return the bright or the BRIGHTNESS_DARK areas in the source image as
 *                 object. Must be of type ::eBrightness
 */
void thresholdOtsu(const image_t *src, image_t *dst, const eBrightness b)
{
    uint32_t hist[256] = {0};
    histogram(src, hist);

    uint32_t total = src->rows * src->cols;

    // Calculate the sum of intensity of the entire image
    uint32_t sum_total = 0;
    for (int i = 0; i < 256; i++)
    {
        sum_total += i * hist[i];
    }

    // All pixels are in the right and are iteratively moved to the left
    uint32_t sum_left = 0;
    uint32_t count_left = 0;

    uint32_t sum_right = sum_total;
    uint32_t count_right = total;

    int best_T = 0;
    double best_bcv = 0.0;

    for (uint32_t T = 0; T < 256; T++)
    {
        // Values added to the left
        sum_left += (uint32_t)T * hist[T];
        count_left += hist[T];

        // are removed from the right
        sum_right -= (uint32_t)T * hist[T];
        count_right -= hist[T];

        // Prevent division by zero
        if (count_left == 0 || count_right == 0)
        {
            continue;
        }

        // Calculate mean value of all pixels to the left of T (mean left)
        double mean_left = (double)sum_left / count_left;

        // Calculate mean value of all pixels to the right of T (mean right)
        double mean_right = (double)sum_right / count_right;

        // BCV = w_left​ x w_right​ x ( mean_left − mean_right )2
        double w_left = (double)count_left / total;
        double w_right = (double)count_right / total;

        double bcv = w_left * w_right * (mean_left - mean_right) * (mean_left - mean_right);

        // Keep best threshold
        if (bcv > best_bcv)
        {
            best_bcv = bcv;
            best_T = T;
        }
    }

    // Threshold the image using the best threshold
    if (b == BRIGHTNESS_DARK)
    {
        threshold(src, dst, 0, best_T);
    }
    else
    {
        threshold(src, dst, best_T, 255);
    }
}

/*!
 * \brief For finding line discontinuities within an image
 *
 * This function is a spatial filter of the source image with a 3x3 mask.
 * Use the following masks for finding lines in specific directions:
 *
 * <table>
 * <caption id="horizontal">Horizontal</caption>
 * <tr><th> -1 <th> -1 <th> -1
 * <tr><th>  2 <th>  2 <th>  2
 * <tr><th> -1 <th> -1 <th> -1
 * </table>
 *
 * <table>
 * <caption id="Vertical">Vertical</caption>
 * <tr><th> -1 <th> 2 <th> -1
 * <tr><th> -1 <th> 2 <th> -1
 * <tr><th> -1 <th> 2 <th> -1
 * </table>
 *
 * <table>
 * <caption id="left_diagonal">Left diagonal</caption>
 * <tr><th>  2 <th> -1 <th> -1
 * <tr><th> -1 <th>  2 <th> -1
 * <tr><th> -1 <th> -1 <th>  2
 * </table>
 *
 * <table>
 * <caption id="right_diagonal">Right diagonal</caption>
 * <tr><th> -1 <th> -1 <th>  2
 * <tr><th> -1 <th>  2 <th> -1
 * <tr><th>  2 <th> -1 <th> -1
 * </table>
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a 3x3 mask
 */
void lineDetector(const image_t *src, image_t *dst, int16_t mask[][3])
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");

    // Verify image consistency
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");
    ASSERT(src == dst, "src and dst are the same images");

    // Set first and last row in dst to 0
    for (int32_t x = 0; x < dst->cols; x++)
    {
        setUint8Pixel(dst, x, 0, 0);
        setUint8Pixel(dst, x, dst->rows - 1, 0);
    }

    // Set first and last column in dst to 0
    for (int32_t y = 0; y < dst->rows; y++)
    {
        setUint8Pixel(dst, 0, y, 0);
        setUint8Pixel(dst, dst->cols - 1, y, 0);
    }

    int32_t sum;

    // Loop all pixels, skipping the border pixels
    for (int32_t y = 1; y < (src->rows - 1); y++)
    {
        for (int32_t x = 1; x < (src->cols - 1); x++)
        {
            // Calculate the sum under the 3x3 mask
            sum = 0;
            for (int32_t j = -1; j <= 1; j++)
            {
                for (int32_t i = -1; i <= 1; i++)
                {
                    sum += getUint8Pixel(src, (x + i), (y + j)) * mask[j + 1][i + 1];
                }
            }

            // Clip the result
            if (sum > 255)
            {
                sum = 255;
            }

            if (sum < 0)
            {
                sum = 0;
            }

            // Store the result
            setUint8Pixel(dst, x, y, sum);
        }
    }
}
