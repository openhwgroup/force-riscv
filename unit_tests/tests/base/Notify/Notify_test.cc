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

#include <Notify.h>
#include <Enums.h>
#include <Register.h>

namespace Force {

 class SenderTest : public Sender<ENotificationType> {
  public:
    SenderTest() : Sender() { }
    SenderTest(const SenderTest& other) : Sender(other) { }
  protected:
   const std::string Description() const { return "SenderTest"; }
  };

  class ReceiverTest : public Receiver<ENotificationType> {
  public:
    ReceiverTest() : Receiver(), mState(0), mEventCount(0) { }
    ReceiverTest(const ReceiverTest& other) : Receiver(other), mState(0), mEventCount(0) { }
    void SetState(uint32 state) { mState = state; }
    uint32 State() const { return mState; }
    uint32 EventCount() const { return mEventCount; }
  protected:
    const std::string Description() const { return "ReceiverTest"; }

    void HandleNotification(const Sender<ENotificationType>* sender, ENotificationType eventType, Object* pPayload) override {
      ++ mEventCount;

      switch (mState) {
      case 0:
	mState = 1;
	break;
      case 1:
	mState = 0;
	sender->SendNotification(ENotificationType::RegisterUpdate);
	break;
      default:
	;
      }
    }
  private:
    uint32 mState; //!< State variable to determine receiver behavior.
    uint32 mEventCount; //!< Count number of events.
  };

}

using text = std::string;

using namespace std;
using namespace Force;

const lest::test specification[] = {
  
  CASE( "test case description" "more description" ) {

    SETUP( "setup description" )  {
    
      SenderTest s_test1;
      ReceiverTest r_test1;
      ReceiverTest r_test2;
      

      SECTION( "Test set 1, simple event passing." ) {
	s_test1.SignUp(&r_test1);
	s_test1.SendNotification(ENotificationType::RegisterUpdate);

	EXPECT(r_test1.State() == 1u);

	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	EXPECT(r_test1.State() == 1u);
	EXPECT(r_test1.EventCount() == 3u);
      }

      SECTION( "Test set 2, block and unblock" ) {
	s_test1.SignUp(&r_test1);
	s_test1.SignUp(&r_test2);

	s_test1.Block(); // block sending.
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	s_test1.SendNotification(ENotificationType::RegisterUpdate);

	EXPECT(r_test1.EventCount() == 0u);
	EXPECT(r_test2.EventCount() == 0u);

	s_test1.Unblock(); // unblock sending.
	EXPECT(r_test1.EventCount() == 1u);
	EXPECT(r_test2.EventCount() == 1u);
      }

      SECTION( "Test set 3, block and clear" ) {
	s_test1.SignUp(&r_test1);
	s_test1.SignUp(&r_test2);

	s_test1.Block(); // block sending.
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	s_test1.SendNotification(ENotificationType::RegisterUpdate);

	EXPECT(r_test1.EventCount() == 0u);
	EXPECT(r_test2.EventCount() == 0u);

	s_test1.Clear(); // clear sending and chched events, no event.
	EXPECT(r_test1.EventCount() == 0u);
	EXPECT(r_test2.EventCount() == 0u);
      }

      // Receiver tests
      SECTION( "Test set 4, block and unblock receiver" ) {

	s_test1.SignUp(&r_test1);
	s_test1.SendNotification(ENotificationType::RegisterUpdate);

	EXPECT(r_test1.State() == 1u);
        r_test1.Block();
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	EXPECT(r_test1.State() == 1u);
	EXPECT(r_test1.EventCount() == 1u);
        
        r_test1.Unblock();
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	EXPECT(r_test1.State() == 1u);
	EXPECT(r_test1.EventCount() == 5u);

        r_test1.Block();
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
        r_test1.Clear();
	s_test1.SendNotification(ENotificationType::RegisterUpdate);
	EXPECT(r_test1.EventCount() == 7u);

      }


    }
  }

};

int main( int argc, char * argv[] )
{
    Force::Logger::Initialize();
    int ret = lest::run( specification, argc, argv );
    Force::Logger::Destroy();
    return ret;
}
