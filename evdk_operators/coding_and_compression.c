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

int compare_tree_nodes(const void *p1, const void *p2)
{
    const TreeNode *t1 = (const TreeNode *)p1;
    const TreeNode *t2 = (const TreeNode *)p2;

    if (t1->freq != t2->freq)
    {
        return t1->freq - t2->freq;
    }

    return t1->value - t2->value;
}

void destroy_node(void *p)
{
    free(p);
}

LinkedListNode *pq_enqueue(LinkedListNode **a_head, void *a_value, int (*cmp_fn)(const void *, const void *))
{
    LinkedListNode *new_node = malloc(sizeof(*new_node));

    if (new_node == NULL)
    {
        return NULL;
    }

    new_node->value = a_value;
    new_node->next = NULL;

    // list is empty or new value is < the current head
    if (*a_head == NULL || cmp_fn(a_value, (*a_head)->value) <= 0)
    {
        new_node->next = *a_head;
        *a_head = new_node;
    }
    else
    {
        LinkedListNode *curr = *a_head;

        while (curr->next != NULL && cmp_fn(a_value, curr->next->value) > 0)
        {
            curr = curr->next;
        }

        // insert new_node just after curr
        new_node->next = curr->next;
        curr->next = new_node;
    }

    return new_node;
}

LinkedListNode *pq_dequeue(LinkedListNode **a_head)
{
    if (a_head == NULL || *a_head == NULL)
    {
        return NULL;
    }

    LinkedListNode *detached_node = *a_head;

    *a_head = detached_node->next;
    detached_node->next = NULL;

    return detached_node;
}

void destroy_list(LinkedListNode **a_head, void (*destroy_fn)(void *))
{
    while (*a_head != NULL)
    {
        LinkedListNode *detached_node = pq_dequeue(a_head);

        if (destroy_fn != NULL && detached_node->value != NULL)
        {
            destroy_fn(detached_node->value);
        }

        free(detached_node);
    }

    *a_head = NULL;
}

LinkedListNode *make_huffman_pq(uint32_t hist[256])
{
    LinkedListNode *head = NULL;

    for (int i = 0; i < 256; i++)
    {
        if (hist[i] > 0)
        {
            TreeNode *new_tree_node = malloc(sizeof(*new_tree_node));

            if (new_tree_node == NULL)
            {
                return NULL;
            }

            new_tree_node->value = i;
            new_tree_node->freq = hist[i];
            new_tree_node->left = NULL;
            new_tree_node->right = NULL;

            pq_enqueue(&head, new_tree_node, compare_tree_nodes);
        }
    }

    return head;
}

TreeNode *make_huffman_tree(LinkedListNode *head)
{
    if (head == NULL)
    {
        return NULL;
    }

    while (head->next != NULL)
    {
        LinkedListNode *wrapper1 = pq_dequeue(&head);
        LinkedListNode *wrapper2 = pq_dequeue(&head);

        TreeNode *left_child = (TreeNode *)wrapper1->value;
        TreeNode *right_child = (TreeNode *)wrapper2->value;

        TreeNode *parent = malloc(sizeof(*parent));

        if (parent == NULL)
        {
            return NULL;
        }

        parent->value = 0;
        parent->freq = left_child->freq + right_child->freq;
        parent->left = left_child;
        parent->right = right_child;

        free(wrapper1);
        free(wrapper2);

        pq_enqueue(&head, parent, compare_tree_nodes);
    }

    LinkedListNode *final_wrapper = pq_dequeue(&head);
    TreeNode *root = (TreeNode *)final_wrapper->value;

    free(final_wrapper);

    return root;
}

void destroy_huffman_tree(TreeNode **a_root)
{
    if (a_root == NULL || *a_root == NULL)
    {
        return;
    }

    TreeNode *root = *a_root;

    destroy_huffman_tree(&(root->left));
    destroy_huffman_tree(&(root->right));

    free(root);

    *a_root = NULL;
}

void build_huffman_table(TreeNode *root, int arr[], int top, HuffmanCode table[])
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

uint8_t *encode_image(image_t *image, TreeNode *root, size_t *out_size)
{
    HuffmanCode table[256] = {0};
    int arr[256];
    build_huffman_table(root, arr, 0, table);

    size_t capacity = (image->rows * image->cols) / 2 + 1;

    uint8_t *encoded_data = malloc(capacity);
    if (!encoded_data)
    {
        return NULL;
    }

    encoded_data[0] = 0;
    size_t byte_index = 0;
    int bits_filled = 0;

    for (size_t i = 0; i < image->rows * image->cols; i++)
    {
        uint8_t pixel = image->data[i];
        HuffmanCode *code = &table[pixel];

        for (int b = 0; b < code->length; b++)
        {
            // shift left to make space
            encoded_data[byte_index] <<= 1;

            // add new bit
            encoded_data[byte_index] |= code->code[b];

            bits_filled++;

            if (bits_filled == 8)
            {
                bits_filled = 0;
                byte_index++;

                if (byte_index >= capacity)
                {
                    capacity *= 2;

                    uint8_t *tmp = realloc(encoded_data, capacity);

                    if (!tmp)
                    {
                        free(encoded_data);
                        return NULL;
                    }

                    encoded_data = tmp;
                }

                encoded_data[byte_index] = 0;
            }
        }
    }

    // pad last byte with zeros if not full
    if (bits_filled > 0)
    {
        encoded_data[byte_index] <<= (8 - bits_filled);
        byte_index++;
    }

    *out_size = byte_index;
    return encoded_data;
}

void decode_image(
    const uint8_t *encoded,
    size_t encoded_size,
    TreeNode *root,
    image_t *dst)
{
    size_t pixel_index = 0;
    TreeNode *current = root;

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