#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATA 0x10000000
#define TEXT 0x400000
#define NA -1

struct Instruction{ //struct for MIPS instructions
	char inst[6],type;
	int op,funct;
}a[21]={{"addu" ,'R',0x00,0x21},
		{"and"  ,'R',0x00,0x24},
		{"nor"  ,'R',0x00,0x27},
		{"or"   ,'R',0x00,0x25},
		{"sltu" ,'R',0x00,0x2B},
		{"subu" ,'R',0x00,0x23},
		{"jr"   ,'R',0x00,0x08},
		{"sll"  ,'R',0x00,0x00},
		{"srl"  ,'R',0x00,0x02},
		{"addiu",'I',0x09,NA},
		{"andi" ,'I',0x0C,NA},
		{"ori"  ,'I',0x0D,NA},
		{"sltiu",'I',0x0B,NA},
		{"lw"   ,'I',0x23,NA},
		{"sw"   ,'I',0x2B,NA},
		{"lui"  ,'I',0x0F,NA},
		{"beq"  ,'I',0x04,NA},
		{"bne"  ,'I',0x05,NA},
		{"j"    ,'J',0x02,NA},
		{"jal"  ,'J',0x03,NA},
		{"la"   ,'P',0x00,NA}}; //pseudo instruction

struct Data{
	char *name;
	unsigned int nx,val;
}d[100000],t[100000];

char c[100000],*tmp[100000];
unsigned int ans[100000],chk[100000];
int di,ti,dn,tn,an=2,errn;
unsigned int htod(char *a){ //hexadecimal to decimal
	int i;
	unsigned int s=0;
	for(i=0;a[i];++i)s=(s<<4)+(a[i]<58?a[i]-48:a[i]-87);
	return s;
}
void dtob(unsigned int n){ //print decimal to binary
	int i;
	for(i=32;i--;)printf("%d",(n>>i)&1);
}
unsigned int stoi(char *c){
	if(c[1]=='x')return htod(c+2);
	return atoi(c);
}
unsigned int get_reg(){
	scanf("%s",c);
	int n=strlen(c)-1;
	if(c[n]==',')c[n]=0;
	return atoi(c+1);
}
unsigned int seek(char *c,int st){
	unsigned int i;
	if(st)for(i=0;i<dn;i=d[i].nx)if(!strcmp(c,d[i].name))return DATA+i*4;
	for(i=0;i<tn;i=t[i].nx)if(!strcmp(c,t[i].name))return TEXT+i*4;
	return 0;
}
unsigned int R(int k){ //op(6)|rs(5)|rt(5)|rd(5)|shamt(5)|funct(6)
	int i;
	unsigned int s=(a[k].op<<26)+a[k].funct,rs=0,rt=0,rd=0,shamt=0;
	if(k>6){ //sll, srl cases => op rd rt shamt
		rd=get_reg();
		rt=get_reg();
		scanf("%s",c);
		shamt=stoi(c);
	}
	else if(k>5){ //jr case => op rs
		rs=get_reg();
	}
	else{ //otherwise => op rd rs rt
		rd=get_reg();
		rs=get_reg();
		rt=get_reg();
	}
	s+=(rs<<21)+(rt<<16)+(rd<<11)+(shamt<<6);
	return s;
}
unsigned int I(int k,unsigned int pc){ //op(6)|rs(5)|rt(5)|imm(16)
	unsigned int s=(a[k].op<<26),rs=0,rt=0,imm=0;
	if(k>15){ //beq, bne cases => op rs rt offset
		rs=get_reg();
		rt=get_reg();
		scanf("%s",c);
		imm=seek(c,0)/4;
		if(!imm)errn=1;
		else imm-=pc+1;
	}
	else if(k>14){ //lui case => op rt imm
		rt=get_reg();
		scanf("%s",c);
		imm=stoi(c);
	}
	else if(k>12){ //lw, sw cases => op rt imm(rs)
		rt=get_reg();
		scanf("%s",c);
		char *p=strtok(c,"(");
		imm=stoi(p);
		p=strtok(NULL,")");
		rs=atoi(p+1);
	}
	else{ //otherwise => op rt rs imm
		rt=get_reg();
		rs=get_reg();
		scanf("%s",c);
		imm=stoi(c);
	}
	s+=(rs<<21)+(rt<<16)+(imm&65535);
	return s;
}
unsigned int J(int k){ //op(6)|addr(26)
	scanf("%s",c);
	unsigned int addr=seek(c,0)/4;
	if(!addr)errn=1;
	return (a[k].op<<26)+addr;
}
void P(){
	unsigned int rd=get_reg();
	scanf("%s",c);
	unsigned int addr=seek(c,1);
	//lui
	ans[++an]=(15<<26)+(rd<<16)+(addr>>16);
	if(addr&65535){ //ori
		ans[++an]=(13<<26)+(rd<<21)+(rd<<16)+(addr&65535);
		++tn;
	}
}
int main(int argc, char* argv[]){

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n");
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else{
		//For input file read (sample_input/example*.s)
		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));
		if(freopen(file, "r",stdin)==0){
			printf("File open Error!\n");
			exit(1);
		}

		int i,j,flag,n;
		for(;;){
			if(scanf("%s",c)!=1)break; //EOF
			//puts(c);
			if(c[0]=='.'){
				if(c[1]=='t')flag=0; //.text
				else if(c[1]=='d')flag=1; //.data
				else flag=2; //.word
				continue;
			}
			if(flag){
				if(flag<2){
					n=strlen(c)-1;
					c[n]=0;
					d[di=dn].name=(char *)malloc(n*sizeof(char));
					strcpy(d[dn].name,c);
					continue;
				}
				d[dn].val=stoi(c);
				d[di].nx=++dn;
				flag=1;
				continue;
			}
			n=strlen(c)-1;
			if(c[n]==':'){
				c[n]=0;
				t[ti=tn].name=(char *)malloc(n*sizeof(char));
				strcpy(t[tn].name,c);
				continue;
			}
			for(i=0;i<21;++i)if(!strcmp(a[i].inst,c))break;
			if(a[i].type=='R')ans[++an]=R(i);
			else if(a[i].type=='I'){
				ans[++an]=I(i,TEXT/4+tn);
				if(errn){
					chk[an]=1;
					n=strlen(c);
					tmp[an]=(char *)malloc(n*sizeof(char));
					strcpy(tmp[an],c);
					errn=0;
				}
			}
			else if(a[i].type=='J'){
				ans[++an]=J(i);
				if(errn){
					chk[an]=2;
					n=strlen(c);
					tmp[an]=(char *)malloc(n*sizeof(char));
					strcpy(tmp[an],c);
					errn=0;
				}
			}
			else P();
			t[ti].nx=++tn;
		}
		for(i=2;i<=an;++i)if(chk[i]){
			if(chk[i]<2)ans[i]+=(seek(tmp[i],0)/4-TEXT/4-i+2)&65535; //tn==an-3
			else ans[i]+=seek(tmp[i],0)/4;
		}
		for(i=0;i<dn;++i)ans[++an]=d[i].val;

		// For output file write
		file[strlen(file)-1] ='o';
		freopen(file,"w",stdout);

		ans[1]=tn*4;
		ans[2]=dn*4;
		for(i=1;i<=an;++i)dtob(ans[i]);
	}
	return 0;
}

