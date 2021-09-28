.globl main
main:
.data
msg0:	.asciiz	"Hello World!!!"
.text
la	$a0, msg0
li	$v0, 4
syscall
