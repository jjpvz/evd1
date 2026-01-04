/*! ***************************************************************************
 *
 * \brief     Smooth the contours of objects and decompose an image into its
 *            fundamental geometrical shape
 * \file      morphological_filters.c
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      October 2024
 *
 * \see       Myler, H. R., & Weeks, A. R. (2009). The pocket handbook of
 *            image processing algorithms in C. Prentice Hall Press.
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
#include "image_fundamentals.h"
#include "morphological_filters.h"

#include <string.h>

/*!
 * \brief Binary dilation of an object increases its geometrical area
 *
 * Dilation is defined as the union of all vector additions of all pixels a
 * in object A with all pixels b in the structuring function B (\p mask).
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void dilation(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Loop all pixels
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            uint32_t smax = 0;

            // Apply the kernel only for pixels within the image
            for (int32_t j = -n / 2; j <= n / 2; j++)
            {
                for (int32_t i = -n / 2; i <= n / 2; i++)
                {
                    if ((x + i) >= 0 &&
                        (y + j) >= 0 &&
                        (x + i) < src->cols &&
                        (y + j) < src->rows)
                    {
                        // Is the pixel set and is the corresponding
                        // cell in the mask set?
                        if ((getUint8Pixel(src, x + i, y + j) == 1) &&
                            (mask[((j + (n / 2)) * n) + (i + (n / 2))] == 1))
                        {
                            // Mark this cell for dilation
                            smax = 1;
                        }
                    }
                }
            }

            // Store the result
            setUint8Pixel(dst, x, y, smax);
        }
    }
}

/*!
 * \brief Graylevel dilation is used to smooth small negative graylevel regions
 *
 * Graylevel dilation is defined as the maximum of the sum of a local region of
 * an image and a given graylevel \p mask.
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void dilationGray(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Loop all pixels
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            int32_t smax = 0;

            // Apply the kernel only for pixels within the image
            for (int32_t j = -n / 2; j <= n / 2; j++)
            {
                for (int32_t i = -n / 2; i <= n / 2; i++)
                {
                    if ((x + i) >= 0 &&
                        (y + j) >= 0 &&
                        (x + i) < src->cols &&
                        (y + j) < src->rows)
                    {
                        int32_t val = getUint8Pixel(src, x + i, y + j) + mask[((j + (n / 2)) * n) + (i + (n / 2))];

                        if (val > smax)
                            smax = val;
                    }
                }
            }

            // Clip the result
            if (smax > 255)
            {
                smax = 255;
            }

            // Store the result
            setUint8Pixel(dst, x, y, smax);
        }
    }
}

/*!
 * \brief Binary erosion of an object decreases its geometrical area
 *
 * Erosion is defined as the complement of the resulting dilation of the
 * complement of object A with structuring function B (\p mask).
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void erosion(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Loop all pixels
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            uint32_t smin = 1;

            // Apply the kernel only for pixels within the image
            for (int32_t j = -n / 2; j <= n / 2; j++)
            {
                for (int32_t i = -n / 2; i <= n / 2; i++)
                {
                    if ((x + i) >= 0 &&
                        (y + j) >= 0 &&
                        (x + i) < src->cols &&
                        (y + j) < src->rows)
                    {
                        // Is the pixel background and is the corresponding
                        // cell in the mask set?
                        if ((getUint8Pixel(src, x + i, y + j) == 0) &&
                            (mask[((j + (n / 2)) * n) + (i + (n / 2))] == 1))
                        {
                            // Mark this cell for erosion
                            smin = 0;
                        }
                    }
                }
            }

            // Store the result
            setUint8Pixel(dst, x, y, smin);
        }
    }
}

/*!
 * \brief Graylevel dilation is used to smooth small positive graylevel regions
 *
 * Graylevel dilation is defined as the minimum of the difference of a local
 * region of an image and a given graylevel \p mask.
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void erosionGray(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Loop all pixels
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            int32_t smin = 255;

            // Apply the kernel only for pixels within the image
            for (int32_t j = -n / 2; j <= n / 2; j++)
            {
                for (int32_t i = -n / 2; i <= n / 2; i++)
                {
                    if ((x + i) >= 0 &&
                        (y + j) >= 0 &&
                        (x + i) < src->cols &&
                        (y + j) < src->rows)
                    {
                        int32_t val = getUint8Pixel(src, x + i, y + j) - mask[((j + (n / 2)) * n) + (i + (n / 2))];

                        if (val < smin)
                            smin = val;
                    }
                }
            }

            // Clip the result
            if (smin < 0)
            {
                smin = 0;
            }

            // Store the result
            setUint8Pixel(dst, x, y, smin);
        }
    }
}

/*!
 * \brief Fills the holes of a binary object
 *
 * Connectivity is as seen from the hole. If the hole is 4-connected, the
 * object’s boundary is 8-connected and vice versa.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  c   The hole's connectivity. Must be of type ::eConnected.
 */
