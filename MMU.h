#ifndef MMU_H_INCLUDED
#define MMU_H_INCLUDED
#include "QUEUE.h"
/*This will translate Virtual address to physical address and will retun if the page is in the memory*/

int Translate(char VA,int PTP,char *frame,char *offset,char *P_no,char *MEM)
{
	*P_no=(VA&0xE0)>>5;
	char P_entry=MEM[*P_no+PTP];
	*frame=P_entry&0x03;
	*offset=(VA&0x1F);
	return ((P_entry>>2)&0x01);
}

void Page_Fault(int pid,char *page_file,struct queue *q,char *MEM,int PTP,char page_no,char frame_cnt)
{
	
	if(q->elements<2)
	{
		enque(q,page_no);
		FILE *fp=fopen(page_file,"r");
		if(frame_cnt==0)
		{
			int i=0;
			int frame_start=(pid-1)*149+86;
			int data=0;
			while((data=fgetc(fp))!=EOF)
			{
				MEM[frame_start++]=(char)data;
			}
			char P_entry=MEM[page_no+PTP];
			if(pid==1)
				P_entry=P_entry|0x00;
			else
				P_entry=P_entry|0x02;
			P_entry=P_entry|0x04;
			MEM[page_no+PTP]=P_entry;
			frame_cnt++;
		}
		else
		{
			int i=0;
			int frame_start=(pid-1)*149+118;
			int data=0;
			while((data=fgetc(fp))!=EOF)
			{
				MEM[frame_start++]=(char)data;
			}
			char P_entry=MEM[page_no+PTP];
			if(pid==1)
				P_entry=P_entry|0x01;
			else
				P_entry=P_entry|0x03;
			P_entry=P_entry|0x04;
			MEM[page_no+PTP]=P_entry;
			//frame_cnt++;
			
		}
		
		
	}
	else
	{
		int p=deque(q);
		enque(q,page_no);
		FILE *fp=fopen(page_file,"r");
		printf("Replacing Page no %d\n",p);
		char P_entry1=MEM[p+PTP];
		P_entry1=P_entry1&0x03;
		char frame_n=P_entry1&0x03;
		MEM[p+PTP]=P_entry1;
		char P_entry2=MEM[page_no+PTP];
		P_entry2=P_entry2|0x04;
		P_entry2=P_entry2|frame_n;
		MEM[p+PTP]=P_entry1;
		int frame_start;
			if(frame_n==0)
			{
				frame_start=86;
			}
			else if(frame_n==1)
			{
				frame_start=118;
			}
			else if(frame_n==2)
			{
				frame_start=235;
			}
			else if(frame_n==3)
			{
				frame_start=268;
			}
			int data=0;
			while((data=fgetc(fp))!=EOF)
			{
				MEM[frame_start++]=(char)data;
			}
	}
	
}
void Load(char *str,int Variable_counter,int Var_ptr,char *MEMORY,char *P_n[],char *Physical_add,int PTP,int pid,struct queue *q,char *length,char *P_no_out)
{
			char var_nm=str[0];
			int index=0;
			int variable_found=0;
			for(index=0;index<Variable_counter;index++)
			{
				if(var_nm==MEMORY[index*3+Var_ptr])
				{
					variable_found=1;
					break;
				}
			}
			if(variable_found==0)
			{
				printf("Could not find variable\n");
				remove(P_n[0]);
				remove(P_n[1]);
				remove(P_n[2]);
				remove(P_n[3]);
				pthread_exit(0);
			}
			char var_vadd=MEMORY[(index*3)+2+Var_ptr];
			//printf("%d 0x%02x",index,var_vadd);
			char frame=0x00;
			char offset=0x00;
			char P_no=0x00;
			if(Translate(var_vadd,PTP,&frame,&offset,&P_no,MEMORY)==0)
			{
				printf("Page Fault\n");
				Page_Fault(pid,P_n[P_no],q,MEMORY,PTP,P_no,q->elements);
				Translate(var_vadd,PTP,&frame,&offset,&P_no,MEMORY);
				*P_no_out=P_no;					
			}
			printf("Pid=%d Page No:0x%02x\n",pid,P_no);
			printf("Pid=%d FrameNo:0x%02x\n",pid,frame);
			printf("Pid=%d Offset:0x%02x\n",pid,offset);
			*Physical_add=((frame<<5)|(offset))+(char)PTP+8;
			printf("Pid=%d Offset:0x%02x\n",pid,offset);
			//printf("Pid=%d Physical Address:0x%02x\n",pid,*Physical_add);
			*length=MEMORY[(index*3)+1+Var_ptr];
			printf("Pid=%d Length:0x%02x\n",pid,*length);
}
#endif
