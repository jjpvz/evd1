/*! ***************************************************************************
 *
 * \brief     Unit test functions for coding and compression
 * \file      test_coding_and_compression.c
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      November 2024
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

#include "main.h"
#include "coding_and_compression.h"
#include <assert.h>

void test_huffman(void)
{
    // Test case 1: Simple image with a few gray values
    uint8_pixel_t src_data_test_case_01[12 * 8] =
        {
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
            10,
            10,
            10,
            10,
            20,
            20,
            20,
            20,
            30,
            30,
            30,
            30,
        };

    // Test case 2: Image with different frequencies
    uint8_pixel_t src_data_test_case_02[12 * 8] =
        {
            5,
            5,
            5,
            5,
            5,
            5,
            5,
            5,
            5,
            5,
            5,
            5, // 12 pixels with value 5
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15, // 4 more 5s, 6 with 10, 2 with 15
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
            5,
            5,
            5,
            5,
            10,
            10,
            10,
            10,
            10,
            10,
            15,
            15,
        };

    typedef struct testcase_t
    {
        uint8_pixel_t *src_data;
        int expected_num_nodes;
        int expected_values[256]; // Expected gray values
        int expected_freqs[256];  // Expected frequencies
        int total_pixels;         // Total number of pixels
    } testcase_t;

    // Compose array of test cases
    testcase_t testcases[] =
        {
            {src_data_test_case_01, 3, {10, 20, 30}, {32, 32, 32}, 96},
            {src_data_test_case_02, 3, {5, 10, 15}, {40, 42, 14}, 96},
        };

    // Prepare image
    image_t src = {12, 8, IMGTYPE_UINT8, NULL};

    // Loop all test cases
    for (uint32_t i = 0; i < (sizeof(testcases) / sizeof(testcase_t)); ++i)
    {
        // Set the data
        src.data = testcases[i].src_data;

        // Execute the operator
        TreeNode *root = build_huffman_tree(&src);

        // Set test case name
        char name[80] = "";
        sprintf(name, "Test case %d of %d", i + 1, (uint32_t)(sizeof(testcases) / sizeof(testcase_t)));

        // Verify tree structure
        int leaf_count = 0;
        int valid = verify_tree(root, testcases[i].total_pixels, &leaf_count);

#if 0
        // Print testcase info
        printf("\n---------------------------------------\n");
        printf("%s\n", name);
        printf("\nInput histogram:\n");
        for (int j = 0; j < testcases[i].expected_num_nodes; j++)
        {
            printf("  Value %3d: frequency %d\n",
                   testcases[i].expected_values[j],
                   testcases[i].expected_freqs[j]);
        }
        printf("  Total pixels: %d\n", testcases[i].total_pixels);

        printf("\nResulting Huffman tree structure:\n");
        print_tree(root, 0, "Root:");

        printf("\nTree validation: %s\n", valid ? "PASSED" : "FAILED");
        printf("Leaf nodes found: %d (expected: %d)\n",
               leaf_count, testcases[i].expected_num_nodes);

#endif

        // Verify the root
        TEST_ASSERT_NOT_NULL_MESSAGE(root, name);
        TEST_ASSERT_EQUAL_MESSAGE(testcases[i].total_pixels, root->freq, name);
        TEST_ASSERT_EQUAL_MESSAGE(testcases[i].expected_num_nodes, leaf_count, name);
        TEST_ASSERT_TRUE_MESSAGE(valid, name);

        // Clean up
        free_tree(root);
    }
}

void test_encode_image(void)
{
    // 3x3 test image
    uint8_pixel_t src_data[3 * 3] =
        {
            10, 10, 20,
            20, 20, 30,
            30, 30, 40};

    image_t src =
        {
            .cols = 3,
            .rows = 3,
            .type = IMGTYPE_UINT8,
            .data = src_data};

    size_t original_size = src.cols * src.rows * sizeof(uint8_pixel_t);

    printf("Original size: %zu bytes\n", original_size);

    // Build Huffman tree
    uint32_t hist[256];
    histogram(&src, hist);

    TreeNode *head = make_huffman_pq(hist);
    TreeNode *root = make_huffman_tree(head);

    // Encode image
    size_t encoded_size = 0;
    uint8_t *encoded = encode_image(&src, root, &encoded_size);

    // Clean up
    destroy_huffman_tree(root);

    // Sanity checks
    assert(encoded != NULL);
    assert(encoded_size > 0);

    printf("Encoded size: %zu byte(s)\n", encoded_size);

    // Print all encoded bytes bit-by-bit
    for (size_t i = 0; i < encoded_size; i++)
    {
        printf("byte %zu: ", i);
        print_byte_bits(encoded[i]);
        printf("\n");
    }

    printf("test_encode_image passed\n");
}

void test_decode_image(void)
{
    // Original image (3x3)
    uint8_pixel_t src_data[9] =
        {
            10, 10, 20,
            20, 20, 30,
            30, 30, 40};

    image_t src =
        {
            .rows = 3,
            .cols = 3,
            .type = IMGTYPE_UINT8,
            .data = src_data};

    // Build Huffman tree
    TreeNode *root = build_huffman_tree(&src);
    assert(root != NULL);

    // ---- ENCODE ----
    size_t encoded_size = 0;
    uint8_t *encoded = encode_image(&src, root, &encoded_size);

    assert(encoded != NULL);
    assert(encoded_size > 0);

    printf("Original size: %zu bytes\n", sizeof(src_data));
    printf("Encoded size : %zu bytes\n", encoded_size);

    // ---- DECODE ----
    uint8_pixel_t decoded_data[9] = {0};

    image_t dst =
        {
            .rows = 3,
            .cols = 3,
            .type = IMGTYPE_UINT8,
            .data = decoded_data};

    decode_image(encoded, encoded_size, root, &dst);

    // ---- VERIFY ----
    for (int i = 0; i < 9; i++)
    {
        assert(decoded_data[i] == src_data[i]);
    }

    // Cleanup
    free(encoded);
    free_tree(root);

    printf("test_encode_decode PASSED\n");
}