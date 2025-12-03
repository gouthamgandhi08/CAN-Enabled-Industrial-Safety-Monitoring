#include<LPC21xx.h>
#include"LCD.h"
typedef unsigned int u32;
typedef unsigned char u8;

#define motor 12
#define LED 17
#define USmsg 0x100
#define RFmsgid 0x101
#define LDRmsg 0x102
typedef struct 
{
	u32 id;
	u32 rtr;
	u32 dlc;
	u32 databyteA;
	u32 databyteB;
}CAN2_MSG;
volatile u32 motor1=2,area=2,light=2,env=2;
unsigned char rf1[2][20]={"ACCESS DENIED","ACCESS GRANTED"};
unsigned char rf2[2][20]={"TRY AGAIN","USER ID: ####"};

unsigned char ldr1[3][20]={"GOOD","BAD","CHECKING..."};
unsigned char ldr2[3][20]={"OFF","ON","CHECKING..."};
unsigned char od1[3][20]={"CLEAR","ALERT! OBSTACLE","CHECKING..."};
unsigned char od2[3][20]={"RUNNING","STOPPED","CHECKING..."};
void can2_init(void);
void can2_tx(CAN2_MSG m1);
void can2_rx(CAN2_MSG *m1);
void display(void);

int main()
{
	CAN2_MSG m1;
	int c=0;
    IODIR0 |=1<<LED|1<<motor;
    IOSET0=1<<LED;
	can2_init();
	LCD_init();
	LCD_command(0x80);
	LCD_STR("WELCOME V24CE9G1");
	while(1)	  
	{
	    IOCLR0=1<<LED;
		  can2_rx(&m1);
			if((m1.id == RFmsgid))
			{
				IOSET0=1<<LED;	     
				LCD_command(0x01);
			    LCD_command(0x80);
				LCD_STR(rf1[m1.databyteA]);
				LCD_command(0xc0);
				LCD_STR(rf2[m1.databyteA]);
				delay_ms(2000);
				if(m1.databyteA == 1)
				{
					if(c==0)
					{
						c=1;
							IOSET0 = 1<<motor;
						 motor1=0;	
					}
					else
					{
						c=0;
						  	IOCLR0 = 1<<motor;
						motor1=1;
					}
				}
			}
			if((m1.id == LDRmsg))
			{
				IOSET0=1<<LED;
				if(m1.databyteA == 1)
				{
						env=1;
					  light=1;
				}
        else
				{
					 env=0;
					 light=0;
				}
			}
			if((m1.id == USmsg))
			{
				IOSET0=1<<LED;
				if(m1.databyteA == 1)
				{
					area=1;
					motor1=1;
					IOCLR0 = 1<<motor;
					c=0;
				}
				else
				{
					area=0;
				}	     
			}
			display();
			
	}
}

void can2_init(void)
{
  PINSEL1=0x14000;
  VPBDIV=1;//PCLK=60MHz
  C2MOD=0x01;
  AFMR=2;
  C2BTR=0x001C001D;
  C2MOD=0x00;
}
void can2_tx(CAN2_MSG m1)
{
  C2TID1=m1.id;
  C2TFI1=m1.dlc<<16;
  if(m1.rtr == 0)
  {
	C2TFI1 &= ~(1<<30);
	C2TDA1 = m1.databyteA;
	C2TDB1 = m1.databyteB;
  }
  else
  {
    C2TFI1 |= (1<<30);
  }
  C2CMR=(1<<0)|(1<<5);//Start Xmission & select Tx Buf1
	//C2CMR=(3<<0)|(1<<5);//Start Xmission & select Tx Buf1
	while((C2GSR&(1<<3))==0);//wait for data Xmission.


}
void can2_rx(CAN2_MSG *m1)
{
	   while((C2GSR&0x01) == 0);
	   m1->id=C2RID;
	   m1->dlc=(C2RFS>>16)&0xF;
	   m1->rtr=(C2RFS>>30)&0x1;
	   if(m1->rtr==0)
	   { 
	     //if data frame
	   	 m1->databyteA=C2RDA;
		   m1->databyteB=C2RDB;
	   }
	   C2CMR=(1<<2);//free receiver buffer(imp)
	   //while(C2GSR & 0x01); // Wait until released
                 delay_ms(10); 
}
void display()
{
	      LCD_command(0x01);
			  LCD_command(0x80);
	      LCD_STR("AREA:");
	      LCD_STR(od1[area]);
				LCD_command(0xc0);
	      LCD_STR("MOTOR:");
				LCD_STR(od2[motor1]);
				LCD_command(0x94);
	      LCD_STR("ENVCON:");
				LCD_STR(ldr1[env]);
	      LCD_command(0xd4);
	      LCD_STR("LIGHT:");
				LCD_STR(ldr2[light]);
	
}
