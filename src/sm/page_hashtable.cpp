/*
 * File: page_hashtable.cpp
 * Description: buffer manager implemention
 * Author: pinyin of your name
 * E-mail:
 *
 */
#include <iostream>
#include "sm/page_hashtable.h"

using std::cout;
using std::endl;

//////////////////////////////////////////////////////////////
// Function: constructor
// Description: init PageHashTable
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
PageHashTable::PageHashTable(int ts)
{
    table_size = ts;
    // allocate memory for page hash table
    hash_table = new PageHashNode*[table_size];
    // init hash table with NULL
    for (int i = 0; i < table_size; i++)
        hash_table[i] = NULL;
}

//////////////////////////////////////////////////////////////
// Function: deconstructor
// Description: free PageHashTable
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
PageHashTable::~PageHashTable()
{
    // free hashtable
    for (int i = 0; i < table_size; i++)
    {
        PageHashNode *cur = hash_table[i];
        while(cur != NULL)
        {
            PageHashNode *next = cur->next;
            delete cur;
            cur = next;
        }
    }
}

//////////////////////////////////////////////////////////////
// Function: search
// Description: search a hash node by fd and page id
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool PageHashTable::search(int fd, int page_id, int &slot)
{
    // hash search
    int bucket = hash(fd, page_id);

    if (bucket < 0)
        return false;

    for (PageHashNode *cur = hash_table[bucket]; cur != NULL; cur = cur->next)
    {
        if (cur->fd == fd && cur->page_id == page_id)
        {
            slot = cur->slot;
            return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////
// Function: search
// Description: search a hash node by fd and page id
// Author: Liu Chaoyang
// E-mail: chaoyanglius@gmail.com
//////////////////////////////////////////////////////////////
bool PageHashTable::insert(int fd, int page_id, int slot)
{
    int bucket = hash(fd, page_id);

    // check if page node exists
    for (PageHashNode *cur = hash_table[bucket]; cur != NULL; cur = cur->next)
    {
        if (cur->fd == fd && cur->page_id == page_id)
        {
            slot = cur->slot;
            cout << "Error: insert failed because of page has existed!" << endl;
            return false;
        }
    }
    
    PageHashNode *node = new PageHashNode(fd, page_id, slot, hash_table[bucket]);

    if (hash_table[bucket] != NULL)
        hash_table[bucket]->prev = node;

    hash_table[bucket] = node;

    return true;
}

bool PageHashTable::remove(int fd, int page_id)
{
    int bucket = hash(fd, page_id);

    PageHashNode *node;
    // check if node is in the bucket
    for (node = hash_table[bucket]; node != NULL; node = node->next)
    {
        if (node->fd == fd && node->page_id == page_id)
        {
            break;
        }
    }

    if (node == NULL)
    {
        cout << "Error: page not found!" << endl;
        return false;
    }
    
    // remove this page node
    if(node == hash_table[bucket])
        hash_table[bucket] = node->next;
    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    delete node;

    return true;
}

int PageHashTable::hash(int fd, int page_id) const
{
    return ((fd + page_id) % table_size);
}
