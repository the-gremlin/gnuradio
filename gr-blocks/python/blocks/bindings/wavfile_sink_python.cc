/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(wavfile_sink.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(0d458d052ee9b31a6407864863e05f28)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/blocks/wavfile_sink.h>
// pydoc.h is automatically generated in the build directory
#include <wavfile_sink_pydoc.h>

void bind_wavfile_sink(py::module& m)
{
    using wavfile_sink = ::gr::blocks::wavfile_sink;

    py::class_<wavfile_sink,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<wavfile_sink>>(m, "wavfile_sink", D(wavfile_sink))

        .def(py::init(&wavfile_sink::make),
             py::arg("filename"),
             py::arg("n_channels"),
             py::arg("sample_rate"),
             py::arg("format"),
             py::arg("subformat"),
             py::arg("append") = false,
             D(wavfile_sink, make))

        .def("open", &wavfile_sink::open, py::arg("filename"), D(wavfile_sink, open))

        .def("close", &wavfile_sink::close, D(wavfile_sink, close))

        .def("set_sample_rate",
             &wavfile_sink::set_sample_rate,
             py::arg("sample_rate"),
             D(wavfile_sink, set_sample_rate))

        .def("set_bits_per_sample",
             &wavfile_sink::set_bits_per_sample,
             py::arg("bits_per_sample"),
             D(wavfile_sink, set_bits_per_sample))

        .def("set_append",
             &wavfile_sink::set_append,
             py::arg("append"),
             D(wavfile_sink, set_append));
}
