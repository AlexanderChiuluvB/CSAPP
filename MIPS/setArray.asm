########



void set_array(int num)
{  int array[10];  for(int i=0; i<10; i++)
			{   array[i] = compare(num, i);  } 
} 

int compare(int a, int b)
{  if(sub(a,b) >= 0) 
  return 1;  else   return 0; }
  
 int sub(int a, int b){
   return a-b; } 




###########
set_array:
	addi $sp,$sp,-12
	sw   $ra,8($sp)
	sw   $s0,4($sp)
	sw   $s1,0($sp)
	addi $s0,$zero,0
loop:
	slti $t0,$s0,10
	beq  $t0,$zero,L1
	add  $a0,$s0,$zero #pass the parameter
	jal compare
	sll  $t2,$s0,2
	la   $s1,array
	add  $t1,$s1,$t2 #&Array[i]
	sw   $v0,0($t1)  # store the comapre result
	addi  $s0,$s0,1
	j loop
L1:
	lw   $ra,8($sp)
	lw   $s0,4($sp)
	lw   $s1,0($sp)
	addi $sp,$sp,12
	jr $ra
compare:
	addi $sp,$sp,-4
	sw  $ra,0($sp)
	jal Sub
	slt $t3,$v0,$zero
	beq $t3,$zero,L2
	add $v0,$zero,$zero
	j done
L2:	
	addi $v0,$zero,1
done:
	lw $ra,0($sp)
	addi $sp,$sp,4
	jr $ra
Sub:
	addi $v0,$a0,$a1
	jr $ra
	