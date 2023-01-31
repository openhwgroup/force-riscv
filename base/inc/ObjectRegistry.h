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
#ifndef Force_ObjectRegistry_H
#define Force_ObjectRegistry_H

#include <map>

#include "Object.h"

namespace Force {

  /*!
    \class ObjectRegistry
    \brief Container of object templates for all light-weight objects derived from Object class.
   */
  class ObjectRegistry {
  public:
    static void Initialize();  //!< Initialization interface.
    static void Destroy();     //!< Destruction clean up interface.
    inline static ObjectRegistry* Instance() { return mspObjectRegistry; } //!< Access ObjectRegistry info instance.

    Object* ObjectInstance(const std::string& objType) const; //!< Return an instance of the Object type specified by objName.
    Object* ObjectInstanceTry(const std::string& objType) const; //!< Return an instance of the Object type specified by objName, return nullptr if not found.
    void RegisterObject(const Object* objPtr); //!< Register an Object type.

  private:
    ObjectRegistry(): mObjectRegistry()  {} //!< Private constructor.
    ObjectRegistry(const ObjectRegistry& rOther) : mObjectRegistry() {} //!< Private copy constructor
    ~ObjectRegistry(); //!< Private destructor.
    void ObjectNotFound(const std::string& objType) const; //!< Report an object of the type specified cannot be found.
  private:
    static ObjectRegistry* mspObjectRegistry;  //!< Pointer to singleton ObjectRegistry object.
    std::map<std::string, const Object*> mObjectRegistry; //!< Container of all light weight objects derived from Object class.

  public:

#ifndef UNIT_TEST

    void Setup(); //!< Add instances of light weight base layer Object derived classes to the ObjectRegistry

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, PhysicalRegister* pr_clone = ObjectRegistry::Instance()->TypeInstance<PhysicalRegister>(obj_type);
     */
    template<typename T>
      T* TypeInstance(const std::string& objType) const
      {
        const auto map_finder = mObjectRegistry.find(objType);
        if (map_finder == mObjectRegistry.end()) {
          ObjectNotFound(objType);
        }

        Object* obj_clone = map_finder->second->Clone();
        T* cast_obj = dynamic_cast<T* >(obj_clone);
        return cast_obj;
      }

    /*!
      Templated function so that a derived class can conveniently cast base class to the desired derived class type.
      For example, PhysicalRegister* pr_clone = ObjectRegistry::Instance()->TypeInstanceTry<PhysicalRegister>(obj_type);
      Will return nullptr if such class instance cannot be found.
     */
    template<typename T>
      T* TypeInstanceTry(const std::string& objType) const
      {
        const auto map_finder = mObjectRegistry.find(objType);
        if (map_finder == mObjectRegistry.end()) {
          return nullptr;
        }

        Object* obj_clone = map_finder->second->Clone();
        T* cast_obj = dynamic_cast<T* >(obj_clone);
        return cast_obj;
      }

#endif
  };

}

#endif