void fillHolesIterative(const image_t *src, image_t *dst, const eConnected c)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verify image consistency
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Copy the image if needed
    if (src != dst)
    {
        copyUint8Image(src, dst);
    }

    // Mark the border pixels that are set
    for (int32_t y = 0; y < dst->rows; y++)
    {
        if (getUint8Pixel(dst, 0, y) == 0)
        {
            setUint8Pixel(dst, 0, y, 2);
        }

        if (getUint8Pixel(dst, (dst->cols - 1), y) == 0)
        {
            setUint8Pixel(dst, (dst->cols - 1), y, 2);
        }
    }

    for (int32_t x = 0; x < dst->cols; x++)
    {
        if (getUint8Pixel(dst, x, 0) == 0)
        {
            setUint8Pixel(dst, x, 0, 2);
        }

        if (getUint8Pixel(dst, x, (dst->rows - 1)) == 0)
        {
            setUint8Pixel(dst, x, (dst->rows - 1), 2);
        }
    }

    uint32_t changes = 0;
    uint32_t iterations = 0;

    // Set all 0 pixels that are connected to an edge to the marker value (2)
    do
    {
        // Loop as long as there are changes
        changes = 0;

        // Pass through from left-top to right-bottom
        // Discard first row, last row, first column and last column
        for (int32_t y = 1; y < dst->rows - 1; y++)
        {
            for (int32_t x = 1; x < dst->cols - 1; x++)
            {
                // Does the pixel belong to the background?
                if (getUint8Pixel(dst, x, y) == 0)
                {
                    // Is a neighbour marked?
                    if (neighbourCount(dst, x, y, 2, c) > 0)
                    {
                        // Also mark this pixel
                        setUint8Pixel(dst, x, y, 2);
                        changes = 1;
                    }
                }
            }
        }

        iterations++;

        if (!changes)
            break;

        // Pass through from right-bottom to left-top
        // Discard first row, last row, firs column and last column
        for (int32_t y = dst->rows - 2; y > 0; y--)
        {
            for (int32_t x = dst->cols - 2; x > 0; x--)
            {
                // Does the pixel belong to the background?
                if (getUint8Pixel(dst, x, y) == 0)
                {
                    // Is a neighbour marked?
                    if (neighbourCount(dst, x, y, 2, c) > 0)
                    {
                        // Also mark this pixel
                        setUint8Pixel(dst, x, y, 2);
                        changes = 1;
                    }
                }
            }
        }

        iterations++;
    } while (changes != 0);

    // Set background to object value
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            if (getUint8Pixel(dst, x, y) == 0)
            {
                setUint8Pixel(dst, x, y, 1);
            }
        }
    }

    // Set marked value to background value
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            if (getUint8Pixel(dst, x, y) == 2)
            {
                setUint8Pixel(dst, x, y, 0);
            }
        }
    }
}

/*!
 * \brief Fills the holes of a binary object
 *
 * Connectivity is as seen from the hole. If the hole is 4-connected, the
 * object’s boundary is 8-connected and vice versa.
 *
 * \param[in]  src     A pointer to the source image
 * \param[out] dst     A pointer to the destination image
 * \param[in]  c       Connectivity defined by ::eConnected
 * \param[in]  lutSize   The maximum number of labels to find. Is used to
 *                       dynamically allocate memory for the lookup table. A
 *                       lower value is faster, but is also able to find fewer
 *                       labels. The value is application dependent.
 *
 * \return 0 Failure
 *           - Memory allocation failed
 *           - Lookup table is too small
 *         1 Success
 */
