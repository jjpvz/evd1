/*! ***************************************************************************
 *
 * \brief     Coding and compression algorithms
 * \file      coding_and_compression.c
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
#include "coding_and_compression.h"
#include "string.h"
#include "histogram_operations.h"

Node *build_huffman_tree(image_t *image)
{
    uint32_t hist[256];
    histogram(image, hist);

    int num_nodes = 0;

    // Count unique gray-values
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] > 0)
        {
            num_nodes++;
        }
    }

    // Allocate array of pointers to nodes
    Node **nodes = malloc(num_nodes * sizeof(Node *));

    int idx = 0;

    // Create a node for each unique gray-value
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] > 0)
        {
            nodes[idx] = malloc(sizeof(Node));
            nodes[idx]->value = i;
            nodes[idx]->freq = hist[i];
            nodes[idx]->left = NULL;
            nodes[idx]->right = NULL;
            idx++;
        }
    }

    int nodes_remaining = num_nodes;

    while (nodes_remaining > 1)
    {
        // Find two nodes with smallest frequency
        int lowest_freq_idx = -1;
        int second_lowest_freq_idx = -1;

        for (int i = 0; i < nodes_remaining; i++)
        {
            if (lowest_freq_idx == -1 || nodes[i]->freq < nodes[lowest_freq_idx]->freq)
            {
                second_lowest_freq_idx = lowest_freq_idx;
                lowest_freq_idx = i;
            }
            else if (second_lowest_freq_idx == -1 || nodes[i]->freq < nodes[second_lowest_freq_idx]->freq)
            {
                second_lowest_freq_idx = i;
            }
        }

        // Combine them into a new linking node
        Node *linking_node = malloc(sizeof(Node));

        linking_node->value = -1;
        linking_node->freq = nodes[lowest_freq_idx]->freq + nodes[second_lowest_freq_idx]->freq;
        linking_node->left = nodes[lowest_freq_idx];
        linking_node->right = nodes[second_lowest_freq_idx];

        // Replace them with the new linking node
        if (lowest_freq_idx > second_lowest_freq_idx)
        {
            int temp = lowest_freq_idx;
            lowest_freq_idx = second_lowest_freq_idx;
            second_lowest_freq_idx = temp;
        }

        nodes[lowest_freq_idx] = linking_node;
        nodes[second_lowest_freq_idx] = nodes[nodes_remaining - 1];

        nodes_remaining--;
    }

    Node *root = nodes[0];

    free(nodes);

    return root;
}

typedef struct
{
    int code[256]; // max depth <= 256
    int length;
} HuffmanCode;

void build_huffman_table(Node *root, int arr[], int top, HuffmanCode table[])
{
    if (!root)
        return;

    if (root->left)
    {
        arr[top] = 0;
        build_huffman_table(root->left, arr, top + 1, table);
    }

    if (root->right)
    {
        arr[top] = 1;
        build_huffman_table(root->right, arr, top + 1, table);
    }

    if (!root->left && !root->right)
    {
        for (int i = 0; i < top; i++)
            table[root->value].code[i] = arr[i];
        table[root->value].length = top;
    }
}

void print_byte_bits(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
        printf("%d", (byte >> i) & 1);
}

uint8_t *encode_image(image_t *image, Node *root)
{
    int arr[256] = {0};
    HuffmanCode table[256] = {0};

    // build code table once
    build_huffman_table(root, arr, 0, table);

    uint8_t first = image->data[0];

    printf("Original: ");
    print_byte_bits(first);
    printf("\n");

    printf("Encoded: ");
    for (int j = 0; j < table[first].length; j++)
        printf("%d", table[first].code[j]);
    printf("\n");

    return NULL;
}

// onsucces return pointer naar plaatje in geheugen
// onfail nullpointer geen geheugen gealloceerd dus iets fout gegaan

// bool decode_image(uint8_t *compressed_bits, HuffmanTree *tree, image_t *output_image)
// {
//     // read compressed_bits
//     // - navigate through tree
//     // - stop when node with value is found
//     // - repeat

//     // check of dit fout kan gaan
// }