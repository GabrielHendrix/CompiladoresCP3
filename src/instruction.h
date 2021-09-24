
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

// Basic arch: 32 int registers and 32 float registers.
#define INT_REGS_COUNT   32     // i0 to i31: int registers.
#define FLOAT_REGS_COUNT 32     // f0 to f31: float registers.
// The machine also has a dedicated program counter (PC) register.

// Memory is split between data and instruction memory.
// This is called the Harvard architecture, in contrast to the von Neumann
// (stored program) architecture.
#define INSTR_MEM_SIZE  1024    // instr_mem[]
#define DATA_MEM_SIZE   1024    // data_mem[]
// The machine also has a string table str_tab[] for storing strings with
// the command SSTR. Maximum size for each string is 128 chars.

// Opcodes.
typedef enum {
    // Arith ops
    ADD,            // add $1,$2,$3         $1=$2+$3
    SUB,            // sub $1,$2,$3         $1=$2-$3 
    ADDI,           // addi $1,$2,100       $1=$2+100                   "Immediate" means a constant number
    SUBi,           // subi $1,$2,100       $1=$2-100                   "Immediate" means a constant number 
    ADDu,           // addu $1,$2,$3        $1=$2+$3                    Values are treated as unsigned 
                    //                                                  integers, not two's complement 
                    //                                                  integers 
    SUBu,           // subu $1,$2,$3        $1=$2-$3                    Values are treated as unsigned
                    //                                                  integers, not two's complement 
                    //                                                  integers
    ADDiu,          // addiu $1,$2,100      $1=$2+100                   Values are treated as unsigned 
                    //                                                  integers, not two's complement 
                    //                                                  integers 
    SUBiu,          // subiu $1,$2,100      $1=$2-100                   Values are treated as unsigned 
                    //                                                  integers, not two's complement 
                    //                                                  integers 
    MUL,            // mul $1,$2,$3         $1=$2*$3                    Result is only 32 bits!
    MULT,           // mult $2,$3           $hi,$low=$2*$3              Upper 32 bits stored in special 
                    //                                                  register hi Lower 32 bits stored 
                    //                                                  in special register lo 
    DIV,            // div $2,$3            $hi,$low=$2/$3              Remainder stored in special register
                    //                                                  hi Quotient stored in special registerlo

    // Logic ops        
    AND,            // and $1,$2,$3         $1=$2&$3                    Bitwise AND 
    OR,             // or $1,$2,$3          $1=$2|$3                    Bitwise OR
    ANDi,           // andi $1,$2,100       $1=$2&100                   Bitwise AND with immediate value
    ORi,            // or $1,$2,100         $1=$2|100                   Bitwise OR with immediate value
    SLL,            // sll $1,$2,10         $1=$2<<10                   Shift left by constant number of bits 
    SRL,            // srl $1,$2,10         $1=$2>>10                   Shift right by constant number of bits

    // Data Transfer ops        
    LW,             // lw $1,100($2)        $1=Memory[$2+100]           Copy from memory to register 
    SW,             // sw $1,100($2)        Memory[$2+100]=$1           Copy from register to memory 
    LUi,            // lui $1,100           $1=100x2^16                 Load constant into upper 16 
                    //                                                  bits. Lower 16 bits are set 
                    //                                                  to zero. 
    LA,             // la $1,label          $1=Address of label         Pseudo-instruction (provided 
                    //                                                  by assembler, not processor!) 
                    //                                                  Loads computed address of label 
                    //                                                  (not its contents) into register
    LI,             // li $1,100            $1=100                      Pseudo-instruction (provided by 
                    //                                                  assembler, not processor!) Loads 
                    //                                                  immediate value into register 
    MFHI,           // mfhi $2              $2=hi                       Copy from special register hi to 
                    //                                                  general register
    MFLO,           // mflo $2              $2=lo                       Copy from special register lo to 
                    //                                                  general register
    MOVE,           // move $1,$2           $1=$2                       Pseudo-instruction (provided by 
                    //                                                  assembler, not processor!) Copy 
                    //                                                  from register to register. 

    // Conditional Branch ops
    BEQ,            // beq $1,$2,100        if($1==$2) go to PC+4+100   Test if registers are equal 
    
    BNE,            // bne $1,$2,100        if($1!=$2) go to PC+4+100   Test if registers are not equal 
    BGT,            // bgt $1,$2,100        if($1>$2) go to PC+4+100    Pseduo-instruction 
    BGE,            // bge $1,$2,100        if($1>=$2) go to PC+4+100   Pseduo-instruction 
    BLT,            // blt $1,$2,100        if($1<$2) go to PC+4+100    Pseduo-instruction 
    BLE,            // ble $1,$2,100        if($1<=$2) go to PC+4+100   Pseduo-instruction

    // Comparison ops
    SLT,            // slt $1,$2,$3         if($2<$3)$1=1; else $1=0    Test if less than. If true, 
                    //                                                  set $1 to 1. Otherwise, set 
                    //                                                  $1 to 0. 
    SLTi,           // slti $1,$2,100       if($2<100)$1=1; else $1=0   Test if less than. If true, 
                    //                                                  set $1 to 1. Otherwise, set 
                    //                                                  $1 to 0.

    // Unconditional Jump 
    JUMP,           // j 1000               go to address 1000          Jump to target address 
    JR,             // jr $1                go to address stored in $1  For switch, procedure return 
    JAL,            // jal 1000             $ra=PC+4;go to address 1000 Use when making procedure call. 
                    //                                                  This saves the return address in $ra 
    TEXT,           // 
    DATA,           //
    WORD,
    HALF,
    BYTE,
    ASCII,
    ASCIIZ,
    SPACE,
    ALIGN,
    GLOBL,
    B2Ss,           // B2Ss ix, iy          ; ix <- @str_tab <- register iy (as str)
        // Integer to String
    I2Ss,           // I2Ss ix, iy          ; ix <- @str_tab <- register iy (as str)
        // Real to String
    R2Ss,           // R2Ss ix, fy          ; ix <- @str_tab <- register fy (as str)

} OpCode;

    // System Calls 
    
    // Service      | Operation                                 | Code (in $v0) | Arguments                         | Results 
    // print_int    | Print integer number (32 bit)             |       1       | $a0 = integer to be printed       | None 
    // print_float  | Print floating-point number (32 bit)      |       2       | $f12 = float to be printed        | None 
    // print_double | Print floating-point number (64 bit)      |       3       | $f12 = double to be printed       | None 
    // print_string | Print null-terminated character string    |       4       | $a0 = address of string in memory | None 
    // read_int     | Read integer number from user             |       5       | None                              | Integer returned in $v0 
    // read_float   | Read floating-point number from user      |       6       | None                              | Float returned in $f0 
    // read_double  | Read double floating-point number from    |       7       | None                              | Double returned in $f0 
    //                user
    // read_string  | Works the same as Standard C Library      |       8       | $a0 = memory address of string    | None
    //                fgets() function.                                           input buffer $a1 = length of 
    //                                                                            string buffer (n)
    // sbrk         | Returns the address to a block of memory  |       9       | $a0 = amount                      | address in $v0 
    //                containing n additional bytes. (Useful
    //                for dynamic memory allocation)
    // exit         | Stop program from running                 |       10      | None                              | None 
    // print_char   | Print character                           |       11      | $a0 = character to be printed     | None 
    // read_char    | Read character from user                  |       12      | None                              | Char returned in $v0 
    // exit2        | Stops program from running and  returns   |       17      | $a0 = result (integer number)     | None
    //                an integer


    // Assembler Directives 
 
    // Directive          | Result 
    // .word w1, ..., wn  | Store n 32-bit values in successive memory words 
    // .half h1, ..., hn  | Store n 16-bit values in successive memory words 
    // .byte b1, ..., bn  | Store n 8-bit values in successive memory words 
    // .ascii str         | Store the ASCII string str in memory. 
    //                      Strings are in double-quotes, i.e. "Computer Science" 
    // .asciiz str        | Store the ASCII string str in memory and null-terminate it 
    //                      Strings are in double-quotes, i.e. "Computer Science" 
    // .space n           | Leave an empty n-byte region of memory for later use 
    // .align n           | Align the next datum on a 2^n byte boundary. 
    //                      For example, .align 2 aligns the next value on a word boundary


    //Registers 
    
    // Register        | Register  | Description
    // Number          | Name  
    // 0               | $zero     | The value 0 
    // 2-3             | $v0 - $v1 | (values) from expression evaluation and function results 
    // 4-7             | $a0 - $a3 | (arguments) First four parameters for subroutine 
    // 8-15, 24-25     | $t0 - $t9 | Temporary variables 
    // 16-23           | $s0 - $s7 | Saved values representing final computed results 
    // 28              | $gp       | Global Pointer
    // 29              | $sp       | Stack Pointer
    // 30              | $fp       | Frame Pointer 
    // 31              | $ra       | Return address 

