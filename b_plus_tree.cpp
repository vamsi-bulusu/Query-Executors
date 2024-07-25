#include "../include/b_plus_tree.h"
#include <iostream>
#include <math.h>
#include <stack>

/*
  Helper FUNCTIONS
*/

void shiftKeys(InternalNode *node, int startIndex) {
    for (int j = node->key_num; j > startIndex; j--) {
        node->keys[j] = node->keys[j - 1];
    }
}

void shiftChildPointers(InternalNode *node, int startIndex) {
    for (int j = node->key_num + 1; j > startIndex + 1; j--) {
        node->children[j] = (InternalNode*)node->children[j - 1];
    }
}

void insertKey(InternalNode *node, int insertIndex, const KeyType &key, InternalNode *newChild) {
    node->keys[insertIndex] = key;
    node->children[insertIndex + 1] = (InternalNode*)newChild;
    node->key_num++;
}

void insertKeyIntoInternalNode(InternalNode *node, const KeyType &key, InternalNode *newChild) {
    int insertIndex = 0;

    // Find the index to insert the key
    while (key > node->keys[insertIndex] && insertIndex < node->key_num) {
        insertIndex++;
    }

    shiftKeys(node, insertIndex);
    shiftChildPointers(node, insertIndex);

    insertKey(node, insertIndex, key, newChild);
}


int BPlusTree::GetInsertIndex(Node *node, const KeyType &key) {
  for(int i=0;i<node->key_num;i++) {
    if(node->keys[i] > key) {
        return i;
    }
    else if(node->keys[i] == key) {
        return i+1;
    }
  }
  return node->key_num;
}

LeafNode* BPlusTree::getLeaf(Node* root, const KeyType &key) {
    Node *current = root;
    while(!current->is_leaf){
        int splitIndex = GetInsertIndex(current, key);
        current = ((InternalNode *)current)->children[splitIndex];
    }
    return ((LeafNode *)current);
}

void BPlusTree::shiftKeysAndPointers(LeafNode* leaf, int startIndex) {
    for (int i = leaf->key_num - 1; i >= startIndex; i--) {
        leaf->keys[i + 1] = leaf->keys[i];
        ((LeafNode*)leaf)->pointers[i + 1] = ((LeafNode*)leaf)->pointers[i];
    }
}

void BPlusTree::insertKeyAndPointer(LeafNode* leaf, int index, const KeyType &key, const RecordPointer &value) {
    leaf->keys[index] = key;
    ((LeafNode*)leaf)->pointers[index] = value;
    leaf->key_num++;
}

void BPlusTree::insertAtLeaf(LeafNode* leaf, const KeyType &key, const RecordPointer &value, int splitIndex) {
    shiftKeysAndPointers(leaf, splitIndex);
    insertKeyAndPointer(leaf, splitIndex, key, value);
}

void resetLeafNode(LeafNode* leafNode) {
    leafNode->next_leaf = NULL;
    leafNode->prev_leaf = NULL;
    leafNode->key_num = 0;
}

void resetInternalNode(InternalNode* internalNode) {
    for (int i = 0; i < internalNode->key_num + 1; i++) {
        internalNode->children[i] = NULL;
    }
    internalNode->key_num = 0;
}

void deleteNode(Node *node) {
    if (node != NULL) {
        if (node->is_leaf) {
            resetLeafNode((LeafNode*)node);
        } else {
            resetInternalNode((InternalNode*)node);
        }
    }
}

void findInsertPosition(KeyType* keyArray, int len, KeyType key, int &pos) {
    pos = 0;
    for (int i = 0; i < len; i++) {
        if (key < keyArray[i]) {
            pos = i;
            break;
        }
        if (i == len - 1) {
            pos = len;
            break;
        }
    }
}

void shiftElements(KeyType* keyArray, RecordPointer* recordArray, int len, int pos) {
    for (int i = len; i > pos; i--) {
        keyArray[i] = keyArray[i - 1];
        if (recordArray != NULL) {
            recordArray[i] = recordArray[i - 1];
        }
    }
}

void insertKeyAtIndex(KeyType* keyArray, RecordPointer* recordArray, KeyType key, RecordPointer record, int pos) {
    keyArray[pos] = key;
    if (recordArray != NULL) {
        recordArray[pos] = record;
    }
}

