======================
Task tracking board
======================

Reference:
https://bitbucket.org/fuzhouch/esch/issues?status=new&status=open

* Windows support
  - Build on Windows.
  - Known issue: Windows uses 16-bit wchar_t, it does not support all
    Unicode points.

* Support unicode when reading source code.

  - [DONE] Support Unicode conversion.
  - [DONE] Support full esch_string interface.
  - [POSTPONE] Support token escape.
  - [DONE] Provide helper functions to recognize input character range. 
  - Support 0xFEFF Unicode indicator from input source code file.

* Create basic data type and data structures.

  - Symbols and strings
  - List
  - Characters
  - Numbers
  - Lambda

* Create callback based parser object.
  - Can I do single-pass compiler?
