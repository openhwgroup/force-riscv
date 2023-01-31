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
#ifndef Force_GenAgent_H
#define Force_GenAgent_H

#include "Defines.h"
#include "Enums.h"
#include "Object.h"
#include ARCH_ENUM_HEADER

namespace Force {

  class Generator;
  class GenRequest;
  class GenQuery;

  /*!
    \class GenAgent
    \brief Base class for all generator agents.
  */
  class GenAgent : public Object {
  public:
    explicit GenAgent(Generator* gen) : Object(), mpGenerator(gen) { } //!< Constructor taking Generator pointer parameter.
    GenAgent() : Object(), mpGenerator(nullptr) { } //!< Constructor.
    ~GenAgent() { } //!< Destructor.
    ASSIGNMENT_OPERATOR_ABSENT(GenAgent);

    const std::string ToString() const override //!< Return a string describing the current state of the GenInstructionAgent object.
    {
      return std::string("GenAgent : ") + Type();
    }

    virtual EGenAgentType GenAgentType() const = 0; //!< Return type of the generator agent.
    void SetGenerator(Generator* gen) { mpGenerator = gen; } //!< Set pointer to owner generator.
    virtual void SetGenRequest(GenRequest* genRequest); //!< Set pointer to the GenRequest object.
    virtual void SetGenQuery(const GenQuery* genQuery) const; //!< Set pointer to the GenQuery object.

    inline void ProcessRequest(GenRequest* genRequest) //!< Process a generator request transaction.
    {
      SetGenRequest(genRequest);
      HandleRequest();
      CleanUpRequest();
    }

    inline void ProcessRequestWithResult(GenRequest* genRequest) //!< Process a generator request transaction which require result to be preserved.
    {
      SetGenRequest(genRequest);
      HandleRequest();
      // not cleaning up request item since result is needed.
      ResetRequest();
    }

    inline void ProcessQuery(const GenQuery* genQuery) const //!< Process a generator query transaction.
    {
      SetGenQuery(genQuery);
      HandleQuery();
    }

  protected:
    GenAgent(const GenAgent& rOther) : Object(rOther), mpGenerator(nullptr) { } //!< Copy constructor, do not copy the generator pointer.
    virtual void HandleRequest() { } //!< Handle generator request.
    virtual void HandleQuery() const { } //!< Handle generator query.
    virtual void CleanUpRequest() { } //!< Clean up request item.
    virtual void ResetRequest() { } //!< Reset request item.
    void UnimplementedMethod(const std::string& methodName) const; //!< Report error on an unimplemented method.
  protected:
    Generator* mpGenerator; //!< Pointer to owner generator.
  };

}

#endif