void insertAtIndex(KeyType* keyArray, RecordPointer* recordArray, KeyType &key, RecordPointer &record, int len) {
    int pos;
    findInsertPosition(keyArray, len, key, pos);
    shiftElements(keyArray, recordArray, len, pos);
    insertKeyAtIndex(keyArray, recordArray, key, record, pos);
}

void BPlusTree::backPropagateInternal(const KeyType &key, InternalNode *parent, InternalNode *newLeaf){
	InternalNode *current = parent;
  
  // if internal node has space
	if(current->key_num < MAX_FANOUT - 1){
    insertKeyIntoInternalNode(current, key, newLeaf);
	}
	else{
		InternalNode *rightInternalNode = new InternalNode();
    rightInternalNode->parent = current->parent;
		KeyType keyArray[MAX_FANOUT];
		InternalNode *recordArray[MAX_FANOUT+1];

    // copy keys and records
		for(int i=0; i<MAX_FANOUT-1;i++){
			keyArray[i] = current->keys[i];
		}
		for(int i=0; i<MAX_FANOUT;i++){
      recordArray[i] = (InternalNode*)current->children[i];
    }

    // find index to insert key
		int i=0,j;
		while((i<MAX_FANOUT-1) && (key > keyArray[i])){
			i++;
		}

    // shifting operation
		for(int j = MAX_FANOUT-1; j>i; j--){
			keyArray[j] = keyArray[j-1]; 
		}
    for(int j = MAX_FANOUT;j>i+1; j--){
			recordArray[j] = recordArray[j-1];
		}

    // insert records
		keyArray[i] = key;
		recordArray[i+1] = (InternalNode*)newLeaf;
		rightInternalNode->is_leaf = false;

		current->key_num = (MAX_FANOUT)/2;
		rightInternalNode->key_num = MAX_FANOUT - (MAX_FANOUT)/2 - 1; 

    // inflate left node
    for(int i=0;i<current->key_num;i++){
      current->keys[i] = keyArray[i];
      current->children[i] = (InternalNode*)recordArray[i];
    }
    current->children[current->key_num] = recordArray[current->key_num];

    // inflate right node
		for(i = 0, j = current->key_num+1; i<rightInternalNode->key_num;i++, j++){
      rightInternalNode->keys[i] = keyArray[j]; 
      rightInternalNode->children[i]=(InternalNode*)recordArray[j];
      rightInternalNode->children[i]->parent = rightInternalNode;
    } 
    rightInternalNode->children[rightInternalNode->key_num] = (InternalNode*)recordArray[current->key_num + rightInternalNode->key_num + 1];
    rightInternalNode->children[rightInternalNode->key_num]->parent = rightInternalNode;

    // value to move up to the parent node
    KeyType passValue = keyArray[current->key_num];

		if(current-> parent == NULL){
			InternalNode *currentRoot = (InternalNode*)current;
      InternalNode *newRoot = new InternalNode;

      // create new internal node and make the current internal node its child
      newRoot->keys[0] = passValue;
      newRoot->children[0] = currentRoot;
      newRoot->children[1] = rightInternalNode;

      currentRoot->parent = newRoot;
      rightInternalNode->parent = newRoot;
      newRoot->is_leaf = false;
      newRoot->key_num = 1;
      root = (Node*)newRoot;
		}
		else{
			backPropagateInternal(passValue, (InternalNode*)current->parent, rightInternalNode);
		}	
	}
}

void shiftFromLeft(int leftIndex, InternalNode* parent,LeafNode* current){
  LeafNode* leftLeafNode = (LeafNode*)parent->children[leftIndex];
  if(leftLeafNode->key_num > MAX_FANOUT/2){
    KeyType keyArray[current->key_num+1];
    RecordPointer recordArray[current->key_num+1];
    
    // move the data to an array and insert
    for(int i=0; i<current->key_num; i++){
        keyArray[i] = current->keys[i];
        recordArray[i] = current->pointers[i];
    } 
    KeyType keyToInsert = leftLeafNode->keys[leftLeafNode->key_num-1];
    RecordPointer recordToInsert = leftLeafNode->pointers[leftLeafNode->key_num-1];
    insertAtIndex(keyArray, recordArray, keyToInsert, recordToInsert, current->key_num);
    
    // copy back to the nodes
    for(int i=0; i<current->key_num+1; i++){
      current->keys[i] = keyArray[i];
      current->pointers[i] = recordArray[i];
    }
    current->key_num++;
    leftLeafNode->key_num--;
    parent->keys[leftIndex] = current->keys[0];
  }
}

