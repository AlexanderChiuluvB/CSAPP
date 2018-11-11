.data
prompt1: .asciiz "Enter the sequence index\n"
prompt2: .asciiz "The Fibonacci value is:\n"
message: .asciiz "The Fibonacci value is:\n0"

.text
#print prompt1
li $v0,4
la $a0,prompt1
syscall 

#read integer
li $v0,5
syscall
#读出来的数据就放在v0

beq $v0,0,equalToZero
#$v0, $v1 are function result registers with $v0 being 32-bit, $v1 64bit
#$a0-$a3 是参数寄存器

move $a0,$v0
jal fib
move $a1,$v0

#print prompt2
li $v0,4
la $a0,prompt2
syscall

#print result
li $v0,1
move $a0,$a1
syscall

#Exit
li $v0,10
syscall

fib:
	addi $sp,$sp,-12
	sw $ra,8($sp)
	sw $s0,4($sp)
	sw $s1,0($sp)
	move $s0,$a0
	li $v0,1 #return value for terminal condition
	ble $s0,0x2,fibexit
	addi $a0,$s0,-1 #set args for recursive call to f(n-1)
	jal fib
	move $s1,$v0 #store result of f(n-1) to s1
	addi $a0,$s0,-2 #set args for recursive call to f(n-2)
	jal fib
	add $v0,$s1,$v0 # add result of f(n-1) to it
fibexit:
	
	lw $ra,8($sp)
	lw $s0,4($sp)
	lw $s1,0($sp)
	addi $sp,$sp,12
	jr $ra
equalToZero:
	li $v0,4
	la $a0,message
	syscall
	
	
	
	
	
	
	
	
	

