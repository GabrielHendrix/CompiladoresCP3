.globl main
main:
.data
x:	.float	0.000000
.data
y:	.float	0.000000
.data
res:	.float	0.000000
.data
temp4:	.float	0.000000
.text
lwc1	$f0, temp4
swc1	$f0, res
.text
li	$v0, 6
syscall
swc1	$f0, x
.data
msg1:	.asciiz	"\nAgora, digite outro número (float): "
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
li	$v0, 6
syscall
swc1	$f0, y
.text
lwc1	$f1, x
.text
lwc1	$f2, y
div.s	$f0, $f1, $f2
mov.s	$f12, $f0
swc1	$f12, res
.data
msg2:	.asciiz	"\nO resultado para a divisão é: "
.text
la	$a0, msg2
li	$v0, 4
syscall
.text
lwc1	$f12, res
li	$v0, 2
syscall