void shiftFromRight(int rightIndex, InternalNode* parent,LeafNode* current){
  LeafNode* righLeafNode = (LeafNode*) parent->children[rightIndex];
  if(righLeafNode->key_num > MAX_FANOUT/2){
    KeyType keyArray[current->key_num+1];
    RecordPointer recordArray[current->key_num+1];
    
    // move the data to an array and insert
    for(int i=0; i<current->key_num; i++){
        keyArray[i] = current->keys[i];
        recordArray[i] = current->pointers[i];
    }
    KeyType keyToInsert = righLeafNode->keys[0];
    RecordPointer recordToInsert = righLeafNode->pointers[0];
    
    insertAtIndex(keyArray, recordArray, keyToInsert, recordToInsert, current->key_num);
    
    // copy back to the nodes
    for(int i=0; i<current->key_num+1; i++){
      current->keys[i] = keyArray[i];
      current->pointers[i] = recordArray[i];
    }
    current->key_num++;
    for(int i=0; i<righLeafNode->key_num-1; i++){
        righLeafNode->keys[i] = righLeafNode->keys[i+1];
        righLeafNode->pointers[i] = righLeafNode->pointers[i+1];
    }
    righLeafNode->key_num--;
    parent->keys[rightIndex-1] = righLeafNode->keys[0];
  }
}

void shiftFromLeftInternal(InternalNode* level2Parent, int leftIndex, InternalNode* current, int index){
  Node* leftNode = level2Parent->children[leftIndex];
  if(leftNode->key_num > MAX_FANOUT/2){
    
    // copy to array
    KeyType keyArray[current->key_num+1];
    for(int i=0; i<current->key_num; i++){
        keyArray[i] = current->keys[i];
    }
    KeyType keyToInsert = level2Parent->keys[leftIndex];

    // insert a dummy record and then move values back to current
    RecordPointer dummy; 
    insertAtIndex(keyArray, NULL, keyToInsert, dummy, current->key_num);
    for(int i=0; i<current->key_num+1; i++){
      current->keys[i] = keyArray[i];
    }
    level2Parent->keys[leftIndex] = leftNode->keys[leftNode->key_num-1];
    
    // copy children to array
    Node** childArray = new Node* [current->key_num+2];
    for(int i=0; i<current->key_num+1; i++){
        childArray[i] = current->children[i];
    }
    InternalNode* leftInternalNode = (InternalNode*) leftNode;
    // shifting operation
    for(int i=current->key_num; i>index; i--){
        childArray[i] = childArray[i-1];
    }
    childArray[index] = leftInternalNode->children[leftNode->key_num];
    
    // inflate current with new set of children
    for(int i=0; i<current->key_num+2; i++){
        current->children[i] = childArray[i];
    }
    current->key_num++;
    leftNode->key_num--;
    return;
  }
}

void shiftFromRightInternal(InternalNode* level2Parent, int rightIndex, InternalNode* current, int index, int currIndex){
  Node* rightNode = level2Parent->children[rightIndex];
  if(rightNode->key_num > MAX_FANOUT/2){

    // copy to array
    KeyType keyArray[current->key_num+1];
    for(int i=0; i<current->key_num; i++){
        keyArray[i] = current->keys[i];
    }
    KeyType keyToInsert = level2Parent->keys[currIndex];

    // insert a dummy record and move values back to current
    RecordPointer dummy;
    insertAtIndex(keyArray, NULL, keyToInsert, dummy, current->key_num);
    for(int i=0; i<current->key_num+1; i++){
        current->keys[i] = keyArray[i];
    }
    level2Parent->keys[currIndex] = rightNode->keys[0];

    // copy children to array
    InternalNode* rightInternalNode = (InternalNode*) rightNode;
    current->children[current->key_num+1] = rightInternalNode->children[0];
    for(int i=0; i<rightNode->key_num;i++){
        rightInternalNode->children[i] = rightInternalNode->children[i+1];
        rightInternalNode->keys[i] = rightInternalNode->keys[i+1];
    }
    rightInternalNode->children[rightNode->key_num] = NULL;
    current->key_num++;
    rightNode->key_num--;
    return;
  }
}

