//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef Force_UtilityAlgorithms_H
#define Force_UtilityAlgorithms_H

#include <Defines.h>
#include <algorithm>
#include <vector>

namespace Force {

  class ConstraintSet;

  /*!
    Insert value type elements to a vector sorted.
  */
  template <typename T>
  inline bool insert_sorted(std::vector<T> & vec_container, T insert_item)
  {
    auto last_iter = vec_container.end();
    auto find_iter = lower_bound(vec_container.begin(), last_iter, insert_item);
    if (find_iter == last_iter) {
      // no match found.
      vec_container.insert(last_iter, insert_item);
      return true;
    } else {
      if (! (insert_item == (*find_iter))) {
        // not the same event
        vec_container.insert(find_iter, insert_item);
        return true;
      }
    }
    return false;
  }

  /*!
    \class RandomSampler
    \brief Used to return a sampling of values in a range.
  */
  class RandomSampler {
  public:
    RandomSampler(uint64 min, uint64 max); //!< Constructor with sample range given.
    ~RandomSampler(); //!< Destructor.
    uint64 Sample();  //!< Pick one from the sampling
  protected:
    // shouldn't be called
    RandomSampler() : mpConstraint(nullptr) {} //!< Default constructor.
    RandomSampler(const RandomSampler& rOther); //!< Copy constructor.
    ASSIGNMENT_OPERATOR_ABSENT(RandomSampler);
  protected:
    ConstraintSet* mpConstraint; //!< ConstraintSet object.
  };
}

#endif
