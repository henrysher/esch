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

Esch tries to stick to C89_ standard. Thus, Esch should be able to compile
with any C89_ compilant C compiler. The only two exceptions are:
C-binding and threading. Esch uses platform-dependent libraries to
implement dynamic library loading and threading.

Right now Esch supports building only on Mac. Linux and other Unix
family should be supported but not tested. Windows build support will be
added later.

To build Esch, make sure to install the following dependencies:

* Mercurial_ to download source code.
* A C89_ compilant C compiler toolchain.
* Scons_ to build source code.
* Python_ runtime (needed by Scons_).

When everything is downloaded, do the following:

::

  hg clone https://fuzhouch@bitbucket.org/fuzhouch/esch
  cd esch
  scons

An output of build result looks like below (assume you are with Unix):

::

  scons: Reading SConscript files ...
  Build mode = release
  scons: done reading SConscript files.
  scons: Building targets ...
  gcc -o src/esch_alloc.o -c -std=c89 -O2 -DNDEBUG -Isrc src/esch_alloc.c
  gcc -o src/esch_log.o -c -std=c89 -O2 -DNDEBUG -Isrc src/esch_log.c
  gcc -o src/utest/esch_t_alloc.o -c -std=c89 -O2 -DNDEBUG -Isrc src/utest/esch_t_alloc.c
  gcc -o src/utest/esch_utest.o -c -std=c89 -O2 -DNDEBUG -Isrc src/utest/esch_utest.c
  ar rc src/libesch.a src/esch_alloc.o src/esch_log.o
  ranlib src/libesch.a
  gcc -o src/esch_utest src/utest/esch_t_alloc.o src/utest/esch_utest.o -Lsrc -lesch
  scons: done building targets.

Now, copy ``src/esch.h`` and ``src/libesch.a`` to your project. Start
coding.

To debug in Esch, use ``scons mode=debug`` instead of ``scons`` to build
project. It will remove optimization, enable assertion and add
additional logging support.

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
* Compelete build system. Support building on Mac.
* Implement language parser.

  - Define variables.
  - If statement.
  - Function definition support.
  - Symbols support.
  - List support.
  - Write to standard output.

* Implement a basic runtime engine.


Esch v1.0
------------

* Implement full R6RS_ language and library specification.


.. _R6RS : http://www.r6rs.org
.. _SCons : http://www.scons.org
.. _C89 : http://en.wikipedia.org/wiki/ANSI_C
.. _Python : http://www.python.org
.. _Mercurial : http://mercurial.selenic.com/
