.globl main
main:
.data
i:	.word	0
.data
y:	.word	0
.text
li	$t0, 10
sw	$t0, i
.text
li	$t0, 2
sw	$t0, y
loop1:
.text
lw	$t1, i
li	$t2, 1
.text
sge	$t3, $t1, $t2
.text
lw	$t1, y
li	$t2, 20
slt	$t4, $t1, $t2
or	$t0, $t3, $t4
beq	$t0, 1, while1
jal	exit1
while1:
.text
lw	$t1, i
li	$t2, 1
sub	$t0, $t1, $t2
sw	$t0, i
.text
lw	$t0, i
move	$a0, $t0
li	$v0, 1
syscall
.data
msg1:	.asciiz	"<- i"
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
lw	$t1, y
li	$t2, 2
add	$t0, $t1, $t2
sw	$t0, y
.data
msg2:	.asciiz	"\n"
.text
la	$a0, msg2
li	$v0, 4
syscall
.data
msg3:	.asciiz	"y ->"
.text
la	$a0, msg3
li	$v0, 4
syscall
.text
lw	$t0, y
move	$a0, $t0
li	$v0, 1
syscall
.data
msg4:	.asciiz	"  \n"
.text
la	$a0, msg4
li	$v0, 4
syscall
jal	loop1
exit1:
