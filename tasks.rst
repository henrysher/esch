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

* Support unicode when reading source code.

  - [DONE] Support Unicode conversion.
  - [DONE] Support full esch_string interface.
  - [POSTPONE] Support token escape.
  - [DONE] Provide helper functions to recognize input character range. 
  - [POSTPONE] Support 0xFEFF Unicode indicator from input source code file.

* Infrastructure
  - [WORKING] Esch_config does not require esch_alloc.
  - [WORKING] Default also take esch_config.
