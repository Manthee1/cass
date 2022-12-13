.data
str info ". Fibonacci num: "
.code
start:
movc $1, 0
movc $2, 1
movc $4, 1
movc $5, 1
loop:
ADD $1, $2
MOV $3, $1
OUT $4
OUTS #info
OUT $3
OUTN 
ADD $4, $5
MOV $1, $2
MOV $2, $3
JMP loop

