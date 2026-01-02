/*! ***************************************************************************
 *
 * \brief     Coding and compression algorithms
 * \file      coding_and_compression.h
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      October 2024
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

#ifdef __cplusplus
extern "C"
{
#endif

/// Include guard to prevent recursive inclusion
#ifndef _CODING_AND_COMPRESSION_H_
#define _CODING_AND_COMPRESSION_H_

#include "image.h"

    // Functions are documented in the source file
    typedef struct TreeNode
    {
        int value;
        uint32_t freq;
        struct TreeNode *left;
        struct TreeNode *right;
    } TreeNode;

    typedef struct LinkedListNode
    {
        void *value;
        struct LinkedListNode *next;
    } LinkedListNode;

    typedef struct
    {
        uint64_t code;
        uint8_t length;
    } HuffmanCode;

    int compare_tree_nodes(const void *p1, const void *p2);
    void destroy_node(void *p);
    LinkedListNode *pq_enqueue(LinkedListNode **a_head, void *a_value, int (*cmp_fn)(const void *, const void *));
    LinkedListNode *pq_dequeue(LinkedListNode **a_head);
    void destroy_list(LinkedListNode **a_head, void (*destroy_fn)(void *));
    LinkedListNode *make_huffman_pq(uint32_t hist[256]);
    TreeNode *make_huffman_tree(LinkedListNode *head);
    void destroy_huffman_tree(TreeNode **a_root);
    void build_huffman_table(TreeNode *root, int arr[], int top, HuffmanCode table[]);
    uint8_t *encode_image(image_t *image, TreeNode *root, size_t *out_size);
    void decode_image(const uint8_t *encoded, size_t encoded_size, TreeNode *root, image_t *dst);

#endif // _CODING_AND_COMPRESSION_H_

#ifdef __cplusplus
}
#endif
