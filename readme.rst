.. vi:ft=rst:expandtab:shiftwidth=4:textwidth=72

=======================
Esch: Embedded Scheme
=======================

Introduction
==============

`Embedded SCHeme` is an implemetation of Scheme interpreter. It starts
from a subset of R6RS_ standard, and may add features in future
releases. The first goal is to make it as much compliant as R6RS_
standard.

The second goal of Esch is to make this implementation small enough to
be embedded into other programs. The code is written in C to keep
portability between compilers and operating systems.

While keeping the two above as committed goals, a third goal here is to
make Esch a compiler to generate bytecode for different virtual
machines. For example, it could be really nice to make Esch genreates
.pyc code so the script can be executed with Python. However, this will
not be implemented in a near future.

Build Esch
============

Esch uses SCons_ as build system. It allows Esch to be built cross
platforms.

Esch tries to stick to C89_ standard. Thus, Esch should be able to compile
with any C89_ compilant C compiler. The only two exceptions are:
C-binding and threading. Esch uses platform-dependent libraries to
implement dynamic library loading and threading.

Right now Esch supports building only on Mac. Linux and other Unix
family OS should be OK but not tested. Windows build support will be
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

To debug Esch code, use ``scons mode=debug`` instead of ``scons`` to build
project. Debug build removes optimization, enables assertion and adds
additional logging support.

When everything is done, user gets two binaries:

* A library, ``libesch.a``. This is a library to provide core features
  of Scheme interpretation and core library.
* An executable interpreter, ``esch``. This is the interpreter and
  debugger to execute given Scheme script.

NOTE: The interpreter is currently under implementing.

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
