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
#ifndef Force_PyConstraint_H
#define Force_PyConstraint_H

#include <Constraint.h>

#include <ThreadContext.h>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace Force {

  PYBIND11_MODULE(Constraint, mod) {
    py::class_<Constraint>(mod, "Constraint");

    py::class_<ValueConstraint, Constraint>(mod, "ValueConstraint")
      .def("lowerBound", &ValueConstraint::LowerBound, py::call_guard<ThreadContextNoAdvance>())
      .def("upperBound", &ValueConstraint::UpperBound, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<RangeConstraint, Constraint>(mod, "RangeConstraint")
      .def("lowerBound", &RangeConstraint::LowerBound, py::call_guard<ThreadContextNoAdvance>())
      .def("upperBound", &RangeConstraint::UpperBound, py::call_guard<ThreadContextNoAdvance>())
      ;

    py::class_<ConstraintSet>(mod, "ConstraintSet")
      .def(py::init<uint64, uint64>(), py::call_guard<ThreadContextNoAdvance>())
      .def(py::init<uint64>(), py::call_guard<ThreadContextNoAdvance>())
      .def(py::init<std::string>(), py::call_guard<ThreadContextNoAdvance>())
      .def(py::init(), py::call_guard<ThreadContextNoAdvance>())
      .def(py::init([](const ConstraintSet& rOther) {  // Defines copy constructor
          return rOther.Clone();
        }),
        py::call_guard<ThreadContextNoAdvance>())
      .def(py::self == py::self, py::call_guard<ThreadContextNoAdvance>())  // Binds operator ==
      .def(py::self != py::self, py::call_guard<ThreadContextNoAdvance>())  // Binds operator !=
      .def("isEmpty", &ConstraintSet::IsEmpty, py::call_guard<ThreadContextNoAdvance>())
      .def("clear", &ConstraintSet::Clear, py::call_guard<ThreadContextNoAdvance>())
      .def("size", &ConstraintSet::Size, py::call_guard<ThreadContextNoAdvance>())
      .def("lowerBound", &ConstraintSet::LowerBound, py::call_guard<ThreadContextNoAdvance>())
      .def("upperBound", &ConstraintSet::UpperBound, py::call_guard<ThreadContextNoAdvance>())
      .def("chooseValue", &ConstraintSet::ChooseValue, py::call_guard<ThreadContextNoAdvance>())
      .def("intersects", &ConstraintSet::Intersects, py::call_guard<ThreadContextNoAdvance>())
      .def("addRange", &ConstraintSet::AddRange, py::call_guard<ThreadContextNoAdvance>())
      .def("addValue", &ConstraintSet::AddValue, py::call_guard<ThreadContextNoAdvance>())
      .def("subRange", &ConstraintSet::SubRange, py::call_guard<ThreadContextNoAdvance>())
      .def("subValue", &ConstraintSet::SubValue, py::call_guard<ThreadContextNoAdvance>())
      .def("subConstraintSet", &ConstraintSet::SubConstraintSet, py::call_guard<ThreadContextNoAdvance>())
      .def("applyConstraintSet", &ConstraintSet::ApplyConstraintSet, py::call_guard<ThreadContextNoAdvance>())
      .def("mergeConstraintSet", &ConstraintSet::MergeConstraintSet, py::call_guard<ThreadContextNoAdvance>())
      .def("containsValue", &ConstraintSet::ContainsValue, py::call_guard<ThreadContextNoAdvance>())
      .def("containsRange", &ConstraintSet::ContainsRange, py::call_guard<ThreadContextNoAdvance>())
      .def("containsConstraintSet", &ConstraintSet::ContainsConstraintSet, py::call_guard<ThreadContextNoAdvance>())
      .def("shiftRight", &ConstraintSet::ShiftRight, py::call_guard<ThreadContextNoAdvance>())
      .def("alignWithSize", &ConstraintSet::AlignWithSize, py::call_guard<ThreadContextNoAdvance>())
      .def("alignOffsetWithSize", &ConstraintSet::AlignOffsetWithSize, py::call_guard<ThreadContextNoAdvance>())
      .def("alignWithPage", &ConstraintSet::AlignWithPage, py::call_guard<ThreadContextNoAdvance>())
      .def("getConstraints", &ConstraintSet::GetConstraints, py::return_value_policy::reference, py::call_guard<ThreadContextNoAdvance>())
      .def("__str__", &ConstraintSet::ToSimpleString, py::call_guard<ThreadContextNoAdvance>())  // Allows conversion with Python str() method
      ;
  }

}

#endif  // Force_PyConstraint_H
