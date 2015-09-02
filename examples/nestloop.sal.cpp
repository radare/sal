/**
 *
 * This is a macrotized nested loop for sal
 *
 **/

#include "sal.cpp"

/* Nested loop */
fordown(3) loop:
  println 1
  fordown(3) loop2:
    push "  "
    println 2
    fordown(3) loop3:
      push "    "
      println 2
    endfor(loop3)
  endfor(loop2)
endfor(loop)
