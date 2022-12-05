.data
start:
movc $1, 1
movc $2, 1
main:
IN 0, $1
IN 0, $2
ADD $1, $2
OUT 0, $1
JMP main

