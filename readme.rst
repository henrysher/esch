.. vi:ft=rst:expandtab:shiftwidth=4:textwidth=72

=======================
Esch: Embedded Scheme
=======================

Introduction
==============

`Embedded SCHeme` tries to provide an R6RS_ compilant Scheme language
interpreter. It tries provide two products:

* A library, ``libesch.a``. This is a library to provide core features
  of Scheme interpretation and core library.
* An executable interpreter, ``esch``. This is the interpreter and
  debugger to execute given Scheme script.

Esch is implemented in so called Clean C89, which means it should be
able to be ported to different operating systems.

Build Esch
============

Esch uses SCons_ as build system. It allows Esch to be built cross
platforms.

Esch tries to follow C89_ standard. Thus, Esch should be able to compile
with any C89_ compilant C compiler. The only two exceptions are:
C-binding and threading. Esch uses platform-dependent libraries to
implement dynamic library loading and threading

Run Scheme scripts with Esch
===============================

TBD

Embedded Esch in C/C++
========================

TBD

Roadmap
=========

Esch v0.1
------------

* Implement source code structure.
* Compelete build system.
* Implement language parser.

  - Define variables.
  - Function definition support.
  - Symbols support.
  - Write to standard output.

* Implement a basic runtime engine.


Esch v1.0
------------

* Implement full R6RS_ language and library specification.


.. _R6RS : http://www.r6rs.org
.. _SCons : http://www.scons.org
.. _C89 : http://en.wikipedia.org/wiki/ANSI_C