// String representations of opcodes.
static char* OpStr[] = {
    "add", "sub", "addi", "subi", "addu", "subu", "addiu", "subiu", "mul", "mult", "div",
    "and", "or", "andi", "ori", "sll", "srl",
    "lw", "sw", "lui", "la", "li", "mfhi", "mflo", "move",
    "beq", "bne", "bgt", "bge", "blt", "ble",
    "slt", "slti",
    "j", "jr", "jal",
    ".text", ".data", ".word", ".half", ".byte", ".ascii", ".asciiz", ".space", ".align", ".globl",
    "B2Ss", "I2Ss", "R2Ss",
};     
       
// Number of arguments for each opcode.
static int OpCount[] = {
    3,  // add  
    3,  // sub  
    3,  // addi
    3,  // subi 
    3,  // addu 
    3,  // subu 
    3,  // addiu
    3,  // subiu
    3,  // mul  
    2,  // mult 
    2,  // div
    3,  // and
    3,  // or  
    3,  // andi
    3,  // ori  
    3,  // sll  
    3,  // srl
    3,  // lw   
    3,  // sw   
    2,  // lui  
    2,  // la   
    2,  // li   
    1,  // mfhi 
    1,  // mflo 
    2,  // move
    3,  // beq  
    3,  // bne  
    3,  // bgt  
    3,  // bge 
    3,  // blt  
    3,  // ble
    3,  // slt  
    3,  // slti 
    1,  // jump
    1,  // jr  
    1,  // jal
    0,  // .text
    0,  // .data  
    0,  // .word
    0,  // .half
    0,  // .byte
    0,  // .ascii
    0,  // .asciiz
    0,  // .space 
    0,  // .align
    1,  // .globl
    2,  // B2Ss
    2,  // I2Ss
    2,  // R2Ss
};

// Instruction quadruple.
typedef struct {
    OpCode op;
    int o1;     // Operands, which can be int or float registers,
    int o2;     // int addresses or offsets, or
    int o3;     // integer or float constants (must be in an integer repr.)
} Instr;

#endif
