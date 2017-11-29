#include<stdio.h>
#include<pthread.h>
#include<math.h>
#include<malloc.h>
#include<semaphore.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include "MMU.h"
#include "QUEUE.h"
char *MEMORY;//Main Memory of 300 Bytes 0-299.
float BtoF(const void *buf) 
{
    const unsigned char *b = (const unsigned char *)buf;
    uint32_t temp = 0;
    temp = ((b[0] << 24) |
            (b[1] << 16) |
            (b[2] <<  8) |
             b[3]);
    return temp;
}
int FtoB(void *buf, float x) 
{
    unsigned char *b = (unsigned char *)buf;
    unsigned char *p = (unsigned char *) &x;
	#if defined (_M_IX86) || (defined (CPU_FAMILY) && (CPU_FAMILY == I80X86))
    b[0] = p[3];
    b[1] = p[2];
    b[2] = p[1];
    b[3] = p[0];
	#else
    b[0] = p[0];
    b[1] = p[1];
    b[2] = p[2];
    b[3] = p[3];
	#endif
    return 4;
}
double BtoD(const void *buf) 
{
    const unsigned char *b = (const unsigned char *)buf;
    uint32_t temp = 0;
    temp = (((long long unsigned)b[0] << 56) |
			((long long unsigned)b[1] << 48) |
            ((long long unsigned)b[2] << 40) |
            ((long long unsigned)b[3] << 32) |
			(b[4] << 24) |
            (b[5] << 16) |
            (b[6] <<  8) |
             b[7]);
    return temp;
}
int DtoB(void *buf, double x) 
{
    unsigned char *b = (unsigned char *)buf;
    unsigned char *p = (unsigned char *) &x;
	#if defined (_M_IX86) || (defined (CPU_FAMILY) && (CPU_FAMILY == I80X86))
    
	b[0] = p[7];
    b[1] = p[6];
    b[2] = p[5];
    b[3] = p[4];
    b[4] = p[3];
    b[5] = p[2];
    b[6] = p[1];
    b[7] = p[0];
	#else
    b[0] = p[0];
    b[1] = p[1];
    b[2] = p[2];
    b[3] = p[3];
    b[4] = p[4];
    b[5] = p[5];
    b[6] = p[6];
    b[7] = p[7];
	#endif
    return 4;
}
void *parse(void *arg)
{
	int ii=*((int*) arg);
	char *name;
	if(ii==1)
	{
		name="One.txt";
	}
	else
	{
		name="Two.txt";
	}
	
	/*---------------Parser Variables------------------------------*/
	FILE *fp;
	fp=fopen(name,"r");
	int ch;
	int i=1;
	int flag_space=0;//Space detected
	int flag_eq=0;//Assignment using equals to sign
	int semcol=0;//Assignment using equals to sign
	int line_no=0;//line no
	int var_name_pos=0;//Variable name index in the instruction
	int space_pos=0;
	int pid=ii;//Which process numberis this
	/*Beginning of Variables' data in Memory 1 Byte for Name 1 Byte for Length and 1 Byte for Virtual Address
	So total 3 Bytes for each entry for a variable.As there is only one character varible name allowed there will be 26 Variables 
	with maximum size=8 Bytes(for Double) So jump of 3 to search for a name of var and
	Max entries needed=26 and total bytes for that=26*3=78 Bytes per process.
	After that comes the Page Table .
	There are 3 bits for a page# so total entries per process will be 8 and physical address has 2 bits for frame number and 
	5 bits for offsets.
	So each table entries will have 2 bits for flags 2 bits for frame number.
	Each page has 2^5 Bytes memory so each page is of size 32 Bytes and each process will have only 2 frame pages.
	*/
	int Var_ptr;  
	int PTP;//Location of Page Table Pointer in memory
	int JUMP=3;//3 Bytes of jump
	int Variable_counter=0;//How many variables have been stored.
	unsigned char VA_counter=0x00;//Which virtual address is this
	char *P_n[8];
	if(pid==1)
	{
		Var_ptr=0;
		PTP=78;
		P_n[0]="One_P1";
		P_n[1]="One_P2";
		P_n[2]="One_P3";
		P_n[3]="One_P4";
		P_n[4]="One_P5";
		P_n[5]="One_P6";
		P_n[6]="One_P7";
		P_n[7]="One_P8";
	}
	else if(pid==2)
	{
		Var_ptr=150;
		PTP=228;
		P_n[0]="Two_P1";
		P_n[1]="Two_P2";
		P_n[2]="Two_P3";
		P_n[3]="Two_P4";
		P_n[4]="Two_P5";
		P_n[5]="Two_P6";
		P_n[6]="Two_P7";
		P_n[7]="Two_P8";
	}
	struct queue *q=calloc(1,sizeof(struct queue));
	q->elements=0;
	while((ch=fgetc(fp))!= EOF)
	{
		char *str=calloc(1,sizeof(char));
		str[0]=ch;
		ch=fgetc(fp);
		while((ch!='\n')&&(ch!=EOF))
		{
			str=(char *)realloc(str,(++i)+1);
			str[i-1]=(char)ch;
			str[i]='\0';
			if(ch==' ')
			{
				flag_space=1;
				var_name_pos=i;
				space_pos=i-1;
			}
				
			else if(ch=='=')
			{
				flag_eq=1;
				
			}	
			else if(ch==';')
			{
				semcol=1;
			}
			ch=fgetc(fp);		
		}
		line_no++;
		printf("PID=%d Instruction=%s\n",pid,str);
		if(semcol==0)
		{
			printf("No semicolon found\n");
			break;
		}
		if(flag_space==1)//Definition of var
		{
			char var_name=str[var_name_pos];
			int index=0;
			int variable_found=0;
			if(Variable_counter==26)
			{
				printf("Cannot define more than 26 variables\n");
				pthread_exit(0);
			}
			for(index=0;index<Variable_counter;index++)
			{
				if(var_name==MEMORY[index*3+Var_ptr])
				{
					printf("Variable already defined\n");
					pthread_exit(0);
				}
			}
			/*Variable is not defined yet so insert an entry for this variable into MEMORY*/
			MEMORY[Variable_counter*3+Var_ptr]=var_name;
			/*Check which data Type it is and assign memory accordingly*/
			char *data_type=calloc(1,sizeof(char));
			for(index=0;index<space_pos;index++)
			{
				data_type=(char *)realloc(data_type,index+2);
				data_type[index]=str[index];
				data_type[index+1]='\0';
			}
			unsigned char incr=0x00;
			if(strcmp(data_type,"int")==0)//Variable is defined as integer
			{
				//Allocate 2 blocks of memory for an integer(32bits)
				MEMORY[((Variable_counter)*3)+1+Var_ptr]=0x02;
				MEMORY[((Variable_counter)*3)+2+Var_ptr]=VA_counter;
				incr=0x02;
				//VA_counter=VA_counter+0x04;
				printf("Pid=%d Datatype=integer Virtual Address counter=0x%02hx\n",pid,VA_counter);
			}
			else if(strcmp(data_type,"float")==0)//Variable is defined as integer
			{
				//Allocate 4 blocks of memory for a float(32bits)
				MEMORY[((Variable_counter)*3)+1+Var_ptr]=0x04;
				MEMORY[((Variable_counter)*3)+2+Var_ptr]=VA_counter;
				incr=0x04;
				VA_counter=VA_counter+0x04;
				printf("Pid=%d Datatype=float Virtual Address counter=0x%02hx\n",pid,VA_counter);
			}
			else if(strcmp(data_type,"double")==0)//Variable is defined as integer
			{
				//Allocate 8 blocks of memory for a double(64bits)
				MEMORY[((Variable_counter)*3)+1+Var_ptr]=0x08;
				MEMORY[((Variable_counter)*3)+2+Var_ptr]=VA_counter;
				incr=0x08;
				//VA_counter=VA_counter+0x08;
				printf("Pid=%d Datatype=double Virtual Address counter=0x%02hx\n",pid,VA_counter);
			}
			else
			{
				printf("This data type is not valid\n");
			}
			char page_no=(VA_counter&0xE0)>>5;
			FILE *page_file=fopen(P_n[page_no],"ab+");
			int sk=0;
			for(sk=0;sk<incr;sk++)
			{
				fputc(0x00,page_file);
			}
			fclose(page_file);
			/*DataTyoe Checked*/
			Variable_counter++;
			free(data_type);
			VA_counter=VA_counter+incr;
			/*Entry Inserted*/
			
		}
		else if(flag_eq==1)//assignement
		{
			//printf("Eq\n");
			unsigned char *Phy_add1=calloc(1,1);
			unsigned char *Phy_add2=calloc(1,1);
			char *length1=calloc(1,1);
			char *P_no_out=calloc(1,1);
			Load(str,Variable_counter,Var_ptr,MEMORY,P_n,Phy_add1,PTP,pid,q,length1,P_no_out);
			int l=0;
			//printf("%d\n",*length1);
			if(*length1==0x02)
			{
				short int load=0;
				for(l=0;l<*length1;l++)
				{	
					load=load|(MEMORY[*Phy_add1+l]<<8*l);
				}
				//printf("%s %d\n",str,strlen(str));
				int str_i=2;
				if((str[2]>96)&(str[2]<123))
				{
					char *inp_str=calloc(2,1);
					inp_str[0]=str[2];
					inp_str[1]='\0';
					char *length2=calloc(1,1);
					char *P_no_out2=calloc(1,1);
					Load(inp_str,Variable_counter,Var_ptr,MEMORY,P_n,Phy_add2,PTP,pid,q,length2,P_no_out2);
					if(*length2==0x02)
					{
						short int load2=0;
						for(l=0;l<*length2;l++)
						{		
							load2=load2|(MEMORY[*Phy_add2+l]<<8*l);
						}
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						int num=atoi(number);
						//printf("num=%d\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						//printf("num=%d\n",load,load2);
						printf("Instruction=%s Variable value on RHS=%d\n",str,load2);
						printf("Instruction=%s LHS after completion=%d\n",str,load);
						char *buf=calloc(2,1);
						buf[1]=load>>8 & 0xFF;
						buf[0]=load & 0xFF;
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							MEMORY[*Phy_add1]=buf[0];
							MEMORY[*Phy_add1+1]=buf[1];
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);	
					}
					else if(*length2==0x04)
					{
						//printf("reached float\n");
						float load2=0;
						char *buf=calloc(5,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						char a[sizeof(float)];
						memcpy(&load2, buf, sizeof(float));
						//load2=BtoF((void*)buf);
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						float num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						load=(int)load;
						memcpy(buf,&load,2);
						printf("Instruction=%s LHS after completion=%d\n",str,load);
						//FtoB(buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
						//for
					}
					else if(*length2==0x08)
					{
						//printf("reached float\n");
						double load2=0;
						char *buf=calloc(9,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						//load2=BtoD((void*)buf);
						memcpy(&load2,buf, sizeof(double));
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						double num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						load=(int)load;
						memcpy(buf,&load,2);
						printf("Instruction=%s LHS after completion=%d\n",str,load);
						//DtoB(buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
					}
					//printf("\na\n");
				/*char *RHS=calloc(1,sizeof(char));
				RHS[0]='\0';
				int var_pres=0;
				while(str[str_i]!='\0')
				{	
					RHS=(char *)realloc(RHS,(str_i));
					RHS[str_i-1]='\0';
					RHS[str_i-2]=str[str_i];
					if(str[str_i]>'')
					{
						
					}
					str_i++;
				}*/
				}
				else
				{
					int str_ind=0;
					char number[strlen(str)-2];
					for(str_ind=2;str_ind<(strlen(str)-1);str_ind++)
					{
						number[str_ind-2]=str[str_ind];
					} 
					int num=atoi(number);
					printf("Instruction=%s Number on RHS=%d\n",str,num);
					char *buf=calloc(2,1);
					buf[1]=num>>8 & 0xFF;
					buf[0]=num & 0xFF;
					MEMORY[*Phy_add1]=buf[0];
					MEMORY[*Phy_add1+l]=buf[1];
					//printf("Instruction=%s After assignment LHS=%d\n",load);
					FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);	
				}
			
			}
			else if(*length1==0x04)
			{
				float load=0;
				char *buf=calloc(5,1);
				for(l=0;l<*length1;l++)
				{	
					buf[l]=MEMORY[*Phy_add1+l];
				}
				memcpy(&load,buf, sizeof(float));
				//load=BtoF(buf);
				//printf("load=%f\n",load);
				int str_i=2;
				if((str[2]>96)&(str[2]<123))
				{
					char *inp_str=calloc(2,1);
					inp_str[0]=str[2];
					inp_str[1]='\0';
					char *length2=calloc(1,1);
					char *P_no_out2=calloc(1,1);
					Load(inp_str,Variable_counter,Var_ptr,MEMORY,P_n,Phy_add2,PTP,pid,q,length2,P_no_out2);
					if(*length2==0x02)
					{
						short int load2=0;
						for(l=0;l<*length2;l++)
						{		
							load2=load2|(MEMORY[*Phy_add2+l]<<8*l);
						}
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						int num=atoi(number);
						//printf("num=%d\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						//printf("num=%d\n",load,load2);
						
						char *buf=calloc(5,1);
						memcpy(buf,&load, sizeof(float));
						//FtoB(buf,load);
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							MEMORY[*Phy_add1]=buf[0];
							MEMORY[*Phy_add1+1]=buf[1];
							MEMORY[*Phy_add1+2]=buf[2];
							MEMORY[*Phy_add1+3]=buf[3];
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);	
					}
					else if(*length2==0x04)
					{
						//printf("reached float\n");
						float load2=0;
						char *buf=calloc(4,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						memcpy(&load2,buf,sizeof(float));
						//load2=BtoF((void*)buf);
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						float num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						memcpy(buf,&load, sizeof(float));
						printf("Instruction=%s Variable value on RHS=%f\n",str,load2);
						printf("Instruction=%s LHS after completion=%f\n",str,load);						
						//FtoB(buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						MEMORY[*Phy_add1+2]=buf[2];
						MEMORY[*Phy_add1+3]=buf[3];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
						//for
					}
					else if(*length2==0x08)
					{
						//printf("reached float\n");
						double load2=0;
						char *buf=calloc(8,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						load2=BtoD((void*)buf);
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						double num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						DtoB(buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						MEMORY[*Phy_add1+2]=buf[2];
						MEMORY[*Phy_add1+3]=buf[3];
						MEMORY[*Phy_add1+4]=buf[4];
						MEMORY[*Phy_add1+5]=buf[5];
						MEMORY[*Phy_add1+6]=buf[6];
						MEMORY[*Phy_add1+7]=buf[7];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
					}
					
				}
				else
				{
					int str_ind=0;
					char number[strlen(str)-2];
					for(str_ind=2;str_ind<(strlen(str)-1);str_ind++)
					{
						number[str_ind-2]=str[str_ind];
					} 
					float num=atof(number);
					printf("Instruction=%s Number on RHS=%f\n",str,num);
					char *buf=calloc(5,1);
					memcpy(buf,&num, sizeof(float));
					//FtoB(buf,num);
					MEMORY[*Phy_add1]=buf[0];
					MEMORY[*Phy_add1+1]=buf[1];
					MEMORY[*Phy_add1+2]=buf[2];
					MEMORY[*Phy_add1+3]=buf[3];
					FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
				}
			
			}
			else if(*length1==0x08)
			{
				double load=0;
				char *buf=calloc(8,1);
				for(l=0;l<8;l++)
				{	
					buf[l]=MEMORY[*Phy_add1+l];
				}
				memcpy(&load,buf,sizeof(double));
				//load=BtoD((void*)buf);
				//printf("load=%f\n",load);
				int str_i=2;
				if((str[2]>96)&(str[2]<123))
				{
					char *inp_str=calloc(2,1);
					inp_str[0]=str[2];
					inp_str[1]='\0';
					char *length2=calloc(1,1);
					char *P_no_out2=calloc(1,1);
					Load(inp_str,Variable_counter,Var_ptr,MEMORY,P_n,Phy_add2,PTP,pid,q,length2,P_no_out2);
					if(*length2==0x02)
					{
						short int load2=0;
						for(l=0;l<*length2;l++)
						{		
							load2=load2|(MEMORY[*Phy_add2+l]<<8*l);
						}
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						int num=atoi(number);
						//printf("num=%d\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						//printf("num=%d\n",load,load2);
						char *buf=calloc(4,1);
						FtoB((void*)buf,load);
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							MEMORY[*Phy_add1]=buf[0];
							MEMORY[*Phy_add1+1]=buf[1];
							MEMORY[*Phy_add1+2]=buf[2];
							MEMORY[*Phy_add1+3]=buf[3];
							MEMORY[*Phy_add1+4]=buf[4];
							MEMORY[*Phy_add1+5]=buf[5];
							MEMORY[*Phy_add1+6]=buf[6];
							MEMORY[*Phy_add1+7]=buf[7];
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);	
					}
					else if(*length2==0x04)
					{
						//printf("reached float\n");
						float load2=0;
						char *buf=calloc(4,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						load2=BtoF((void*)buf);
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						float num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						FtoB((void*)buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						MEMORY[*Phy_add1+2]=buf[2];
						MEMORY[*Phy_add1+3]=buf[3];
						MEMORY[*Phy_add1+4]=buf[4];
						MEMORY[*Phy_add1+5]=buf[5];
						MEMORY[*Phy_add1+6]=buf[6];
						MEMORY[*Phy_add1+7]=buf[7];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
							if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
						//for
					}
					else if(*length2==0x08)
					{
						//printf("reached double\n");
						double load2=0;
						char *buf=calloc(9,1);
						for(l=0;l<*length2;l++)
						{		
							buf[l]=MEMORY[*Phy_add2+l];
						}
						memcpy(&load2,buf,sizeof(double));
						//load2=BtoD(buf);
						//printf("load2=%f\n",load2);
						int str_ind=0;
						char number[strlen(str)-4];
						for(str_ind=4;str_ind<(strlen(str)-1);str_ind++)
						{
							number[str_ind-4]=str[str_ind];
						} 
						double num=atof(number);
						//printf("num=%f\n",num);
						if(str[3]=='+')
						{
							load=load2+num;
						}
						else if(str[3]=='-')
						{
							load=load2-num;
						}
						else if(str[3]=='*')
						{
							load=load2*num;
						}
						else
						{
							load=load2/num;
						}
						memcpy(buf,&load, sizeof(double));
						printf("Instruction=%s Variable value on RHS=%f\n",str,load2);
						printf("Instruction=%s LHS after completion=%f\n",str,load);
						//DtoB((void*)buf,load);
						MEMORY[*Phy_add1]=buf[0];
						MEMORY[*Phy_add1+1]=buf[1];
						MEMORY[*Phy_add1+2]=buf[2];
						MEMORY[*Phy_add1+3]=buf[3];
						MEMORY[*Phy_add1+4]=buf[4];
						MEMORY[*Phy_add1+5]=buf[5];
						MEMORY[*Phy_add1+6]=buf[6];
						MEMORY[*Phy_add1+7]=buf[7];
						FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
					}
					
				}
				else
				{
					int str_ind=0;
					char number[strlen(str)-2];
					for(str_ind=2;str_ind<(strlen(str)-1);str_ind++)
					{
						number[str_ind-2]=str[str_ind];
					} 
					double num=atof(number);
					printf("Instruction=%s Number on RHS=%f\n",str,num);
					//printf("%f",num);
					char *buf=calloc(8,1);
					memcpy(buf,&num,sizeof(double));
					//FtoB((void*)buf,num);
					//printf("\n%c blah %c\n",buf[6],buf[7]);
					MEMORY[*Phy_add1]=buf[0];
							MEMORY[*Phy_add1+1]=buf[1];
							MEMORY[*Phy_add1+2]=buf[2];
							MEMORY[*Phy_add1+3]=buf[3];
							MEMORY[*Phy_add1+4]=buf[4];
							MEMORY[*Phy_add1+5]=buf[5];
							MEMORY[*Phy_add1+6]=buf[6];
							MEMORY[*Phy_add1+7]=buf[7];
					FILE *page_file=fopen(P_n[*P_no_out],"w");
						char P_entry=MEMORY[*P_no_out+PTP];
						char frame_n=P_entry&0x03;
						int frame_start=0;	
						if(frame_n==0)
							{		
								frame_start=86;
							}
							else if(frame_n==1)
							{
								frame_start=118;
							}
							else if(frame_n==3)
							{
								frame_start=235;
							}
							else if(frame_n==3)
							{
								frame_start=268;
							}
							
						int sk=0;
						for(sk=0;sk<31;sk++)
						{
							fputc(MEMORY[frame_start+sk],page_file);
						}
						fclose(page_file);
				}
			
			}
			
		}
		else
		{
			printf("Error at Line %d :Statement is not valid and File:%s\n",line_no,name);
			break;
		}
		//printf("\naa\n");
		if(ch==EOF)
		{
			break;
		}
		flag_space=0;
		flag_eq=0;
		semcol=0;
		free(str);
		i=1;
	}
	fclose(fp);
	unlink(P_n[0]);
	unlink(P_n[1]);
	unlink(P_n[2]);
	unlink(P_n[3]);
	unlink(P_n[4]);
	unlink(P_n[5]);
	unlink(P_n[6]);
	unlink(P_n[7]);
	/*---------------------------------------------------------*/
	pthread_exit(0);
	
}
int main()
{
	int i=1;
	pthread_t thread[2];
	MEMORY=calloc(300,1);
	//char name[7] ="One.txt";
	for(i=1;i<3;i++)
	{
	
		int *arg = malloc(sizeof(*arg));
		*arg = i;	
		if(pthread_create(&(thread[i-1]), NULL,parse,arg)!=0)
		{
			printf("Error creating thread\n");
		}		
	}			
	
	for(i=0;i<2;i++)
	{
		if(pthread_join(thread[i], NULL)!=0)
		{
			printf("Error destroying thread\n");
		}		
	}
}

