
::

  program ::= comment | datum

  datum ::= lexeme-datum | compound datum

  lexeme-datum ::= boolean | number | character | string | symbol

  symbol ::= identifier

  compound-datum ::= list | vector | bytevector

  list ::= ( datum* ) | [ datum* ] | 
           ( datum . datum ) | [ datum . datum ] |
           abbreviation

  abbriviation ::= abbrev-prefix datum

  abbrev-prefix ::= ' | ` | , | ,@ | #` | #` | #, | #,@

  vector ::= #( datum* )

  bytevector ::= #vu8( u8* )

  u8 ::= any number prepresenting an exact integer in { 0, .. 255 }
