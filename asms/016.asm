.globl main
main:
.data
x:	.word	0
.data
y:	.word	0
.data
res:	.word	0
.text
li	$t0, 0
sw	$t0, res
.text
li	$v0, 5
syscall
sw	$v0, x
.data
msg1:	.asciiz	"\nAgora, digite outro número inteiro: "
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
div	$t1, $t2
mfhi	$t0
sw	$t0, res
.data
msg2:	.asciiz	"\nO resto da divisão é: "
.text
la	$a0, msg2
li	$v0, 4
syscall
.text
lw	$t0, res
move	$a0, $t0
li	$v0, 1
syscall
