#include "../include/nested_loop_join_executor.h"
#include <iostream>


NestedLoopJoinExecutor::NestedLoopJoinExecutor(
    AbstractExecutor *left_child_executor,
    AbstractExecutor *right_child_executor, const std::string join_key)
    : left_(left_child_executor),
      right_(right_child_executor),
      join_key_(join_key){};

void NestedLoopJoinExecutor::Init() {
  left_->Init();
  right_->Init();
  Tuple leftTuple, rightTuple;
  getNext = false;
}

bool matchAndAssignTuple(const Tuple& leftTuple, const Tuple& rightTuple, const std::string& join_key, Tuple* tuple) {
    if (join_key == "val1" && leftTuple.val1 == rightTuple.val1) {
        *tuple = Tuple(leftTuple.id, leftTuple.val1, leftTuple.val2);
        return true;
    } else if (join_key == "id" && leftTuple.id == rightTuple.id) {
        *tuple = Tuple(leftTuple.id, leftTuple.val1, leftTuple.val2);
        return true;
    } else if (join_key == "val2" && leftTuple.val2 == rightTuple.val2) {
        *tuple = Tuple(leftTuple.id, leftTuple.val1, leftTuple.val2);
        return true;
    }
    return false;
}

bool NestedLoopJoinExecutor::Next(Tuple* tuple) {
    Tuple leftTuple, rightTuple;

    if (getNext) {
        while (right_->Next(&rightTuple)) {
            if (matchAndAssignTuple(prevValue, rightTuple, join_key_, tuple)) {
                return true;
            }
        }
        getNext = false;
    }

    while (left_->Next(&leftTuple)) {
        prevValue = leftTuple;
        right_->Init();

        while (right_->Next(&rightTuple)) {
            if (matchAndAssignTuple(leftTuple, rightTuple, join_key_, tuple)) {
                getNext = true;
                return true;
            }
        }
    }

    return false;
}
