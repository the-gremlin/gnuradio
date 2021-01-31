/*
 * Copyright 2021 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(pdu.h)                                                     */
/* BINDTOOL_HEADER_FILE_HASH(34ae96025ac87967432c6b7092d1fd60)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/pdu/pdu.h>
// pydoc.h is automatically generated in the build directory
#include <pdu_pydoc.h>

void bind_pdu(py::module& m)
{


    m.def("PMTCONSTSTR__data", &::gr::pdu::PMTCONSTSTR__data, D(PMTCONSTSTR__data));


    m.def("PMTCONSTSTR__dict", &::gr::pdu::PMTCONSTSTR__dict, D(PMTCONSTSTR__dict));


    m.def("PMTCONSTSTR__emit", &::gr::pdu::PMTCONSTSTR__emit, D(PMTCONSTSTR__emit));


    m.def("PMTCONSTSTR__msg", &::gr::pdu::PMTCONSTSTR__msg, D(PMTCONSTSTR__msg));


    m.def("PMTCONSTSTR__pdu", &::gr::pdu::PMTCONSTSTR__pdus, D(PMTCONSTSTR__pdus));
}