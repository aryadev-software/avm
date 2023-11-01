  malloc.byte 3
  mov.word 0
  push.reg.word 0
  push.byte 'a'
  mset.byte 0
  push.reg.word 0
  push.byte 'b'
  mset.byte 1
  push.reg.word 0
  push.byte 'c'
  mset.byte 2

  push.reg.word 0
  msize
  malloc.stack.byte
  mov.word 1
  push.word 0
  mov.word 2
  push.reg.word 0
  push.reg.word 2
  push.word 1
  push.reg.word 0
  msize
  sub.word
  sub.word
  mget.stack.byte
  mov.byte 24
  push.reg.word 1
  push.reg.byte 24
  push.reg.word 2
  mset.stack.byte
  push.reg.word 2
  push.word 1
  plus.word
  mov.word 2
  push.reg.word 2
  push.reg.word 0
  msize
  eq.word
  not.byte
  jump.if.byte 17
  halt
