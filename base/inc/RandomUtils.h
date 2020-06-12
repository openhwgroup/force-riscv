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
#ifndef Force_RandomUtils_H
#define Force_RandomUtils_H

#include <Defines.h>
#include <algorithm>
#include <vector>
// C++UP accumulate defined in numeric
#include <numeric>

namespace Force {

  uint32 random_value32(uint32 min, uint32 max); //!< Return a random 32-bit integer value in the range [min, max]
  uint64 random_value64(uint64 min, uint64 max); //!< Return a random 64-bit integer value in the range [min, max]
  double random_real(double min, double max); //!< Return a random 64-bit real value in the range [min, max)
  void report_error(const char* pErrMsg); //!< Report an error message.

  /*!
    Select an item considering weights from the vector.
  */
  template <typename T>
    T* choose_weighted_item(const std::vector<T *>& vec_container)
    {
      typedef typename std::vector<T *>::const_iterator TConstIterator;

      switch (vec_container.size()) {
      case 0:
        return nullptr;
      case 1:
        return vec_container.front();
      default:
        ;
      }

      uint32 all_weights = std::accumulate(vec_container.begin(), vec_container.end(), 0,
        [](cuint32 partialSum, const T* vec_item) { return (partialSum + vec_item->Weight()); });

      uint32 picked_value = random_value32(0, all_weights - 1);
      auto end_iter = vec_container.end();
      for (TConstIterator vec_iter = vec_container.begin(); vec_iter != end_iter; ++ vec_iter) {
        T* vec_item = (*vec_iter);
        if (picked_value < vec_item->Weight()) {
          return vec_item;
        }
        picked_value -= vec_item->Weight();
      }
      return nullptr;
    }

  /*!
    Return an item's index in the vector considering relative weights..
  */
  template <typename T, typename WeightMethod>
  uint32 choose_item_index(const std::vector<T *>& vec_container, WeightMethod pWeightMethod)
    {
      typedef typename std::vector<T *>::const_iterator TConstIterator;

      switch (vec_container.size()) {
      case 0:
        report_error("choose_item_index: empty container");
        return 0;
      case 1:
        return 0;
      default:
        ;
      }

      uint64 all_weights = std::accumulate(vec_container.begin(), vec_container.end(), 0,
        [pWeightMethod](cuint64 partialSum, const T* vec_item) { return (partialSum + (vec_item->*pWeightMethod)()); });

      uint32 picked_value = random_value64(0, all_weights - 1);
      auto end_iter = vec_container.end();
      uint32 ret_index = 0;
      for (TConstIterator vec_iter = vec_container.begin(); vec_iter != end_iter; ++ vec_iter) {
        T* vec_item = (*vec_iter);
        if (picked_value < (vec_item->*pWeightMethod)()) {
          return ret_index;
        }
        picked_value -= (vec_item->*pWeightMethod)();
        ++ ret_index;
      }
      return ret_index;
    }

  /*!
    \class ItemChooser
    \brief Utility class to choose an item randomly
  */
  template<class ItemType> class ItemChooser {
  public:
    explicit ItemChooser(const std::vector<ItemType>& items):mItems(items), mWeights() { // Constructor
      mWeights.assign(items.size(), 10);
    }
    ~ItemChooser() { }  //Destructor

    uint32 Choose(ItemType& item) const { //!< Choose one item randomly
      uint32 index = 0;
      uint32 picked_value = random_value32(0, TotalWeight() - 1);

      for (auto weight : mWeights) {
        if (picked_value < weight)
          break;
        else {
          picked_value -= weight;
          index ++;
        }
      }
      item = mItems[index];
      return index;
    }

    void Erase(uint32 index) { //!< Erase an item
      if (index >= mItems.size()) {
        report_error("index-over-flow");
        return;
      }
      mWeights[index] = 0;
    }

    bool HasChoice() const { //!< Whether has choice no not
      return TotalWeight();
    }
  protected:
    uint32 TotalWeight() const { //!< get total weight
      return std::accumulate(mWeights.begin(), mWeights.end(), 0);
    }
  protected:
    std::vector<ItemType> mItems; //!< the container for items
    std::vector<uint32> mWeights; //!< the weights for each items
  };

}

#endif
