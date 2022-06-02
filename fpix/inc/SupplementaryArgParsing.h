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
#ifndef SUPPLEMENTARY_ARG_PARSING_H
#define SUPPLEMENTARY_ARG_PARSING_H

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace Force{

    // Splits a string into a vector of substring using a character as a splitting token
    std::vector<std::string> split(char aSplitCharacter, std::string &aInputString){
        std::vector<std::string> split_string;
        std::size_t cursor_start = 0;

        //chomp the input string, splitting by the split character token
        while(cursor_start < aInputString.size()){
            auto it_at_token = std::find(aInputString.begin() + cursor_start, aInputString.end(), aSplitCharacter);    
            
            if(it_at_token == aInputString.end()){
                break;
            }
            else{
                std::string part(aInputString.begin() + cursor_start, it_at_token);
                split_string.push_back(part);
                cursor_start += part.size() + 1;
            }
        }

        //The string will probably not end with the split character
        if(cursor_start < aInputString.size()){
            std::string part(aInputString.begin() + cursor_start, aInputString.end());
            split_string.push_back(part);
        }

        return split_string;
    }

    void testForSplit(){
        std::string testString("blah=42");
        char splitChar = '=';

        std::vector<std::string> mySeparatedString(split(splitChar, testString));
    
        for(auto part : mySeparatedString){
            std::cout << "Testing split function: part: " << part << std::endl;
        }
    }

};




#endif