uint32_t fillHolesTwoPass(const image_t *src, image_t *dst,
                          const eConnected connected, const uint32_t lutSize)
{
    uint32_t *lut = (uint32_t *)malloc(lutSize * sizeof(uint32_t));

    if (lut == NULL)
    {
        return 0;
    }

    for (uint32_t i = 0; i < lutSize; ++i)
    {
        lut[i] = 0;
    }

    lut[1] = 1;
    lut[2] = 2;

    uint32_t nextLabel = 3;

    for (uint32_t y = 0; y < src->rows; y++)
    {
        for (uint32_t x = 0; x < src->cols; x++)
        {
            uint32_t idx = y * src->cols + x;

            // if on border
            if (x == 0 || x == src->cols - 1 || y == 0 || y == src->rows - 1)
            {
                // if background
                if (src->data[idx] == 0)
                {
                    // set in destination 2
                    dst->data[idx] = 2;
                }
            }
        }
    }

    // Pass 1: Label the background, skipping the borders, and record equivalences in lut
    for (int32_t y = 1; y < src->rows - 1; y++)
    {
        for (int32_t x = 1; x < src->cols - 1; x++)
        {
            uint32_t idx = y * src->cols + x;

            // if background
            if (src->data[idx] == 0)
            {
                /*
                    1 1 1
                    1 0 0
                    0 0 0
                */
                uint32_t n = dst->data[idx - src->cols];
                uint32_t w = dst->data[idx - 1];
                uint32_t nw = 0;
                uint32_t ne = 0;

                // Alleen diagonalen meenemen als we 8-connected gebruiken
                if (connected == CONNECTED_EIGHT)
                {
                    nw = dst->data[idx - src->cols - 1];
                    ne = dst->data[idx - src->cols + 1];
                }

                // if neighbours labelled
                if (n > 1 || nw > 1 || ne > 1 || w > 1)
                {
                    // 1. Label this pixel with the lowest value of neighbor
                    uint32_t minLabel = UINT32_MAX;

                    if (n > 1 && n < minLabel)
                    {
                        minLabel = n;
                    }

                    if (nw > 1 && nw < minLabel)
                    {
                        minLabel = nw;
                    }

                    if (ne > 1 && ne < minLabel)
                    {
                        minLabel = ne;
                    }

                    if (w > 1 && w < minLabel)
                    {
                        minLabel = w;
                    }

                    // 2. Label this pixel with that value
                    dst->data[idx] = minLabel;

                    // record equivalence(s)
                    if (n > 1 && n != minLabel)
                    {
                        lut[n] = minLabel;
                    }

                    if (nw > 1 && nw != minLabel)
                    {
                        lut[nw] = minLabel;
                    }

                    if (ne > 1 && ne != minLabel)
                    {
                        lut[ne] = minLabel;
                    }

                    if (w > 1 && w != minLabel)
                    {
                        lut[w] = minLabel;
                    }
                }
                else
                {
                    if (nextLabel >= lutSize)
                    {
                        free(lut);
                        return 0; // Lookup table is too small
                    }

                    // Assign next label value
                    dst->data[idx] = nextLabel;

                    // record in lut
                    lut[nextLabel] = nextLabel;
                    nextLabel++;
                }
            }
            else
            {
                // label this pixel with foreground value
                dst->data[idx] = 1;
            }
        }
    }

    // Record border equivalences
    for (int32_t y = 1; y < dst->rows - 1; y++)
    {
        for (int32_t x = 1; x < dst->cols - 1; x++)
        {
            uint32_t idx = y * dst->cols + x;
            uint32_t currentLabel = dst->data[idx];

            // if object
            if (currentLabel != 0)
            {
                // if col 2
                if (x == 1)
                {
                    // check pixels links
                    uint32_t left = dst->data[idx - 1];

                    // als waarde = 2
                    if (left == 2)
                    {
                        // pas aan in lut
                        lut[currentLabel] = 2;
                    }

                    if (connected == CONNECTED_EIGHT)
                    {
                        uint32_t bottom_left = dst->data[idx + src->cols - 1];

                        if (bottom_left == 2)
                        {
                            lut[currentLabel] = 2;
                        }
                    }
                }

                // if col max - 1
                if (x == dst->cols - 2)
                {
                    // check pixels rechts
                    uint32_t right = dst->data[idx + 1];

                    // als waarde = 2
                    if (right == 2)
                    {
                        // pas aan in lut
                        lut[currentLabel] = 2;
                    }
                }

                // if row max - 1
                if (y == dst->rows - 2)
                {
                    // check pixels onder
                    uint32_t bottom = dst->data[idx + dst->cols];

                    // als waarde = 2
                    if (bottom == 2)
                    {
                        // pas aan in lut
                        lut[currentLabel] = 2;
                    }

                    if (connected == CONNECTED_EIGHT)
                    {
                        uint32_t bottom_left = dst->data[idx + dst->cols - 1];

                        if (bottom_left == 2)
                        {
                            lut[currentLabel] = 2;
                        }

                        uint32_t bottom_right = dst->data[idx + dst->cols + 1];

                        if (bottom_right == 2)
                        {
                            lut[currentLabel] = 2;
                        }
                    }
                }
            }
        }
    }

    // Resolve equivalences
    for (uint32_t i = 3; i < nextLabel; i++)
    {
        if (lut[i] != i)
        {
            lut[i] = lut[lut[i]];
        }
    }

    for (uint32_t y = 0; y < dst->rows; y++)
    {
        for (uint32_t x = 0; x < dst->cols; x++)
        {
            uint32_t idx = y * dst->cols + x;
            uint32_t currentLabel = dst->data[idx];

            if (currentLabel == 0)
            {
                dst->data[idx] = 1;
            }
            else if (currentLabel == 2 || (currentLabel >= 3 && lut[currentLabel] == 2))
            {
                dst->data[idx] = 0;
            }
            else
            {
                dst->data[idx] = 1;
            }
        }
    }

    free(lut);

    return 1;
}

