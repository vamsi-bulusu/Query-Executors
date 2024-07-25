//===----------------------------------------------------------------------===//
//
//                         Rutgers CS539 - Database System
//                         ***DO NO SHARE PUBLICLY***
//
// Identification:   include/b_plus_tree.h
//
// Copyright (c) 2023, Rutgers University
//
//===----------------------------------------------------------------------===//
#pragma once

#include <queue>
#include <string>
#include <vector>
#include "para.h"
#include <stack>

using namespace std;

// Value structure we insert into BPlusTree
struct RecordPointer {
  int page_id;
  int record_id;
  RecordPointer() : page_id(0), record_id(0){};
  RecordPointer(int page, int record) : page_id(page), record_id(record){};
  bool operator==(const RecordPointer& other) const {
    return page_id == other.page_id && record_id == other.record_id;
  }
};

// BPlusTree Node
class Node {
public:
  Node(bool leaf) : key_num(0), is_leaf(leaf), parent(NULL){};
  bool is_leaf;
  int key_num;
  KeyType keys[MAX_FANOUT - 1];
  Node *parent;
};

// internal b+ tree node
class InternalNode : public Node {
public:
  InternalNode() : Node(false) {};
  Node * children[MAX_FANOUT];
};

class LeafNode : public Node {
public:
  LeafNode() : Node(true) {};
  RecordPointer pointers[MAX_FANOUT - 1];
  // pointer to the next/prev leaf node
  LeafNode *next_leaf = NULL;
  LeafNode *prev_leaf = NULL;
};


/**
 * Main class providing the API for the Interactive B+ Tree.
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain record pointers
 * (1) We only support (and test) UNIQUE key
 * (2) Support insert & remove
 * (3) Support range scan, return multiple values.
 * (4) The structure should shrink and grow dynamically
 */

class BPlusTree {
 public:
  BPlusTree(){
    root = NULL;
  };

  // Returns true if this B+ tree has no keys and values
  bool IsEmpty() const;

  // Insert a key-value pair into this B+ tree.
  bool Insert(const KeyType &key, const RecordPointer &value);

  // Remove a key and its value from this B+ tree.
  void Remove(const KeyType &key);

  // return the value associated with a given key
  bool GetValue(const KeyType &key, RecordPointer &result);

  // return the values within a key range [key_start, key_end) not included key_end
  void RangeScan(const KeyType &key_start, const KeyType &key_end,
                 std::vector<RecordPointer> &result);

  void shiftKeysAndPointers(LeafNode* leaf, int startIndex);
  
  void insertKeyAndPointer(LeafNode* leaf, int index, const KeyType &key, const RecordPointer &value);
  
  void insertAtLeaf(LeafNode* leaf, const KeyType &key, const RecordPointer &value, int splitIndex);

  int GetInsertIndex(Node *node, const KeyType &key);

  void backPropagateInternal(const KeyType &key, InternalNode *parent, InternalNode *newLeaf); 
  
  void removeParent(Node* remNode,int index, InternalNode* parent);

  LeafNode* getLeaf(Node* root, const KeyType &key);
  Node *root;
};
