#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include "SMS_STS.h"
#include "SMSCL.h"
#include "SMSBL.h"
#include "SCS.h"

namespace nb = nanobind;

NB_MODULE(scservo_python, m) {
    m.doc() = "Python bindings for SCServo_Linux core classes (safe constructors and methods only)";

    nb::class_<SMS_STS>(m, "SMS_STS")
        .def(nb::init<>())
        .def("begin", &SMS_STS::begin)
        .def("end", &SMS_STS::end);

    nb::class_<SMSCL>(m, "SMSCL")
        .def(nb::init<>())
        .def("begin", &SMSCL::begin)
        .def("end", &SMSCL::end);

    nb::class_<SMSBL>(m, "SMSBL")
        .def(nb::init<>())
        .def("EnableTorque", &SMSBL::EnableTorque);

    // SCS is abstract, do not bind

    nb::class_<SCSerial>(m, "SCSerial")
        .def(nb::init<>())
        .def("getErr", &SCSerial::getErr);
}