/*!
 * \brief This function is used to find geometrical features
 *
 * The function uses a hit mask and a miss mask with the requirement that the
 * intersection of the two masks is empty.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  m1  3x3 Hit mask
 * \param[in]  m2  3x3 Miss mask
 */
void hitmiss(const image_t *src, image_t *dst, const uint8_t *m1, const uint8_t *m2)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT((m1[0] & m2[0]) == 1 ||
               (m1[1] & m2[1]) == 1 ||
               (m1[2] & m2[2]) == 1 ||
               (m1[3] & m2[3]) == 1 ||
               (m1[4] & m2[4]) == 1 ||
               (m1[5] & m2[5]) == 1 ||
               (m1[6] & m2[6]) == 1 ||
               (m1[7] & m2[7]) == 1 ||
               (m1[8] & m2[8]) == 1,
           " m1 AND m2 must be 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    // Create temporary image
    image_t *org = newUint8Image(src->cols, src->rows);
    image_t *tmp = newUint8Image(src->cols, src->rows);

    ASSERT(tmp == NULL, "not enough memory for image allocation");

    copyUint8Image(src, org);

    // Set tmp image to the complement of org
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            setUint8Pixel(tmp, x, y, 1 - getUint8Pixel(src, x, y));
        }
    }

    erosion(org, dst, m1, 3);
    erosion(tmp, org, m2, 3);

    // Calculate the intersection
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            setUint8Pixel(dst, x, y, getUint8Pixel(dst, x, y) & getUint8Pixel(org, x, y));
        }
    }

    // Cleanup
    deleteUint8Image(org);
    deleteUint8Image(tmp);
}

/*!
 * \brief Change all of the object’s pixels to the background value, except
 * those pixels that lie on the object’s contour
 *
 * The contour width is determined by the structuring element \p mask.
 * The result is the eroded image subtracted from the original image or the
 * original image subtracted from the dilated image.
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void outline(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verifiy mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    erosion(src, dst, mask, n);

    // Loop all pixels
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            setUint8Pixel(dst, x, y, getUint8Pixel(src, x, y) - getUint8Pixel(dst, x, y));
        }
    }
}

/*!
 * \brief Removes all binary objects that are 4/8-connected to a border.
 *
 * \param[in]  src A pointer to the source image
 * \param[out] dst A pointer to the destination image
 * \param[in]  c   Connectivity defined by ::eConnected
 */
