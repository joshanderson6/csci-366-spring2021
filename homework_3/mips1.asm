.text
li $t0, 1
li $t1, 11
loop:
	li $v0, 1 # print int
	move $a0, $t0
	syscall
	addi $t0, $t0, 1
	blt $t0, $t1, loop
	
	li $v0, 10 # exit
	syscall
	