void BPlusTree::removeParent(Node* delNode,int index, InternalNode* parent){
    InternalNode* current = parent;

    if(((Node*) current == root) && (current->key_num == 1)){
      // make the child the root if parent is being deleted
      if(delNode == current->children[0]){
        root = (Node* ) current->children[1];
        return;
      }
      if(delNode == current->children[1]){
          root = (Node*) current->children[0];
          return;
      }
    }
    
    // shifting operation to remove key
    for(int i = index; i<current->key_num-1; i++){
        current->keys[i] = current->keys[i+1];
    }
    
    // find index of child and delete child
    int delIndex = -1;
    for(int i=0; i<current->key_num+1; i++){
        if(current->children[i] == delNode){
            delIndex = i;
        }
    }
    if(delIndex == -1){
        return;
    }
    for(int i = delIndex; i<current->key_num; i++){
        current->children[i] = current->children[i+1];
    }
    current->children[current->key_num] = NULL;
    current->key_num--;
    
    if(current == (InternalNode*)root){
        return;
    }
    if (current->key_num + 1 < MAX_FANOUT/2){
      // in case of underflow borrow
      int currIndex = -1;
      InternalNode* level2Parent = (InternalNode*) current->parent;
      if (level2Parent != NULL){
        for(int i=0; i<level2Parent->key_num+1; i++){
          if(current == level2Parent->children[i]){
            currIndex = i;
          }
        }
      }
      int leftIndex = currIndex - 1;
      int rightIndex = currIndex + 1;
      
      // borrow from left
      if(leftIndex >= 0){
        shiftFromLeftInternal(level2Parent, leftIndex,current, index);
        return;
      }

      // borrow from right
      if (rightIndex <= level2Parent->key_num){            
        shiftFromRightInternal(level2Parent, rightIndex,current, index, currIndex);
        return;
      }
        
      // merge if no borrowing can be done
      if(leftIndex >= 0){
        Node* leftNode = level2Parent->children[leftIndex];
        leftNode->keys[leftNode->key_num] = level2Parent->keys[leftIndex];
        for(int i=0; i<current->key_num; i++){
            leftNode->keys[leftNode->key_num+i+1] = current->keys[i];
        }
        InternalNode* leftInternalNode = (InternalNode*) leftNode;
        for(int i=0; i<current->key_num+1; i++){
            leftInternalNode->children[leftNode->key_num+i+1] = current->children[i];
            current->children[i]->parent = leftNode;
        }
        for(int i=0; i<current->key_num+1; i++){
            current->children[i] = NULL;
        }
        leftNode->key_num = leftNode->key_num + current->key_num + 1;
        removeParent(current, leftIndex, level2Parent);
        return;
      }
        
      if(rightIndex <= level2Parent->key_num){
        Node* rightNode = level2Parent->children[rightIndex];
        current->keys[current->key_num] = level2Parent->keys[rightIndex-1];
        for(int i=0; i<rightNode->key_num; i++){
            current->keys[current->key_num+1+i] = rightNode->keys[i];
        }
        InternalNode* rightInternalNode = (InternalNode*) rightNode;
        for(int i=0; i<rightNode->key_num+1; i++){
            current->children[current->key_num+i+1] = rightInternalNode->children[i];
            rightInternalNode->children[i]->parent = current;
        }
        for(int i=0; i<rightNode->key_num+1; i++){
            rightInternalNode->children[i] = NULL;
        }
        current->key_num = rightInternalNode->key_num+current->key_num+1;
        removeParent(rightInternalNode, rightIndex-1, level2Parent);
        return;
      }     
    }
    else{
      return;
    }
   
}


/*
 * Helper function to decide whether current b+tree is empty
 */

