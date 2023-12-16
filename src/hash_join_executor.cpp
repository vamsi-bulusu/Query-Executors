#include "../include/hash_join_executor.h"
using std::cout;
using std::endl;

HashJoinExecutor::HashJoinExecutor(AbstractExecutor *left_child_executor,
                                   AbstractExecutor *right_child_executor,
                                   SimpleHashFunction *hash_fn)
    : left_(left_child_executor),
      right_(right_child_executor),
      hash_fn_(hash_fn) {}

void HashJoinExecutor::Init(){
  left_->Init();
  right_->Init();
  Tuple leftTuple;
  SimpleHashJoinHashTable ht2;
  ht = ht2;
  while (left_->Next(&leftTuple))
  {
    hash_t hashValue = hash_fn_->GetHash(leftTuple);
    ht.Insert(hashValue, leftTuple);
  }
  getNext = true;
};

bool HashJoinExecutor::Next(Tuple *tuple) {
  if (getNext) {
    return ProcessNextRightTuple(tuple);
  } else {
    return ProcessMatchedValues(tuple);
  }
}

bool HashJoinExecutor::ProcessNextRightTuple(Tuple *tuple) {
  Tuple rightTuple;
  while (right_->Next(&rightTuple)) {
    hash_t hashValue = hash_fn_->GetHash(rightTuple);
    prevVal = rightTuple;
    ht.GetValue(hashValue, &matchedValues);
    if (!matchedValues.empty()) {
      Tuple res = matchedValues.front();
      if (matchedValues.size() > 1) {
        getNext = false;
      }
      matchedValues.erase(matchedValues.begin());
      *tuple = CreateTupleFromResult(res);
      return true;
    }
  }
  return false;
}

bool HashJoinExecutor::ProcessMatchedValues(Tuple *tuple) {
  Tuple res = matchedValues.front();
  if (IsMatchingHash(res)) {
    matchedValues.erase(matchedValues.begin());
    if (matchedValues.size() == 0) {
      getNext = true;
    }
    *tuple = CreateTupleFromResult(res);
    return true;
  } else {
    matchedValues.erase(matchedValues.begin());
    return false;
  }
}

Tuple HashJoinExecutor::CreateTupleFromResult(const Tuple& res) const {
  return Tuple(res.id, res.val1, res.val2);
}

bool HashJoinExecutor::IsMatchingHash(const Tuple& res) const {
  return (hash_fn_->type == "id" && res.id == prevVal.id) ||
         (hash_fn_->type == "val1" && res.val1 == prevVal.val1) ||
         (hash_fn_->type == "val2" && res.val2 == prevVal.val2);
}
