#include "sal.cpp"

eof:
  println("EOF");
  close(%r0)
  exit(0)

cannot_connect:
  println("Cannot connect to remote host")
  exit(1)

main:
  connect("radare.org",80)
    on_error(cannot_connect)
      is(%r0)
  write(%r0,"GET /r/ HTTP/1.0\r\nHost: www.radare.org\r\n\r\n")

again:
  readln(%r0)
    on_error(eof)
      is(%r1)
  print(%r1)
  again
