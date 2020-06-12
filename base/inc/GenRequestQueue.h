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
#ifndef Force_GenRequestQueue_H
#define Force_GenRequestQueue_H

#include <Defines.h>
#include <Object.h>
//#include <Enums.h>
#include ARCH_ENUM_HEADER

#include <list>
#include <vector>

namespace Force {

  class GenRequest;

  /*!
    \class GenRequestQueue
    \brief A container for GenRequest based request objects, one instance per generator thread.
   */
  class GenRequestQueue : public Object {
  public:
    typedef std::list<GenRequest* >::const_iterator ConstGRequestQIter; //!< Short handle typedef for list<GenRequest* > const iterator
    typedef std::list<GenRequest* >::iterator GRequestQIter; //!< Short handle typedef for list<GenRequest* > iterator

    GenRequestQueue(); //!< Constructor.
    ~GenRequestQueue(); //!< Destructor.
    Object* Clone() const override;  //!< Return a cloned GenRequestQueue object of the same type and content.
    const std::string ToString() const override; //!< Return a string describing the current state of the GenRequestQueue object.
    const char* Type() const override { return "GenRequestQueue"; } //!< Return the type of the object

    ConstGRequestQIter StartRound(); //!< Start a generation round of one more instructions.
    void PrependRequest(GenRequest* genRequest); //!< Prepend a GenRequest object to the queue.
    void PrependRequests(std::vector<GenRequest* >& requests); //!< Prepend a vector of GenRequest objects to the queue.
    bool RoundFinished(const ConstGRequestQIter& rRoundEndIter) const; //!< Return true if reached the end of a generation round.
    GenRequest* PopFront(); //!< Remove a GenRequest object from the front of the queue and return.
    uint32 Size() const { return mRequestQueue.size(); }
  private:
    GenRequestQueue(const GenRequestQueue& rOther); //!< Copy constructor hidden.
  private:
    std::list<GenRequest* > mRequestQueue; //!< List of GenRequest objects
  };

}

#endif
