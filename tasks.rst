======================
Task tracking board
======================

Reference:
https://bitbucket.org/fuzhouch/esch/issues?status=new&status=open

* Windows support
  - [DONE] Build on Windows.
  - [DONE] Known issue: Windows uses 16-bit wchar_t, it does not
    support all Unicode points.
  - Provide a build macro to remove support of unicode > 0xFFFF

* String

  - [DONE] Support Unicode conversion.
  - [DONE] Support full esch_string interface.
  - [DONE] Provide helper functions to recognize input character range. 
  - [THINKING] Totally remove wchat_t in code. Use pure UTF-8.

* Scheme identifier
  - [DONE] Support unicode identifier character checker.
  - [POSTPONE] Support token escape.
  - [BUG] Windows does not support UCS-4.
  - [POSTPONE] Support 0xFEFF Unicode indicator from input source code file.

* Infrastructure
  - [DONE] Esch_config does not require esch_alloc.
  - [DONE] All objects take esch_config during initialization.

* Vector
  - [DONE] Basic vector support: create, delete, append, access.

* BigInt
  - [THINKING] Basic feature.

* New Object model.
  - [DONE] Add esch_type to support defining new types.
  - [DONE] Update esch_object to use esch_type.
  - [DONE] Update esch_object to support GC.
  - [DONE] Update alloc/config/log types to use new object model.
  - [DONE] Add a dummy GC to unblock test.
  - [DROP] Update esch_integer
  - [DROP] Update esch_vector
  - [DONE] Update esch_string

* Implement a working mark-and-sweep GC.
  - [DONE] Basic logic for naive GC.
  - Use esch_iterator to iterate primitive and object.
  - Implement esch_vector (fix length array) and test.
  - Update GC to make unit test fully pass (esch_vector as gc root).

* Implement list type (requires GC for verification)

* Implement a memory-saving character type.

* Define module format
