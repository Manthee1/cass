.data
start:
movc $1, 0
movc $2, 1
loop:
ADD $1, $2
MOV $3, $1
OUT $3
OUTN 
MOV $1, $2
MOV $2, $3
JMP loop

