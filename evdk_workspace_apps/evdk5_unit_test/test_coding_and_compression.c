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

void test_make_huffman_pq()
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

    // Build Huffman tree
    uint32_t hist[256];
    histogram(&src, hist);

    // Build the Priority Queue
    LinkedListNode *head = make_huffman_pq(hist);

    printf("Verifying Priority Queue order...\n");

    LinkedListNode *curr = head;
    uint32_t expected_freqs[] = {1, 2, 3, 3};
    uint8_t expected_values[] = {40, 10, 20, 30};

    for (int i = 0; i < 4; i++)
    {
        assert(curr != NULL);
        TreeNode *tn = (TreeNode *)curr->value;

        printf("Node %d: Value %u, Freq %u\n", i, tn->value, tn->freq);

        assert(tn->freq == expected_freqs[i]);
        // If your tie-breaker is based on value, 20 will come before 30.
        assert(tn->value == expected_values[i]);

        curr = curr->next;
    }

    // Verify list ends after 4 unique values
    assert(curr == NULL);

    printf("test_make_huffman_pq: PASSED\n");

    // Clean up
    destroy_list(&head, destroy_node);
}

void test_make_huffman_tree()
{
    printf("Running test_make_huffman_tree...\n");

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

    uint32_t hist[256];
    histogram(&src, hist);
    LinkedListNode *head = make_huffman_pq(hist);

    TreeNode *root = make_huffman_tree(head);

    // 3. Verify Root
    assert(root != NULL);
    printf("Root frequency: %u (Expected: 9)\n", root->freq);
    assert(root->freq == 9);

    // 4. Verify Structure (Walking the tree)
    if (root->left->value == 30)
    {
        printf("Left child is leaf 30\n");
        assert(root->left->freq == 3);
        assert(root->right->freq == 6);
    }
    else if (root->right->value == 30)
    {
        printf("Right child is leaf 30\n");
        assert(root->right->freq == 3);
        assert(root->left->freq == 6);
    }
    else
    {
        assert(0 && "Tree structure incorrect: 30 should be a direct child of root");
    }

    printf("test_make_huffman_tree: PASSED\n");

    // Clean up
    destroy_huffman_tree(&root);
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

    LinkedListNode *head = make_huffman_pq(hist);
    TreeNode *root = make_huffman_tree(head);

    // Encode image
    size_t encoded_size = 0;
    uint8_t *encoded = encode_image(&src, root, &encoded_size);

    // In our 3x3 case, 9 pixels * 8 bits = 72 bits (9 bytes)
    // The Huffman version should be roughly 21-25 bits (~3-4 bytes)
    printf("Original: 9 bytes | Encoded: %zu bytes\n", encoded_size);
    assert(encoded_size < 9);

    printf("\n--- Encoded Bitstream Inspection ---\n");
    for (size_t i = 0; i < encoded_size; i++)
    {
        printf("Byte %zu: Binary: ", i, encoded[i]);

        // Print bits from MSB (7) to LSB (0)
        for (int b = 7; b >= 0; b--)
        {
            printf("%d", (encoded[i] >> b) & 1);
        }
        printf("\n");
    }
    printf("------------------------------------\n");

    // Cleanup
    free(encoded);
    destroy_huffman_tree(&root);

    printf("test_encode_image passed\n");
}

void test_decode_image(void)
{
    printf("Running test_decode_image...\n");

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

    LinkedListNode *head = make_huffman_pq(hist);
    TreeNode *root = make_huffman_tree(head);

    // Encode image
    size_t encoded_size = 0;
    uint8_t *encoded = encode_image(&src, root, &encoded_size);

    // 4. Prepare Destination Image for Decoding
    image_t dst;
    dst.cols = src.cols;
    dst.rows = src.rows;
    dst.data = malloc(dst.cols * dst.rows * sizeof(uint8_pixel_t));
    memset(dst.data, 0, dst.cols * dst.rows); // Clear it first

    // 5. Run the Decode Function
    decode_image(encoded, encoded_size, root, &dst);

    // 6. Verification
    printf("Verifying pixels...\n");
    for (int i = 0; i < 9; i++)
    {
        printf("Pixel %d: Original %u, Decoded %u\n", i, src_data[i], dst.data[i]);
        assert(src_data[i] == dst.data[i]);
    }

    printf("test_decode_image: PASSED (Lossless reconstruction confirmed)\n");

    // Cleanup
    free(encoded);
    free(dst.data);
    destroy_huffman_tree(&root);
}