void removeBorderBlobsIterative(const image_t *src, image_t *dst, const eConnected c)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    copyUint8Image(src, dst);

    // Mark the border pixels that are set
    for (int32_t y = 0; y < dst->rows; y++)
    {
        if (getUint8Pixel(dst, 0, y) == 1)
        {
            setUint8Pixel(dst, 0, y, 2);
        }

        if (getUint8Pixel(dst, (dst->cols - 1), y) == 1)
        {
            setUint8Pixel(dst, (dst->cols - 1), y, 2);
        }
    }

    for (int32_t x = 0; x < dst->cols; x++)
    {
        if (getUint8Pixel(dst, x, 0) == 1)
        {
            setUint8Pixel(dst, x, 0, 2);
        }

        if (getUint8Pixel(dst, x, (dst->rows - 1)) == 1)
        {
            setUint8Pixel(dst, x, (dst->rows - 1), 2);
        }
    }

    uint32_t changes = 0;
    uint32_t iterations = 0;

    // Set all 1 pixels that are connected to an edge to the marker value (2)
    do
    {
        // Loop as long as there are changes
        changes = 0;

        // Pass through from left-top to right-bottom
        // Discard first row, last row, first column and last column
        for (int32_t y = 1; y < dst->rows - 1; y++)
        {
            for (int32_t x = 1; x < dst->cols - 1; x++)
            {
                // Does the pixel belong to a blob?
                if (getUint8Pixel(dst, x, y) == 1)
                {
                    // Is a neighbour marked?
                    if (neighbourCount(dst, x, y, 2, c) > 0)
                    {
                        // Also mark this pixel
                        setUint8Pixel(dst, x, y, 2);
                        changes = 1;
                    }
                }
            }
        }

        iterations++;
    } while (changes != 0);

    // Set marked value to background value
    for (int32_t y = 0; y < src->rows; y++)
    {
        for (int32_t x = 0; x < src->cols; x++)
        {
            if (getUint8Pixel(dst, x, y) == 2)
            {
                setUint8Pixel(dst, x, y, 0);
            }
        }
    }
}

uint32_t find(uint32_t *lut, uint32_t i)
{
    while (lut[i] != i)
    {
        i = lut[i];
    }
    return i;
}

/*!
 * \brief Removes all binary objects that are 4/8-connected to a border.
 *
 * \param[in]  src     A pointer to the source image
 * \param[out] dst     A pointer to the destination image
 * \param[in]  c       Connectivity defined by ::eConnected
 * \param[in]  lutSize The maximum number of labels to find. Is used to
 *                     dynamically allocate memory for the lookup table. A
 *                     lower value is faster, but is also able to find fewer
 *                     labels. The value is application dependent.
 *
 * \return 0 Failure
 *           \li Memory allocation failed
 *           \li Lookup table is too small
 *         1 Success
 *
 */
uint32_t removeBorderBlobsTwoPass(const image_t *src, image_t *dst,
                                  const eConnected connected,
                                  const uint32_t lutSize)
{
    uint32_t *lut = (uint32_t *)malloc(lutSize * sizeof(uint32_t));

    if (lut == NULL)
    {
        return 0;
    }

    memset(lut, 0, lutSize * sizeof(uint32_t));

    lut[1] = 1;
    lut[2] = 2;

    uint32_t nextLabel = 3;

    // Mark the border pixels in the destination
    MarkBorderPixels(src, dst);

    // Pass 1: Label the objects, skipping the borders, and record equivalences in lut
    for (int32_t y = 1; y < src->rows - 1; y++)
    {
        for (int32_t x = 1; x < src->cols - 1; x++)
        {
            uint32_t idx = y * src->cols + x;

            // if object
            if (src->data[idx] != 0)
            {
                /*
                    1 1 1
                    1 0 0
                    0 0 0
                */
                uint32_t n = dst->data[idx - src->cols];
                uint32_t w = dst->data[idx - 1];
                uint32_t nw = 0;
                uint32_t ne = 0;

                // Alleen diagonalen meenemen als we 8-connected gebruiken
                if (connected == CONNECTED_EIGHT)
                {
                    nw = dst->data[idx - src->cols - 1];
                    ne = dst->data[idx - src->cols + 1];
                }

                // if neighbours labelled
                if (n != 0 || nw != 0 || ne != 0 || w != 0)
                {
                    // 1. Label this pixel with the lowest value of neighbor
                    uint32_t minLabel = UINT32_MAX;

                    if (n != 0 && n < minLabel)
                    {
                        minLabel = n;
                    }

                    if (nw != 0 && nw < minLabel)
                    {
                        minLabel = nw;
                    }

                    if (ne != 0 && ne < minLabel)
                    {
                        minLabel = ne;
                    }

                    if (w != 0 && w < minLabel)
                    {
                        minLabel = w;
                    }

                    // 2. Label this pixel with that value
                    dst->data[idx] = minLabel;

                    // record equivalence(s)
                    if (n != 0 && n != minLabel)
                    {
                        lut[n] = minLabel;
                    }

                    if (nw != 0 && nw != minLabel)
                    {
                        lut[nw] = minLabel;
                    }

                    if (ne != 0 && ne != minLabel)
                    {
                        lut[ne] = minLabel;
                    }

                    if (w != 0 && w != minLabel)
                    {
                        lut[w] = minLabel;
                    }
                }
                else
                {
                    if (nextLabel >= lutSize)
                    {
                        free(lut);
                        return 0; // Lookup table is too small
                    }

                    // Assign next label value
                    dst->data[idx] = nextLabel;

                    // record in lut
                    lut[nextLabel] = nextLabel;
                    nextLabel++;
                }
            }
            else
            {
                // label this pixel with background value
                dst->data[idx] = 0;
            }
        }
    }

    RecordBorderEquivalences(dst, lut, connected, src);

    ResolveEquivalences(nextLabel, lut);

    SecondPass(dst, lut);

    free(lut);

    return 1;
}

