/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \internal
/// \brief Hashing container
///


#ifndef __HASH_H
#define __HASH_H 

#include "RakAssert.h"
#include <string.h> // memmove
#include "Export.h"
#include "RakString.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
namespace DataStructures
{
    struct HashIndex
    {
        size_t primaryIndex;
        size_t secondaryIndex;
        bool IsInvalid(void) const {return primaryIndex==(size_t) -1;}
        void SetInvalid(void) {primaryIndex=(size_t) -1; secondaryIndex=(size_t) -1;}
    };

    /// \brief Using a string as a identifier for a node, store an allocated pointer to that node
    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    class RAK_DLL_EXPORT Hash
    {
    public:
        /// Default constructor
        Hash();

        // Destructor
        ~Hash();

        void Push(key_type key, const data_type &input );
        data_type* Peek(key_type key );
        bool Pop(data_type& out, key_type key );
        bool RemoveAtIndex(HashIndex index );
        bool Remove(key_type key );
        HashIndex GetIndexOf(key_type key);
        bool HasData(key_type key);
        data_type& ItemAtIndex(const HashIndex &index);
        key_type  KeyAtIndex(const HashIndex &index);
        void GetAsList(DataStructures::List<data_type> &itemList,DataStructures::List<key_type > &keyList) const;
        size_t Size(void) const;

        /// \brief Clear the list
        void Clear(  );

        struct Node
        {
            Node(key_type strIn, const data_type &_data) {string=strIn; data=_data;}
            key_type  string;
            data_type data;
            // Next in the list for this key
            Node *next;
        };

    protected:
        void ClearIndex(size_t index);
        Node **nodeList;
        size_t size;
    };

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    Hash<key_type, data_type, HASH_SIZE, hashFunction>::Hash()
    {
        nodeList=0;
        size=0;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    Hash<key_type, data_type, HASH_SIZE, hashFunction>::~Hash()
    {
        Clear();
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    void Hash<key_type, data_type, HASH_SIZE, hashFunction>::Push(key_type key, const data_type &input )
    {
        unsigned long hashIndex = (*hashFunction)(key) % HASH_SIZE;
        if (nodeList==0)
        {
            nodeList = new Node *[HASH_SIZE];
            memset(nodeList,0,sizeof(Node *)*HASH_SIZE);
        }

        Node *newNode= new Node(key, input);
        newNode->next=nodeList[hashIndex];
        nodeList[hashIndex]=newNode;

        size++;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    data_type* Hash<key_type, data_type, HASH_SIZE, hashFunction>::Peek(key_type key )
    {
        if (nodeList==0)
            return 0;

        unsigned long hashIndex = (*hashFunction)(key) % HASH_SIZE;
        Node *node = nodeList[hashIndex];
        while (node!=0)
        {
            if (node->string==key)
                return &node->data;
            node=node->next;
        }
        return 0;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    bool Hash<key_type, data_type, HASH_SIZE, hashFunction>::Pop(data_type& out, key_type key )
    {
        if (nodeList==0)
            return false;

        unsigned long hashIndex = (*hashFunction)(key) % HASH_SIZE;
        Node *node = nodeList[hashIndex];
        if (node==0)
            return false;
        if (node->next==0)
        {
            // Only one item.
            if (node->string==key)
            {
                // Delete last item
                out=node->data;
                ClearIndex(hashIndex);
                return true;
            }
            else
            {
                // Single item doesn't match
                return false;
            }
        }
        else if (node->string==key)
        {
            // First item does match, but more than one item
            out=node->data;
            nodeList[hashIndex]=node->next;
            delete node;
            size--;
            return true;
        }

        Node *last=node;
        node=node->next;

        while (node!=0)
        {
            // First item does not match, but subsequent item might
            if (node->string==key)
            {
                out=node->data;
                // Skip over subsequent item
                last->next=node->next;
                // Delete existing item
                delete node;
                size--;
                return true;
            }
            last=node;
            node=node->next;
        }
        return false;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    bool Hash<key_type, data_type, HASH_SIZE, hashFunction>::RemoveAtIndex(HashIndex index )
    {
        if (index.IsInvalid())
            return false;

        Node *node = nodeList[index.primaryIndex];
        if (node==0)
            return false;
        if (node->next==0)
        {
            // Delete last item
            ClearIndex(index.primaryIndex);
            return true;
        }
        else if (index.secondaryIndex==0)
        {
            // First item does match, but more than one item
            nodeList[index.primaryIndex]=node->next;
            delete node;
            size--;
            return true;
        }

        Node *last=node;
        node=node->next;
        --index.secondaryIndex;

        while (index.secondaryIndex!=0)
        {
            last=node;
            node=node->next;
            --index.secondaryIndex;
        }

        // Skip over subsequent item
        last->next=node->next;
        // Delete existing item
        delete node;
        size--;
        return true;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    bool Hash<key_type, data_type, HASH_SIZE, hashFunction>::Remove(key_type key )
    {
        return RemoveAtIndex(GetIndexOf(key));
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    HashIndex Hash<key_type, data_type, HASH_SIZE, hashFunction>::GetIndexOf(key_type key)
    {
        if (nodeList==0)
        {
            HashIndex temp;
            temp.SetInvalid();
            return temp;
        }
        HashIndex idx;
        idx.primaryIndex=(*hashFunction)(key) % HASH_SIZE;
        Node *node = nodeList[idx.primaryIndex];
        if (node==0)
        {
            idx.SetInvalid();
            return idx;
        }
        idx.secondaryIndex=0;
        while (node!=0)
        {
            if (node->string==key)
            {
                return idx;
            }
            node=node->next;
            idx.secondaryIndex++;
        }

        idx.SetInvalid();
        return idx;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    bool Hash<key_type, data_type, HASH_SIZE, hashFunction>::HasData(key_type key)
    {
        return GetIndexOf(key).IsInvalid()==false;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    data_type& Hash<key_type, data_type, HASH_SIZE, hashFunction>::ItemAtIndex(const HashIndex &index)
    {
        Node *node = nodeList[index.primaryIndex];
        RakAssert(node);
        for (size_t i=0; i < index.secondaryIndex; i++)
        {
            node=node->next;
            RakAssert(node);
        }
        return node->data;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    key_type  Hash<key_type, data_type, HASH_SIZE, hashFunction>::KeyAtIndex(const HashIndex &index)
    {
        Node *node = nodeList[index.primaryIndex];
        RakAssert(node);
        for (size_t i=0; i < index.secondaryIndex; i++)
        {
            node=node->next;
            RakAssert(node);
        }
        return node->string;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    void Hash<key_type, data_type, HASH_SIZE, hashFunction>::Clear()
    {
        if (nodeList)
        {
            for (size_t i=0; i < HASH_SIZE; i++)
                ClearIndex(i);
            delete[] nodeList;
            nodeList=0;
            size=0;
        }
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    void Hash<key_type, data_type, HASH_SIZE, hashFunction>::ClearIndex(size_t index)
    {
        Node *node = nodeList[index];
        Node *next;
        while (node)
        {
            next=node->next;
            delete node;
            node=next;
            size--;
        }
        nodeList[index]=0;
    }

    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    void Hash<key_type, data_type, HASH_SIZE, hashFunction>::GetAsList(DataStructures::List<data_type> &itemList,DataStructures::List<key_type > &keyList) const
    {
        if (nodeList==0)
            return;
        itemList.Clear(false);
        keyList.Clear(false);

        for (size_t i=0; i < HASH_SIZE; i++)
        {
            if (nodeList[i])
            {
                Node *node=nodeList[i];
                while (node)
                {
                    itemList.Push(node->data);
                    keyList.Push(node->string);
                    node=node->next;
                }
            }
        }
    }
    template <class key_type, class data_type, size_t HASH_SIZE, unsigned long (*hashFunction)(const key_type &) >
    size_t Hash<key_type, data_type, HASH_SIZE, hashFunction>::Size(void) const
    {
        return size;
    }
}
#endif
