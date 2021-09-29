.globl main
main:
.data
n1:	.word	0
.data
n2:	.word	0
.data
res:	.word	0
.data
option:	.word	0
.data
control:	.word	0
.text
li	$t0, 1
sw	$t0, control
loop1:
.text
lw	$t1, control
li	$t2, 1
beq	$t1, $t2, while1
jal	exit1
while1:
.text
li	$v0, 5
syscall
sw	$v0, n1
.data
msg1:	.asciiz	"\n Agora, insira outro valor inteiro: "
.text
la	$a0, msg1
li	$v0, 4
syscall
.text
li	$v0, 5
syscall
sw	$v0, n2
.data
msg2:	.asciiz	"\n Selecione dentre as opções:"
.text
la	$a0, msg2
li	$v0, 4
syscall
.data
msg3:	.asciiz	"\n 0 - Soma | 1 - Subtração | 2 - Divisão | 3 - Multiplicação \n"
.text
la	$a0, msg3
li	$v0, 4
syscall
.text
li	$v0, 5
syscall
sw	$v0, option
.data
msg4:	.asciiz	"\n O Resultado de "
.text
la	$a0, msg4
li	$v0, 4
syscall
.text
lw	$t0, n1
move	$a0, $t0
li	$v0, 1
syscall
.text
lw	$t1, option
li	$t2, 0
beq	$t1, $t2, if2
jal	else2
if2:
.text
lw	$t1, n1
lw	$t2, n2
add	$t0, $t1, $t2
sw	$t0, res
.data
msg5:	.asciiz	" + "
.text
la	$a0, msg5
li	$v0, 4
syscall
else2:
.text
lw	$t1, option
li	$t2, 1
beq	$t1, $t2, if3
jal	else3
if3:
.text
lw	$t1, n1
lw	$t2, n2
sub	$t0, $t1, $t2
sw	$t0, res
.data
msg6:	.asciiz	" - "
.text
la	$a0, msg6
li	$v0, 4
syscall
else3:
.text
lw	$t1, option
li	$t2, 2
beq	$t1, $t2, if4
jal	else4
if4:
.text
lw	$t1, n1
lw	$t2, n2
div	$t1, $t2
mflo	$t0
sw	$t0, res
.data
msg7:	.asciiz	" / "
.text
la	$a0, msg7
li	$v0, 4
syscall
else4:
.text
lw	$t1, option
li	$t2, 3
beq	$t1, $t2, if5
jal	else5
if5:
.text
lw	$t1, n1
lw	$t2, n2
mul	$t0, $t1, $t2
sw	$t0, res
.data
msg8:	.asciiz	" * "
.text
la	$a0, msg8
li	$v0, 4
syscall
else5:
.text
lw	$t0, n2
move	$a0, $t0
li	$v0, 1
syscall
.data
msg9:	.asciiz	" é: "
.text
la	$a0, msg9
li	$v0, 4
syscall
.text
lw	$t0, res
move	$a0, $t0
li	$v0, 1
syscall
.data
msg10:	.asciiz	"\n\n Deseja realizar outra operação? (1 - sim | 0 - não) "
.text
la	$a0, msg10
li	$v0, 4
syscall
.text
li	$v0, 5
syscall
sw	$v0, control
.text
lw	$t1, control
li	$t2, 1
beq	$t1, $t2, if6
jal	else6
if6:
.data
msg11:	.asciiz	"\n Insira um valor inteiro: "
.text
la	$a0, msg11
li	$v0, 4
syscall
else6:
jal	loop1
exit1:
