#input： N in $s0
#output： fib(N) in $s0
.text
.globl __start
__start:
	ori $a0,$zero,10  #Call fib(N)
	jal fib  
	or $zero,$zero,$zero  #nop 
	or $s0,$zero,$v0

   	#Exit:
   	addiu $v0,$zero,10
   	syscall #Exit
 fib:
 	# N is passed in register $a0
 	
 	# s0 - saved N
 	# s1 - fib(N-1)
 	# s2 - fib(N-2)
 	
 	# save return address 帧指针 相当于push
 	addi $sp,$sp,-4
 	# return address
 	sw $ra,0($sp)   
 	
 	# Fib(0)case
 	or $v0,$zero,$zero
 	beq $a0,$zero,end
 	or $zero,$zero,$zero  #nop in delay slot
 	
 	# Fib(1)case
 	ori $v0,$zero,1
 	beq $a0,1,end
 	or $zero,$zero,$zero  #nop in delay slot
 	
 	# Fib(N)case
 	# recursion
 	
 	#要把s0,s1分别存到接下来的栈帧当中
 	addi $sp,$sp,-8
 	sw $s0,0($sp)
 	sw $s1,4($sp)

 	# recover N from a0
 	or $s0,$zero,$a0
 	
 	#get fib(N-1)
 	# $a0是传入函数的参数，$s0是保存N的寄存器
 	addi $a0,$s0,-1
 	jal fib
 	or $zero,$zero,$zero
 	or $s1,$v0,$zero
 	
 
 	addi $a0,$s0,-2
 	jal fib
 	or $zero,$zero,$zero
 	or $s2,$v0,$zero
 	
 	# Return fib(N-1)+fib(N-2)
 	add $v0,$s1,$s2
 	
 	# restore s0,s1,s2
 	lw $s0,0($sp)
 	lw $s1,4($sp)
 	addi $sp,$sp,8
 	
 end:	
 	lw $ra,0($sp)
	addi $sp,$sp,4
	jr $ra
	or $zero,$zero,$zero
	
 	 	
 	
 	
 	
 	
 	
 	
 	 	
 	 	 	 	
 		
 			
 				
 						