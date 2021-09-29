.globl main
main:
.data
vet:	.word	0, 0, 0
.text
li	$t0, 4
sw	$t0, vet + 0
.text
li	$t0, 8
sw	$t0, vet + 4
.text
li	$t0, 16
sw	$t0, vet + 8
.text
lw	$t0, vet + 0
move	$a0, $t0
li	$v0, 1
syscall
.data
msg1:	.asciiz	"\n"
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
lw	$t0, vet + 4
move	$a0, $t0
li	$v0, 1
syscall
.data
msg2:	.asciiz	" \n"
.text
la	$a0, msg2
li	$v0, 4
syscall
.text
lw	$t0, vet + 8
move	$a0, $t0
li	$v0, 1
syscall
.data
msg3:	.asciiz	"  \n"
.text
la	$a0, msg3
li	$v0, 4
syscall
