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
#include <lest/lest.hpp>
#include <Log.h>

#include <ObjectRegistry.h>

using text = std::string;

using namespace std;

namespace Force {

  class ObjectTEST1 : public Object {
    virtual Object* Clone() const { return new ObjectTEST1(*this); }
    virtual const string ToString() const { return string("Object type of: ") + Type(); } 
    virtual const char* Type() const { return "ObjectTEST1"; }
  };

  class ObjectTEST2 : public Object {
    virtual Object* Clone() const { return new ObjectTEST2(*this); }
    virtual const string ToString() const { return string("Object type of: ") + Type(); } 
    virtual const char* Type() const { return "ObjectTEST2"; }
  };

  class ObjectTEST3 : public Object {
    virtual Object* Clone() const { return new ObjectTEST3(*this); }
    virtual const string ToString() const { return string("Object type of: ") + Type(); } 
    virtual const char* Type() const { return "ObjectTEST3"; }
  };

}

const lest::test specification[] = {

CASE( "ObjectRegistry test set 1" ) {

  using namespace Force;

    SETUP( "setup ObjectRegistry" )  {
      ObjectRegistry::Initialize();
      ObjectRegistry * obj_reg = ObjectRegistry::Instance();

      EXPECT( obj_reg != nullptr );

        SECTION( "Test Object registration" ) {
	  Object* orig_TEST1 = new ObjectTEST1();
	  obj_reg->RegisterObject(orig_TEST1);
	  obj_reg->RegisterObject(new ObjectTEST2());
	  obj_reg->RegisterObject(new ObjectTEST3());

	  Object* clone_TEST1 = obj_reg->ObjectInstance("ObjectTEST1");
	  EXPECT(clone_TEST1 != nullptr);
	  EXPECT(clone_TEST1 != orig_TEST1);
	  EXPECT(clone_TEST1->Type() == "ObjectTEST1");

	  Object* dup_TEST2 = new ObjectTEST2();

	  EXPECT_FAIL( obj_reg->RegisterObject(dup_TEST2), "register-duplicated-object" );
	  delete dup_TEST2; // avoid memory leak;

	  EXPECT_FAIL( obj_reg->ObjectInstance("ObjectUNKNOWN"), "object-not-found");
        }

      ObjectRegistry::Destroy();
      obj_reg = ObjectRegistry::Instance();
      EXPECT( obj_reg == nullptr );
    }
}};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
