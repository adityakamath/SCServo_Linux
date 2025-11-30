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
        .def("end", &SMS_STS::end)
        .def("writeByte", &SMS_STS::writeByte)
        .def("SyncWriteSpe", [](SMS_STS &self, nb::list py_ids, nb::list py_speeds, nb::list py_accs) {
            size_t n = py_ids.size();
            std::vector<u8> ids(n);
            std::vector<s16> speeds(n);
            std::vector<u8> accs(n);
            for (size_t i = 0; i < n; ++i) {
                ids[i] = nb::cast<u8>(py_ids[i]);
                speeds[i] = nb::cast<s16>(py_speeds[i]);
                accs[i] = nb::cast<u8>(py_accs[i]);
            }
            self.SyncWriteSpe(ids.data(), n, speeds.data(), accs.data());
        })
        .def("EnableTorque", &SMS_STS::EnableTorque)
        .def("FeedBack", &SMS_STS::FeedBack)
        .def("ReadSpeed", &SMS_STS::ReadSpeed);

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