void RecordBorderEquivalences(image_t *dst, uint32_t *lut, const eConnected connected, const image_t *src)
{
    for (int32_t y = 1; y < dst->rows - 1; y++)
    {
        for (int32_t x = 1; x < dst->cols - 1; x++)
        {
            uint32_t idx = y * dst->cols + x;
            uint32_t currentLabel = dst->data[idx];

            // if object
            if (currentLabel != 0)
            {
                // if col 2
                if (x == 1)
                {
                    // check pixels links
                    uint32_t left = dst->data[idx - 1];

                    // als waarde = 2
                    if (left == 2)
                    {
                        linkLabelToBorder(lut, currentLabel);
                    }

                    if (connected == CONNECTED_EIGHT)
                    {
                        uint32_t bottom_left = dst->data[idx + src->cols - 1];

                        if (bottom_left == 2)
                        {
                            linkLabelToBorder(lut, currentLabel);
                        }
                    }
                }

                // if col max - 1
                if (x == dst->cols - 2)
                {
                    // check pixels rechts
                    uint32_t right = dst->data[idx + 1];

                    // als waarde = 2
                    if (right == 2)
                    {
                        linkLabelToBorder(lut, currentLabel);
                    }
                }

                // if row max - 1
                if (y == dst->rows - 2)
                {
                    // check pixels onder
                    uint32_t bottom = dst->data[idx + dst->cols];

                    // als waarde = 2
                    if (bottom == 2)
                    {
                        linkLabelToBorder(lut, currentLabel);
                    }

                    if (connected == CONNECTED_EIGHT)
                    {
                        uint32_t bottom_left = dst->data[idx + dst->cols - 1];

                        if (bottom_left == 2)
                        {
                            linkLabelToBorder(lut, currentLabel);
                        }

                        uint32_t bottom_right = dst->data[idx + dst->cols + 1];

                        if (bottom_right == 2)
                        {
                            linkLabelToBorder(lut, currentLabel);
                        }
                    }
                }
            }
        }
    }
}

void SecondPass(image_t *dst, uint32_t *lut)
{
    for (uint32_t y = 0; y < dst->rows; y++)
    {
        for (uint32_t x = 0; x < dst->cols; x++)
        {
            uint32_t idx = y * dst->cols + x;
            uint32_t currentLabel = dst->data[idx];

            // 1. If background, keep it background
            if (currentLabel != 0)
            {
                // Label is equivalent to 2 according to lut?
                if (lut[currentLabel] == 2)
                {
                    // Yes: Set to 0
                    dst->data[idx] = 0;
                }
                else
                {
                    // No: Set to 1
                    dst->data[idx] = 1;
                }
            }
        }
    }
}

void ResolveEquivalences(uint32_t nextLabel, uint32_t *lut)
{
    for (uint32_t i = 3; i < nextLabel; i++)
    {
        if (lut[i] != i)
        {
            lut[i] = lut[lut[i]];
        }
    }
}