bool BPlusTree::IsEmpty() const {
  return root == NULL;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */

bool BPlusTree::GetValue(const KeyType &key, RecordPointer &result) {
    if(root != NULL) {
        // Get leaf node
        LeafNode* current = getLeaf(root, key);
        for(int idx = 0; idx < current->key_num; idx++){
            if (key == current->keys[idx]){
                result = ((LeafNode *)current)->pointers[idx];
                return true;
            }
        }
    }
    return false; 
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * If current tree is empty, start new tree, otherwise insert into leaf Node.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */


bool BPlusTree::Insert(const KeyType &key, const RecordPointer &value) {

  if(IsEmpty()){
        root = new LeafNode();
        root->keys[0] = key;
        root->key_num++;
        ((LeafNode*)root)->pointers[0] = value;
        return true;
  }

  InternalNode *current = (InternalNode*)getLeaf(root, key);

  // If leaf node has space
  LeafNode *currentLeaf = (LeafNode*)current;

  if(currentLeaf->key_num < MAX_FANOUT-1) {
      insertAtLeaf(currentLeaf, key, value, GetInsertIndex(currentLeaf, key));
  }

  else{
    LeafNode *rightLeaf = new LeafNode();
    KeyType keyArray[MAX_FANOUT]; 
    RecordPointer recordArray[MAX_FANOUT]; 

    // copy keys and records to array
    for(int i=0;i<MAX_FANOUT-1;i++){
        keyArray[i] = currentLeaf->keys[i];
        recordArray[i] = currentLeaf->pointers[i];
    }

    // find insert index
    int i=0, j;
    while((i < MAX_FANOUT-1) && (key > keyArray[i])){ 
        i++;	
    }

    // shifting operation
    for(int j = MAX_FANOUT-1;j>i;j--){
        keyArray[j] = keyArray[j-1];
        recordArray[j] = recordArray[j-1];
    }

    // insert key and value
    keyArray[i] = key;
    recordArray[i] = value;

    // Populating new leaf node
    rightLeaf->is_leaf = true;
    rightLeaf->parent = currentLeaf->parent;
    currentLeaf->key_num = (MAX_FANOUT)/2; 
    rightLeaf->key_num = (MAX_FANOUT+1)/2;

    // copy records to new leaves
    for(i = 0;i<currentLeaf->key_num;i++){
        currentLeaf->keys[i] = keyArray[i];
        currentLeaf->pointers[i] = recordArray[i];
    }
    for(i=0, j = currentLeaf->key_num; i<rightLeaf->key_num; i++,j++){
        rightLeaf->keys[i] = keyArray[j];
        rightLeaf->pointers[i] = recordArray[j];
    }

    // adjust next and previous pointers
    rightLeaf->next_leaf = currentLeaf->next_leaf;
    currentLeaf->next_leaf = rightLeaf;
    rightLeaf->prev_leaf = currentLeaf;

    // if parent is null, create new internal node
    if(current->parent == NULL){
      InternalNode *newInternalNode = new InternalNode();
      newInternalNode->keys[0] = rightLeaf->keys[0];

      newInternalNode->children[0] = current;
      newInternalNode->children[1] = rightLeaf;

      current->parent = newInternalNode;
      rightLeaf->parent = newInternalNode;

      newInternalNode->is_leaf = false;
      newInternalNode->key_num = 1;
      root = newInternalNode;
    }
    else
    {
      backPropagateInternal(rightLeaf->keys[0], (InternalNode*)current->parent, (InternalNode*)rightLeaf);
    }
  }

  return true;

}


/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf node as deletion target, then
 * delete entry from leaf node. Remember to deal with redistribute or merge if
 * necessary.
 */

void BPlusTree::Remove(const KeyType &key) {
  RecordPointer temp;
  if(IsEmpty()){
      return;
  }
  else if(!GetValue(key, temp)){
      return;
  }
  else{

    // go to leafnode with key
    Node* keyIndex = getLeaf(root, key);
    InternalNode* current = (InternalNode*) keyIndex;
    InternalNode* oldParent = (InternalNode*) keyIndex->parent;
      
    // find adjacent leaf nodes
    int adjLeafIndex = -1;
    if(oldParent != NULL){
      for(int i=0; i<oldParent->key_num+1; i++){
        if(current == oldParent->children[i]){
          adjLeafIndex = i;
        }
      }
    }

    int leftLeaf = adjLeafIndex-1;
    int rightLeaf = adjLeafIndex+1;
      
    // get index of record to delete
    int deleteKey = -1;
    for(int i=0; i<current->key_num; i++){
      if(current->keys[i] == key){
        deleteKey = i;
        break;
      }
    }
      
    if(deleteKey == -1){
      return;
    }

    LeafNode* dummyCurrent = (LeafNode*) current;

    // shifting operation to remove record
    for(int i=deleteKey; i<dummyCurrent->key_num-1; i++){
      dummyCurrent->keys[i] = dummyCurrent->keys[i+1];
      dummyCurrent->pointers[i] = dummyCurrent->pointers[i+1];
    }
    dummyCurrent->key_num--;

    // if the deleted value was at root node, release the root
    if(dummyCurrent == (LeafNode*) root && dummyCurrent->key_num == 0){
      deleteNode(root);
      root = NULL;
      return;
    }
    if(dummyCurrent == (LeafNode*) root){
      return;
    }

    // In case of underflow in the leaf
    if(dummyCurrent->key_num < MAX_FANOUT/2){
      // if left leaf exists, borrow from left
      if(leftLeaf >= 0){
        shiftFromLeft(leftLeaf, oldParent, dummyCurrent);
        return;    
      }

      // if right leaf exists, borrow from right
      if(rightLeaf <= oldParent->key_num){
        shiftFromRight(rightLeaf, oldParent,dummyCurrent);
        return;
      }
          
      // merge
      if(leftLeaf >= 0){
          LeafNode* leftNode = (LeafNode*)oldParent->children[leftLeaf];
          // merge the two leaf nodes
          for(int i=0; i< dummyCurrent->key_num; i++){
            leftNode->keys[leftNode->key_num+i] = dummyCurrent->keys[i];
            leftNode->pointers[leftNode->key_num+i] = dummyCurrent->pointers[i];
          }
          leftNode->key_num = leftNode->key_num + dummyCurrent->key_num;
          leftNode->next_leaf = dummyCurrent->next_leaf;

          if(dummyCurrent->next_leaf != NULL){
            dummyCurrent->next_leaf->prev_leaf = leftNode;
          }
          removeParent(dummyCurrent, leftLeaf, oldParent);
          deleteNode((Node*)dummyCurrent);
          return;
      }
      if(rightLeaf <= oldParent->key_num){
        LeafNode* rightNode = (LeafNode*)oldParent->children[rightLeaf];
        // merge the two nodes
        for(int i=0; i<rightNode->key_num; i++){
            dummyCurrent->keys[i+dummyCurrent->key_num] = rightNode->keys[i];
            dummyCurrent->pointers[i+dummyCurrent->key_num] = rightNode->pointers[i];
        }
        dummyCurrent->key_num = dummyCurrent->key_num + rightNode->key_num;
        dummyCurrent->next_leaf = rightNode->next_leaf;

        if(rightNode->next_leaf != NULL)
            rightNode->next_leaf->prev_leaf = dummyCurrent;
        removeParent(rightNode, rightLeaf-1, oldParent);
        deleteNode((Node*)rightNode);
        return;
      }
    }
    else{
      return;
    }
  }
}

/*****************************************************************************
 * RANGE_SCAN
 *****************************************************************************/
/*
 * Return the values that within the given key range
 * First find the node large or equal to the key_start, then traverse the leaf
 * nodes until meet the key_end position, fetch all the records.
 */

void BPlusTree::RangeScan(const KeyType &key_start, const KeyType &key_end, std::vector<RecordPointer> &result) {
    if (root != NULL) {
        // Get leaf node
        LeafNode* current = getLeaf(root, key_start);
        
        while (current != NULL) {
            int idx = 0;
            for (idx = 0; idx < current->key_num; idx++) {
                if (current->keys[idx] >= key_start && current->keys[idx] <= key_end) {
                    result.push_back(((LeafNode *)current)->pointers[idx]);
                } else if (current->keys[idx] > key_end) {
                    return; // Exit the loop when keys are out of range
                }
            }
            current = current->next_leaf;
        }
    }
}