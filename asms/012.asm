.globl main
main:
.data
i:	.word	0
.data
y:	.word	0
.text
li	$t0, 1
sw	$t0, i
.text
li	$t0, 2
sw	$t0, y
loop1:
.text
lw	$t1, i
li	$t2, 1
.text
seq	$t3, $t1, $t2
.text
li	$t1, 2
lw	$t2, y
seq	$t4, $t1, $t2
and	$t0, $t3, $t4
beq	$t0, 1, while1
jal	exit1
while1:
.text
li	$t0, 4
sw	$t0, i
.text
lw	$t0, i
move	$a0, $t0
li	$v0, 1
syscall
.text
li	$t0, 8
sw	$t0, i
.data
msg1:	.asciiz	"\n "
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
lw	$t0, i
move	$a0, $t0
li	$v0, 1
syscall
.text
lw	$t1, i
li	$t2, 8
add	$t0, $t1, $t2
sw	$t0, i
jal	loop1
exit1:
.data
msg3:	.asciiz	"\n"
.text
la	$a0, msg3
li	$v0, 4
syscall
.text
lw	$t0, i
move	$a0, $t0
li	$v0, 1
syscall
