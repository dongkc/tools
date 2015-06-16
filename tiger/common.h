//****************************************************************************
// Model: pelican.qm
// File:  ./common.h
//
// This code has been generated by QM tool (see state-machine.com/qm).
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//****************************************************************************
//${.::common.h} .............................................................
#ifndef COMMON_H
#define COMMON_H

namespace PELICAN {

enum PelicanSignals {
    PEDS_WAITING_SIG = QP::Q_USER_SIG,
    TERMINATE_SIG,
    MAX_PUB_SIG, // the last published signal

    ON_SIG,
    OFF_SIG,
    TIMEOUT_SIG,

    MAX_SIG   // keep always last
};

}

#endif