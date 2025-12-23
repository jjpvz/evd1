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

// void huffman_clean_up(Node *nodes[])
// {
//     for (int i = 0; i < sizeof(nodes); i++)
//     {
//         free(nodes[i]);
//     }

//     free(nodes);
// }

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
    // Init length of address with sizeof void pointer (NULL)
    Node **nodes = malloc(num_nodes * sizeof(NULL));

    // if (nodes == NULL)
    // {
    //     return 0;
    // };

    int idx = 0;

    // Create a node for each unique gray-value
    for (int i = 0; i < 256; i++)
    {
        if (hist[i] > 0)
        {
            nodes[idx] = malloc(sizeof(Node));

            // if (nodes[idx] == NULL)
            // {
            //     huffman_clean_up(nodes);
            //     return 0;
            // }

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

        // if (linking_node == NULL)
        // {
        //     huffman_clean_up(nodes);
        //     return 0;
        // }

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

    // huffman_clean_up(nodes);

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
    // for (int i = 7; i >= 0; i--)
    //     printf("%d", (byte >> i) & 1);
}

uint8_t *encode_image(image_t *image, Node *root, size_t *out_size)
{
    HuffmanCode table[256] = {0};
    int arr[256];
    build_huffman_table(root, arr, 0, table);

    size_t capacity = 1;
    uint8_t *output = malloc(capacity);
    output[0] = 0;

    size_t byte_index = 0;
    int bits_filled = 0;

    for (size_t i = 0; i < image->rows * image->cols; i++)
    {
        uint8_t pixel = image->data[i];
        HuffmanCode *code = &table[pixel];

        for (int b = 0; b < code->length; b++)
        {
            // shift left to make space
            output[byte_index] <<= 1;

            // add new bit
            output[byte_index] |= code->code[b];

            bits_filled++;

            if (bits_filled == 8)
            {
                bits_filled = 0;
                byte_index++;

                if (byte_index >= capacity)
                {
                    capacity *= 2;
                    output = realloc(output, capacity);
                }

                output[byte_index] = 0;
            }
        }
    }

    // pad last byte with zeros if not full
    if (bits_filled > 0)
    {
        output[byte_index] <<= (8 - bits_filled);
        byte_index++;
    }

    *out_size = byte_index;
    return output;
}

void decode_image(
    const uint8_t *encoded,
    size_t encoded_size,
    Node *root,
    image_t *dst)
{
    size_t pixel_index = 0;
    Node *current = root;

    for (size_t byte_index = 0; byte_index < encoded_size; byte_index++)
    {
        uint8_t byte = encoded[byte_index];

        // MSB → LSB
        for (int bit = 7; bit >= 0; bit--)
        {
            int b = (byte >> bit) & 1;

            // Walk the tree
            if (b == 0)
                current = current->left;
            else
                current = current->right;

            // Leaf reached?
            if (!current->left && !current->right)
            {
                dst->data[pixel_index++] = (uint8_pixel_t)current->value;
                current = root;

                // Stop exactly after all pixels
                if (pixel_index == dst->rows * dst->cols)
                    return;
            }
        }
    }
}
