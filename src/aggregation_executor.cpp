#include "../include/aggregation_executor.h"
#include <iostream>
#include <climits>
using std::cout;
using namespace std;

AggregationExecutor::AggregationExecutor(AbstractExecutor *child_executor,
                                         AggregationType aggr_type)
    : child_(child_executor), aggr_type_(aggr_type){};

void AggregationExecutor::Init() {
    child_->Init();
    getNext = true;
}

bool AggregationExecutor::Next(Tuple *tuple) {

    int result = 0;
    if(getNext){
        Tuple temp;
        if (aggr_type_ == AggregationType::COUNT) {
            while (child_->Next(&temp)) {
                result++;
            }
        } else if (aggr_type_ == AggregationType::MAX) {
            int max = INT32_MIN;
            while (child_->Next(&temp)) {
                max = std::max(max, temp.val1);
            }
            result = max;
        } else if (aggr_type_ == AggregationType::MIN) {
            int min = INT32_MAX;
            while (child_->Next(&temp)) {
                min = std::min(min, temp.val1);
            }
            result = min;
        } else {
            int sum = 0;
            while (child_->Next(&temp)) {
                sum += temp.val1;
            }
            result = sum;
        }
        tuple->val1 = result;
        getNext = false;
        return true;
    }
    return false;
}
