#include "sal.cpp"

eof:
  println("EOF");
  close(%r0)
  exit(0)

cannot_connect:
  println("Cannot connect to remote host")
  exit(1)

main:
  connect("nopcode.org",80)
    on_error(cannot_connect)
      is(%r0)
  write(%r0,"GET /blog/ HTTP/1.1\r\nHost: www.nopcode.org\r\n\r\n")

again:
  readln(%r0)
    on_error(eof)
      is (%r1)
  println(%r1)
  again
