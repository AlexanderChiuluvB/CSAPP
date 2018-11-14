.data
array: .space 1024
input_msg: .asciiz "Enter the number of integers:\n"
input_int_msg: .asciiz "Enter integers to be sorted\n"
output_msg: .asciiz "the sorted numbers are\n"


.text
.globl main

main:
	la $a0,input_msg
	li $v0,4
	syscall
	 
	li $v0,5  #接受用户输入的数组长度
	syscall
	

	
	la $t6,array #数组首地址
	move $t7,$zero #循环变量i
	move $t8,$zero #循环变量j
	move $t9,$v0  #数组长度

	addi $t3,$zero,1
	beq $v0,$t3,special
			
input:

	la $a0,input_int_msg
	li $v0,4
	syscall
	
	
	li $v0,5  
	syscall
	
	#for(int i=0;i<num;i++)
        #cin>>arr[i];
        
        move $t0,$t7 #$t0 is i
        sll $t0,$t0,2 #i become byte offset
        move $t1,$t6 #$t1 is &array[0]
        add $t1,$t1,$t0 #$t1 is &array[i]
        sw $v0,0($t1) #cin>>array[i]
	
	addi $t7,$t7,1
	blt $t7,$t9,input #if ++i<length
 
	move $t7,$zero
	
loop1:	# 每次外层循环比内层循环的循环变量设为0
	move $t8,$zero
loop2:
	#read arr[j+1] and arr[j]
	
	move $t2,$t8 #t2 is j
	sll $t2,$t2,2 #j=*4;
	addu $t1,$t2,$t6 #&arr[j]
	lw $t4,0($t1)  #arr[j]
	
	
	addi $t2,$t8,1 #j+1
	sll $t2,$t2,2 #(j+1)*4
	addu $t0,$t2,$t6 #&arr[j+1]
	lw   $t5,0($t0) #$t5 = arr[j+1]
	
	
	bge $t5,$t4,skip #if arr[j+1]>arr[j] skip

	sw  $t5,0($t1)
	sw  $t4,0($t0)
	
skip:
	
	
	addi $t8,$t8,1 #j++
	subi $t1,$t9,1 #t1 = num-1
	sub  $t1,$t1,$t7 #t1 = num-1-i
	blt  $t8,$t1,loop2 # if j<num-i-1 t2=1
	addi $t7,$t7,1 #i++
	sub  $t3,$t9,1
	blt  $t7,$t3,loop1  #if i<num-1 then continue loop1
	j output
	
special:
	la $a0,input_int_msg
	li $v0,4
	syscall
	
	
	li $v0,5  
	syscall
	
	#for(int i=0;i<num;i++)
        #cin>>arr[i];
        
        move $t0,$t7 #$t0 is i
        sll $t0,$t0,2 #i become byte offset
        move $t1,$t6 #$t1 is &array[0]
        add $t1,$t1,$t0 #$t1 is &array[i]
        sw $v0,0($t1) #cin>>array[i]
	
output:
	
	la $a0,output_msg
	li $v0,4
	syscall
	
	move $t7,$zero
print:
	
	move $t0,$t7
	
	sll $t0,$t0,2
	
	add $t1,$t6,$t0
	
	lw $a0,0($t1) #argument a0
	
	li $v0,1
	syscall
	
	addi $t7,$t7,1
	blt $t7,$t9,print
	
	
	                            
	
