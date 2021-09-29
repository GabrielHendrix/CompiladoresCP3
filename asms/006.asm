.globl main
main:
.data
x:	.word	0
.data
y:	.word	0
.text
li	$v0, 5
syscall
sw	$v0, x
.data
msg1:	.asciiz	"\n Digite outro valor inteiro para comparação: "
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
li	$v0, 5
syscall
sw	$v0, y
.text
lw	$t1, x
lw	$t2, y
bge	$t1, $t2, if1
jal	else1
if1:
.data
msg2:	.asciiz	"x = "
.text
la	$a0, msg2
li	$v0, 4
syscall
.text
lw	$t0, x
move	$a0, $t0
li	$v0, 1
syscall
.data
msg3:	.asciiz	" é maior/igual que y = "
.text
la	$a0, msg3
li	$v0, 4
syscall
.text
lw	$t0, y
move	$a0, $t0
li	$v0, 1
syscall
jal	out1
else1:
.data
msg4:	.asciiz	"x =  "
.text
la	$a0, msg4
li	$v0, 4
syscall
.text
lw	$t0, x
move	$a0, $t0
li	$v0, 1
syscall
.data
msg5:	.asciiz	" é menor que y = "
.text
la	$a0, msg5
li	$v0, 4
syscall
.text
lw	$t0, y
move	$a0, $t0
li	$v0, 1
syscall
out1:
