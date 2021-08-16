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

instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}
void extract(int pc,uint32_t *op,uint32_t *rs,uint32_t *rt,uint32_t *rd,uint32_t *shamt,uint32_t *func,uint32_t *imm,uint32_t *target){
    *op=*rs=*rt=*rd=*shamt=*func=*imm=*target=0;
    instruction *instr=get_inst_info(pc);
    int o=*op=OPCODE(instr);
	if(!o||o>3){ //R or I
        *rs=RS(instr),*rt=RT(instr);
		if(!o)*rd=RD(instr),*shamt=SHAMT(instr),*func=FUNC(instr); //R
		else *imm=IMM(instr); //I
	}
	else *target=TARGET(instr); //J
}
void IF(){
    if(!FETCH_BIT){
        CURRENT_STATE.PIPE[IF_STAGE]=0;
        return;
    }
    if(FETCH_BIT>1)return;
    uint32_t pc=CURRENT_STATE.PC,op,rs,rt,rd,shamt,func,imm,target;
    extract(pc,&op,&rs,&rt,&rd,&shamt,&func,&imm,&target);
    CURRENT_STATE.PIPE[IF_STAGE]=pc;
    CURRENT_STATE.PC=pc+4;
}
void ID(){
    CURRENT_STATE.ID_EX_DEST=0;
    uint32_t pc=CURRENT_STATE.PIPE[ID_STAGE],op,rs,rt,rd,shamt,func,imm,target;
    if(!pc){
        CURRENT_STATE.ID_EX_REG1=CURRENT_STATE.ID_EX_REG2=CURRENT_STATE.ID_EX_IMM=0;
        return;
    }
    extract(pc,&op,&rs,&rt,&rd,&shamt,&func,&imm,&target);
    if(CURRENT_STATE.PIPE[EX_STAGE]){
        if(OPCODE(get_inst_info(CURRENT_STATE.PIPE[EX_STAGE]))==0x23){
            if(CURRENT_STATE.EX_MEM_DEST==rs||CURRENT_STATE.EX_MEM_DEST==rt){
                CURRENT_STATE.PIPE_STALL[IF_STAGE]=CURRENT_STATE.PIPE_STALL[ID_STAGE]=1;
                return;
            }
        }
    }
    CURRENT_STATE.ID_EX_REG1=CURRENT_STATE.REGS[rs],CURRENT_STATE.ID_EX_REG2=CURRENT_STATE.REGS[rt];
    if(!op){
        CURRENT_STATE.ID_EX_IMM=shamt;
        if(func==0x8)CURRENT_STATE.JUMP_PC=CURRENT_STATE.REGS[rs]; //JR
        else CURRENT_STATE.ID_EX_DEST=rd;
    }
    else{
        if(op>0x5&&op<0x2b)CURRENT_STATE.ID_EX_DEST=rt;
        CURRENT_STATE.ID_EX_IMM=imm;
        if(op==0x3)CURRENT_STATE.REGS[31]=pc+4; //JAL
        if(op==0x2||op==0x3)CURRENT_STATE.JUMP_PC=target*4; //J or JAL
    }
}
void EX(){
    uint32_t pc=CURRENT_STATE.PIPE[EX_STAGE],op,rs,rt,rd,shamt,func,imm,target;
    if(!pc){
        CURRENT_STATE.EX_MEM_ALU_OUT=CURRENT_STATE.EX_MEM_BR_TAKE=CURRENT_STATE.EX_MEM_BR_TARGET=CURRENT_STATE.EX_MEM_DEST=CURRENT_STATE.EX_MEM_W_VALUE=0;
        return;
    }
    extract(pc,&op,&rs,&rt,&rd,&shamt,&func,&imm,&target);
    if(rs)rs=CURRENT_STATE.EX_MEM_FORWARD_REG==rs?CURRENT_STATE.EX_MEM_FORWARD_VALUE:(CURRENT_STATE.MEM_WB_FORWARD_REG==rs?CURRENT_STATE.MEM_WB_FORWARD_VALUE:CURRENT_STATE.ID_EX_REG1);
    if(rt)rt=CURRENT_STATE.EX_MEM_FORWARD_REG==rt?CURRENT_STATE.EX_MEM_FORWARD_VALUE:(CURRENT_STATE.MEM_WB_FORWARD_REG==rt?CURRENT_STATE.MEM_WB_FORWARD_VALUE:CURRENT_STATE.ID_EX_REG2);
    imm=shamt=CURRENT_STATE.ID_EX_IMM;
    CURRENT_STATE.EX_MEM_BR_TAKE=0;
    if(op==0x9)CURRENT_STATE.EX_MEM_ALU_OUT=rs+imm;	 //ADDIU
	if(op==0xc)CURRENT_STATE.EX_MEM_ALU_OUT=rs&imm;	 //ANDI
	if(op==0xf)CURRENT_STATE.EX_MEM_ALU_OUT=imm<<16; //LUI
	if(op==0xd)CURRENT_STATE.EX_MEM_ALU_OUT=rs|imm;  //ORI
	if(op==0xb)CURRENT_STATE.EX_MEM_ALU_OUT=rs<imm;  //SLTIU
	if(op==0x4)CURRENT_STATE.EX_MEM_BR_TARGET=pc+imm*4+4,CURRENT_STATE.EX_MEM_BR_TAKE=(rs==rt); //BEQ
	if(op==0x5)CURRENT_STATE.EX_MEM_BR_TARGET=pc+imm*4+4,CURRENT_STATE.EX_MEM_BR_TAKE=(rs!=rt); //BNE
	if(op==0x0){
        if(func==0x0)CURRENT_STATE.EX_MEM_ALU_OUT=rt<<shamt; //SLL
        if(func==0x2)CURRENT_STATE.EX_MEM_ALU_OUT=rt>>shamt; //SRL
        if(func==0x8)CURRENT_STATE.PIPE[ID_STAGE]=0;         //JR
        if(func==0x21)CURRENT_STATE.EX_MEM_ALU_OUT=rs+rt;    //ADDU
        if(func==0x23)CURRENT_STATE.EX_MEM_ALU_OUT=rs-rt;    //SUBU
        if(func==0x24)CURRENT_STATE.EX_MEM_ALU_OUT=rs&rt;    //AND
        if(func==0x25)CURRENT_STATE.EX_MEM_ALU_OUT=rs|rt;    //OR
        if(func==0x27)CURRENT_STATE.EX_MEM_ALU_OUT=~(rs|rt); //NOR
        if(func==0x2B)CURRENT_STATE.EX_MEM_ALU_OUT=rs<rt;    //SLTU
    }
    if(op==0x2||op==0x3)CURRENT_STATE.PIPE[ID_STAGE]=0; //J or JAL
    if(op==0x23||op==0x2b)CURRENT_STATE.EX_MEM_W_VALUE=rs+imm; //LW or SW
    CURRENT_STATE.EX_MEM_DEST=CURRENT_STATE.ID_EX_DEST;
}
void MEM(){
    uint32_t pc=CURRENT_STATE.PIPE[MEM_STAGE],op,rs,rt,rd,shamt,func,imm,target;
    if(!pc){
        CURRENT_STATE.MEM_WB_DEST=CURRENT_STATE.MEM_WB_OUT=0;
        return;
    }
    extract(pc,&op,&rs,&rt,&rd,&shamt,&func,&imm,&target);
    CURRENT_STATE.MEM_WB_DEST=CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.MEM_WB_OUT=CURRENT_STATE.EX_MEM_ALU_OUT;
    if(CURRENT_STATE.EX_MEM_BR_TAKE){
        FETCH_BIT=CURRENT_STATE.PIPE[ID_STAGE]=CURRENT_STATE.PIPE[EX_STAGE]=0;
        CURRENT_STATE.BRANCH_PC=CURRENT_STATE.EX_MEM_BR_TARGET;
    }
    if(op==0x23)CURRENT_STATE.MEM_WB_OUT=mem_read_32(CURRENT_STATE.EX_MEM_W_VALUE); //LW
	if(op==0x2b)mem_write_32(CURRENT_STATE.EX_MEM_W_VALUE,CURRENT_STATE.MEM_WB_FORWARD_REG==rt?CURRENT_STATE.MEM_WB_FORWARD_VALUE:CURRENT_STATE.REGS[rt]); //SW
}
void WB(){
    uint32_t pc=CURRENT_STATE.PIPE[WB_STAGE],op,rs,rt,rd,shamt,func,imm,target;
    if(!pc)return;
    ++INSTRUCTION_COUNT;
    extract(pc,&op,&rs,&rt,&rd,&shamt,&func,&imm,&target);
    if((!op&&func!=0x8)||(0x5<op&&op<0x25))CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST]=CURRENT_STATE.MEM_WB_OUT;
}
void process_instruction(){
    int i;
    for(i=4;i--;)if(!CURRENT_STATE.PIPE_STALL[i])CURRENT_STATE.PIPE[i+1]=CURRENT_STATE.PIPE[i],CURRENT_STATE.PIPE[i]=0;
    FETCH_BIT=1;
    if(CURRENT_STATE.PIPE_STALL[IF_STAGE])FETCH_BIT=2;
    CURRENT_STATE.IF_PC=CURRENT_STATE.JUMP_PC=CURRENT_STATE.BRANCH_PC=0;
    CURRENT_STATE.PIPE_STALL[IF_STAGE]=CURRENT_STATE.PIPE_STALL[ID_STAGE]=0;
    WB();MEM();EX();ID();
    if((CURRENT_STATE.PC-MEM_TEXT_START)/4>=NUM_INST)FETCH_BIT=0;
    IF();
    if(CURRENT_STATE.JUMP_PC)CURRENT_STATE.PC=CURRENT_STATE.JUMP_PC;
    if(CURRENT_STATE.BRANCH_PC)CURRENT_STATE.PC=CURRENT_STATE.BRANCH_PC;
    CURRENT_STATE.EX_MEM_FORWARD_REG=CURRENT_STATE.EX_MEM_DEST;
    CURRENT_STATE.EX_MEM_FORWARD_VALUE=CURRENT_STATE.EX_MEM_ALU_OUT;
    CURRENT_STATE.MEM_WB_FORWARD_REG=CURRENT_STATE.MEM_WB_DEST;
    CURRENT_STATE.MEM_WB_FORWARD_VALUE=CURRENT_STATE.MEM_WB_OUT;
    if(!CURRENT_STATE.PIPE[IF_STAGE]&&!CURRENT_STATE.PIPE[ID_STAGE]&&!CURRENT_STATE.PIPE[EX_STAGE]&&!CURRENT_STATE.PIPE[MEM_STAGE]&&(CURRENT_STATE.PIPE[WB_STAGE]-MEM_TEXT_START)/4==NUM_INST-1)RUN_BIT=0;
}
