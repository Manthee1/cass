; This test no worky becauze DAta pointer thingy is not implemented yet
.text
text hw "Hello, world!\n" 14

.data
start:
start:
movc $1, 9
mov $0, $1
movd $2, #hw
OUT 0, $2