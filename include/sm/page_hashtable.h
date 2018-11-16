/*
 * File: page_hashtable.h
 * Description: buffer manager interface
 * Author: pinyin of your name
 * E-mail:
 *
 */
#pragma once

// 
// Page hash node
//
struct PageHashNode
{
    PageHashNode *next;
    PageHashNode *prev;
    int fd;
    int page_id;
    int slot;
    PageHashNode(int f, int p, int s, PageHashNode *n = NULL, PageHashNode *pr = NULL)
        : fd(f), page_id(p), slot(s), next(n), prev(pr) {}
};

//
// PageHashTable
// use separate chaining method
//
class PageHashTable
{
    private:
        int hash(int fd, int page_id) const;
        int table_size;
        PageHashNode **hash_table;
    public:
        PageHashTable(int ts = 20);
        ~PageHashTable();
        bool search(int fd, int page_id, int &slot);
        bool insert(int fd, int page_id, int slot);
        bool remove(int fd, int page_id);
};
