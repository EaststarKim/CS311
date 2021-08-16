/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
	/** Implement this function */
    if(get_inst_info(CURRENT_STATE.PC)>&INST_INFO[NUM_INST-1]){
        RUN_BIT=0;
        return;
    }
    instruction instr=*get_inst_info(CURRENT_STATE.PC);
    uint32_t *pc=&CURRENT_STATE.PC,op=instr.opcode,*rs,*rt,*rd,shamt,func,imm,target;
	if(!op||op>3){ //R or I
        rs=&CURRENT_STATE.REGS[instr.r_t.r_i.rs];
		rt=&CURRENT_STATE.REGS[instr.r_t.r_i.rt];
		if(!op){ //R
			rd=&CURRENT_STATE.REGS[instr.r_t.r_i.r_i.r.rd];
			shamt=instr.r_t.r_i.r_i.r.shamt;
			func=instr.func_code;
		}
		else imm=instr.r_t.r_i.r_i.imm; //I
	}
	else target=instr.r_t.target; //J

    if(op==0x9)*rt=*rs+imm;	//ADDIU
	if(op==0xc)*rt=*rs&imm;	//ANDI
	if(op==0xf)*rt=imm<<16; //LUI
	if(op==0xd)*rt=*rs|imm; //ORI
	if(op==0xb)*rt=*rs<imm; //SLTIU
	if(op==0x23)*rt=mem_read_32(*rs+imm); //LW
	if(op==0x2b)mem_write_32(*rs+imm,*rt); //SW
	if(op==0x4)*pc+=*rs==*rt?imm*4:0;	//BEQ
	if(op==0x5)*pc+=*rs!=*rt?imm*4:0; //BNE

	if(op==0x0){
        if(func==0x0)*rd=*rt<<shamt; //SLL
        if(func==0x2)*rd=*rt>>shamt; //SRL
        if(func==0x8)*pc=*rs; //JR
        if(func==0x21)*rd=*rs+*rt; //ADDU
        if(func==0x23)*rd=*rs-*rt; //SUBU
        if(func==0x24)*rd=*rs&*rt; //AND
        if(func==0x25)*rd=*rs|*rt; //OR
        if(func==0x27)*rd=~(*rs|*rt); //NOR
        if(func==0x2B)*rd=*rs<*rt; //SLTU
    }
	
    if(op==0x2)*pc=target*4; //J
	if(op==0x3)CURRENT_STATE.REGS[31]=*pc+4,*pc=target*4; //JAL
    
    if(op>3||!op&&func!=8)*pc+=4;
}