void linkLabelToBorder(uint32_t *lut, uint32_t currentLabel)
{
    if (currentLabel == 0)
        return;

    uint32_t root = currentLabel;

    while (lut[root] != root && root != 2)
    {
        root = lut[root];
    }

    lut[root] = 2;

    lut[currentLabel] = 2;
}

void MarkBorderPixels(const image_t *src, image_t *dst)
{
    for (uint32_t y = 0; y < src->rows; y++)
    {
        for (uint32_t x = 0; x < src->cols; x++)
        {
            uint32_t idx = y * src->cols + x;

            // if on border
            if (x == 0 || x == src->cols - 1 || y == 0 || y == src->rows - 1)
            {
                // if object
                if (src->data[idx] != 0)
                {
                    // set in destination 2
                    dst->data[idx] = 2;
                }

                // if background
                if (src->data[idx] == 0)
                {
                    // set in destination 0
                    dst->data[idx] = 0;
                }
            }
        }
    }
}

/*!
 * \brief Defines a unique compressed geometrical representation of an object.
 *
 * Binary skeleton is defined as the union of the set of pixels computed from
 * the difference of the n_th eroded image and the opening of the n_th
 * eroded image.
 * The number of erosions n required by the skeleton algorithm is the number
 * of erosions of the original image A by the structuring function B that
 * yields the null image.
 * The function does not necessarily produce a fully connected object.
 *
 * \param[in]  src  A pointer to the source image
 * \param[out] dst  A pointer to the destination image
 * \param[in]  mask A pointer to a square mask of size \p n
 * \param[in]  n    The size of the mask
 */
void skeleton(const image_t *src, image_t *dst, const uint8_t *mask, const uint8_t n)
{
    // Verify image validity
    ASSERT(src == NULL, "src image is invalid");
    ASSERT(dst == NULL, "dst image is invalid");
    ASSERT(src->data == NULL, "src data is invalid");
    ASSERT(dst->data == NULL, "dst data is invalid");
    ASSERT(src->type != IMGTYPE_UINT8, "src type is invalid");
    ASSERT(dst->type != IMGTYPE_UINT8, "dst type is invalid");

    // Verify mask validity
    ASSERT(mask == NULL, "mask is invalid");
    ASSERT(n == 0, "mask size can not be equal to 0");

    // Verify image consistency
    ASSERT(src == dst, "src and dst are the same images");
    ASSERT(src->cols != dst->cols, "src and dst have different number of columns");
    ASSERT(src->rows != dst->rows, "src and dst have different number of rows");

    clearUint8Image(dst);

    // Create temporary images
    image_t *org = newUint8Image(src->cols, src->rows);
    image_t *eroded = newUint8Image(src->cols, src->rows);
    image_t *opened = newUint8Image(src->cols, src->rows);

    ASSERT(opened == NULL, "not enough memory for image allocation");

    copyUint8Image(src, org);

    // Loop as long as the original image has not been fully eroded
    uint8_t changes = 1;
    while (changes)
    {
        changes = 0;

        // eroded = erode_n(org)
        erosion(org, eroded, mask, n);

        // Opening: an erosion followed by dilation
        // opened = open(erode_n(org), mask)
        dilation(eroded, opened, mask, n);

        // Loop all pixels, skip the border
        for (int32_t y = n / 2; y < org->rows - (n / 2); y++)
        {
            for (int32_t x = (n / 2); x < org->cols - (n / 2); x++)
            {
                // Calculate Kn(A): erode_n-1(org) - open(erode_n(org), mask)
                uint8_pixel_t p = getUint8Pixel(org, x, y) - getUint8Pixel(opened, x, y);

                // Create skeleton by the union of Kn(A) of all erosions
                setUint8Pixel(dst, x, y, getUint8Pixel(dst, x, y) | p);

                // Copy eroded image to original image
                setUint8Pixel(org, x, y, getUint8Pixel(eroded, x, y));

                // Continue as long as the original image has not yet been fully eroded
                if (getUint8Pixel(org, x, y) == 1)
                    changes = 1;
            }
        }
    }

    // Cleanup
    deleteUint8Image(org);
    deleteUint8Image(eroded);
    deleteUint8Image(opened);
}
