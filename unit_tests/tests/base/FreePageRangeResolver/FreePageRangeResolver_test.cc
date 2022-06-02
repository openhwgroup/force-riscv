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
#include "FreePageRangeResolver.h"

#include <vector>

#include "lest/lest.hpp"

#include "Choices.h"
#include "Constraint.h"
#include "Defines.h"
#include "Enums.h"
#include "Log.h"
#include "Random.h"

using text = std::string;

const lest::test specification[] = {

CASE( "Test Free Page Range Resolver") {
   SETUP( "Set up Free Page Range Resolver") {
     using namespace Force;
     using namespace std;

     auto ps_tree = new ChoiceTree("Page size#4K granule", 0, 10);
     auto choice = new Choice("4K", 0, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("2M", 1, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("1G", 2, 10);
     ps_tree->AddChoice(choice);

     GranulePageSizeTree granule_tree(EPageGranuleType::G4K, ps_tree);
     auto virt_usable = new ConstraintSet("0x0-0x800000");
     FreePageRangeResolver resolver(virt_usable, granule_tree);
 
     SECTION ("test request ranges") {
       ConstraintSet request_range("0x700000-0x800000");
       vector<uint64> request_page_sizes = {0, 0, 0};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == true);
       EXPECT(resolved_page_sizes.size() == 3u);
       EXPECT(resolved_page_sizes[0] == 0x1000u);
       LOG(notice) << "start address:0x"<< hex << start_addr << ",resolved ranges:" << resolved_ranges.ToSimpleString() << endl;
     }
     
     SECTION("test request page size") {
       ConstraintSet request_range;
       vector<uint64> request_page_sizes = {0x1000, 0, 0x100000};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == false); // invalid page request
     }
     
     SECTION("test request page size") {
       ConstraintSet request_range;
       vector<uint64> request_page_sizes = {0x1000, 0, 0x200000, 0};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == true);
       EXPECT(resolved_page_sizes.size() == 4u);
       EXPECT(resolved_page_sizes[0] == 0x1000u);
       EXPECT(resolved_page_sizes[2] == 0x200000u);
       LOG(notice) << "start address:0x"<< hex << start_addr << ",resolved ranges:" << resolved_ranges.ToSimpleString() << endl;
     }

   }
},

CASE( "Test Free Page Cross Range Resolver") {
   SETUP( "Set up Free Page Cross Range Resolver") {
     using namespace Force;
     using namespace std;

     auto ps_tree = new ChoiceTree("Page size#4K granule", 0, 10);
     auto choice = new Choice("4K", 0, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("2M", 1, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("1G", 2, 10);
     ps_tree->AddChoice(choice);

     GranulePageSizeTree granule_tree(EPageGranuleType::G4K, ps_tree);
     auto cloned_tree = dynamic_cast<ChoiceTree* >(ps_tree->Clone());
     GranulePageSizeTree extra_granule_tree(EPageGranuleType::G4K, cloned_tree);
     auto virt_usable = new ConstraintSet("0x0-0x100000,0xffffffffff000000-0xffffffffffffffff");
     FreePageCrossRangeResolver resolver(virt_usable, granule_tree, extra_granule_tree);

     SECTION ("test request ranges") {
       ConstraintSet request_range("0x0-0x1000,0xffffffffff700000-0xffffffffffffffff");
       vector<uint64> request_page_sizes = {0, 0, 0};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == true);
       EXPECT(resolved_page_sizes.size() == 3u);
       EXPECT(resolved_page_sizes[2] == 0x1000u);
       LOG(notice) << "start address:0x"<< hex << start_addr << ",resolved ranges:" << resolved_ranges.ToSimpleString() << endl;
     }
     
     SECTION ("test request page size") {
       ConstraintSet request_range;
       vector<uint64> request_page_sizes = {0, 0x200000, 0};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == true);
       EXPECT(resolved_page_sizes.size() == 3u);
       EXPECT(resolved_page_sizes[1] == 0x200000u);
       EXPECT(resolved_page_sizes[2] == 0x1000u);
       LOG(notice) << "start address:0x"<< hex << start_addr << ",resolved ranges:" << resolved_ranges.ToSimpleString() << endl;
     }

     SECTION ("test request page size") {
       ConstraintSet request_range;
       vector<uint64> request_page_sizes = {0, 0, 0, 0x200000};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == false);
     }
   }
},

CASE( "Test Free Page Cross Range Resolver") {
  SETUP( "Set up Free Page Cross Range Resolver") {
     using namespace Force;
     using namespace std;

     auto ps_tree = new ChoiceTree("Page size#4K granule", 0, 10);
     auto choice = new Choice("4K", 0, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("2M", 1, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("1G", 2, 10);
     ps_tree->AddChoice(choice);

     GranulePageSizeTree granule_tree(EPageGranuleType::G4K, ps_tree);
     
     ps_tree = new ChoiceTree("Page size#16K granule", 0, 10);
     choice = new Choice("16K", 0, 10);
     ps_tree->AddChoice(choice);
     choice = new Choice("32M", 1, 10);
     ps_tree->AddChoice(choice);
     GranulePageSizeTree extra_granule_tree(EPageGranuleType::G4K, ps_tree);

     auto virt_usable = new ConstraintSet("0x0-0x400000,0xffffffffff000000-0xffffffffffffffff");
     FreePageCrossRangeResolver resolver(virt_usable, granule_tree, extra_granule_tree);
     SECTION ("test request page size") {
       ConstraintSet request_range;
       vector<uint64> request_page_sizes = {0, 0x200000, 0};
       uint64 start_addr; 
       ConstraintSet resolved_ranges;
       vector<uint64> resolved_page_sizes;

       auto resolved = resolver.ResolveFreePageRanges(request_range, request_page_sizes, start_addr, resolved_ranges, resolved_page_sizes);
       EXPECT(resolved == true);
       EXPECT(resolved_page_sizes.size() == 3u);
       EXPECT(resolved_page_sizes[0] == 0x4000u);
       EXPECT(resolved_page_sizes[1] == 0x200000u);
       EXPECT(resolved_page_sizes[2] == 0x1000u);
       LOG(notice) << "start address:0x"<< hex << start_addr << ",resolved ranges:" << resolved_ranges.ToSimpleString() << endl;
     } 
  }
},
};

int main( int argc, char * argv[] )
{
  Force::Logger::Initialize();
  Force::Random::Initialize();
  int ret = lest::run( specification, argc, argv );
  Force::Random::Destroy();
  Force::Logger::Destroy();
  return ret;

}      
