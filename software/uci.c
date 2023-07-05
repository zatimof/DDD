#include <iom8.h>					//подключаем стандартные библиотеки
#include <intrinsics.h>

//constant declare					//объявление констант
#define DELAY 50					//задержка формирования сигналов
#define ENCDEL 3					//количество циклов подтвержения
#define ALARM 5000000 					//расстояние до точки оповещения, нм

//functions declare					//объявление функций
void initialize(void);					//функция инициализации
void update(void);					//функция обновления экрана
void clock(void);					//функция формирования тактового импульса

//variables declare					//объявление переменных
unsigned char buffer[6]={99,12,13,14,15,99};		//буфер экрана,0-крайнее левое знакоместо, 5-правое
unsigned char dir[10]={2,2,2,2,2,2,2,2,2,2};
unsigned char i,k,dot=0,m=0,enc=0,encp1=0,encp2=0,enccnt=0,beep=0,cnt0=0,cnt1=0;
long  tmp,data[10]={0,0,0,0,0,0,0,0,0,0},err=0,step=0;

__flash const char img[16][100]={{			//массив данных знакогенератора символов цифр и знаков
#include <D:\REPO_PUBLIC\DDD\software\0.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\1.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\2.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\3.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\4.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\5.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\6.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\7.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\8.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\9.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\+.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\-.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\zast1.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\zast2.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\zast3.dat>
},{
#include <D:\REPO_PUBLIC\DDD\software\zast4.dat>
}};

__flash const char dotim[5]={0xC0,0xF8,0xFE,0x7E,0x1E};	//массив данных знакогенератора символа запятой

__flash const char mem[9][25]={				//массив данных знакогенератора надписей М1 - М9
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06,0x06,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x82,0xC3,0xE3,0xB1,0xB1,0x9F,0x8E,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x42,0xC3,0x89,0x89,0xCB,0xFF,0x76,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x30,0x38,0x2C,0x26,0x23,0xFF,0xFF,0x20,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x5C,0xDF,0x8F,0x89,0x89,0xF9,0x71,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x7E,0xFF,0x89,0x89,0x89,0xFB,0x72,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x03,0x03,0xF1,0xF9,0x1F,0x0F,0x03,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x76,0xFF,0x89,0x89,0x89,0xFF,0x76,0x00,0x00,0x00,0x00,0x00},
  {0xFF,0xFF,0x0E,0x1C,0x38,0x38,0x1C,0x0E,0xFF,0xFF,0x00,0x00,0x00,0x46,0xCF,0x89,0x89,0x89,0xFF,0x7E,0x00,0x00,0x00,0x00,0x00}};

__flash const char arrow[25]=				//массив данных знакогенератора стрелки направления
  {0x00,0x00,0x00,0x00,0x18,0x18,0x3C,0x3C,0x7E,0x7E,0xFF,0xDB,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00};

//main function						//главная функция
int main(void)
{
    initialize();					//вызов функции инициализации
    
    update();						//обновление экрана индикатора (вывод заставки)
    __delay_cycles(10000000);				//пауза

    tmp=step;						//пересчет двоичного числа step в двоично-десятичное
    i=0;						//для отображения цифр на индикаторе
    while(tmp>99999)
    {
      tmp-=100000;
      i++;
    }
    buffer[0]=i;
    
    i=0;
    while(tmp>9999)
    {
      tmp-=10000;
      i++;
    }
    buffer[1]=i;
      
    i=0;
    while(tmp>999)
    {
      tmp-=1000;
      i++;
    }
    buffer[2]=i;
        
    i=0;
    while(tmp>99)
    {
      tmp-=100;
      i++;
    }
    buffer[3]=i;
       
    i=0;
    while(tmp>9)
    {
      tmp-=10;
      i++;
    }
    buffer[4]=i;
        
    buffer[5]=tmp;
    
    update();						//обновление экрана индикатора (вывод числа step)
    __delay_cycles(10000000); 				//пауза

 	dot=0x08;					//устанавливаем местоположение запятой - в знакоместе 3
	
	while(1)					//бесконечный цикл программы
	{	  
      if(data[0]>999999999)				//проверяем достижение максимально допустимых значений
        data[0]=999999999;				//для индикатора, если превышают то ограничиваем
      if(data[0]<-999999999)
        data[0]=-999999999;
      
      for(i=1;i<10;i++)					//в цикле проверяем все точки оповещения
      {							//при достижении включаем сигнал
        err=data[i]-data[0];
        if(((dir[i]==0)&&(err<0)&&(err>-ALARM))||((dir[i]==1)&&(err>0)&&(err<ALARM)))
          beep|=0x01<<i;
        else
          beep&=(0x01<<i)^0xFF;
      }
            
      tmp=data[m];					//пересчет числа координаты для отображения на индикаторе
            
      if(tmp>=0)					//отдельно для положительных и отрицательных значений 
      {
        buffer[0]=99;
      
        i=0;
        while(tmp>99999999)
        {
          tmp-=100000000;
          i++;
        }
        buffer[1]=i;
      
        i=0;
        while(tmp>9999999)
        {
          tmp-=10000000;
          i++;
        }
        buffer[2]=i;
        
        i=0;
        while(tmp>999999)
        {
          tmp-=1000000;
          i++;
        }
        buffer[3]=i;
       
        i=0;
        while(tmp>99999)
        {
          tmp-=100000;
          i++;
        }
        buffer[4]=i;
        
        i=0;
        while(tmp>9999)
        {
          tmp-=10000;
          i++;
        }
        buffer[5]=i;
      }
      else
      {
        buffer[0]=11;

        i=0;
        while(tmp<-99999999)
        {
          tmp+=100000000;
          i++;
        }
        buffer[1]=i;
       
        i=0;
        while(tmp<-9999999)
        {
          tmp+=10000000;
          i++;
        }
        buffer[2]=i;
        
        i=0;
        while(tmp<-999999)
        {
          tmp+=1000000;
          i++;
        }
        buffer[3]=i;
        
        i=0;
        while(tmp<-99999)
        {
          tmp+=100000;
          i++;
        }
        buffer[4]=i;
        
        i=0;
        while(tmp<-9999)
        {
          tmp+=10000;
          i++;
        }
        buffer[5]=i;
      }
      
      update();
    }
}

//initialize function
void initialize(void)
{
	//init ports
	DDRB=0x3F;
	PORTB=0x00;
	
	DDRC=0x2F;
	PORTC=0x20;
	
	DDRD=0xF8;
	PORTD=0x08;
	  
	//timers
	TCCR0=0x04;
    TCCR1A=0x00;
    TCCR1B=0x03;
    		
	//init interrupt
	TIMSK=0x05;
	GICR=0x40;
	MCUCR=0x01;
	SREG|=0x80;	//прерывания разрешены

    //first read counter
    DDRC=0x20;
    PORTD&=0xF7;
    __delay_cycles(DELAY);
    cnt1=PINC&0x0F;
    PORTD|=0x08;
    DDRC=0x2F;
    
    //read STEP from EEPROM
    while(EECR&0x02);
    k=0;
    for(i=0;i<4;i++)
    {
      EEAR=i;
      EECR|=0x01;
      tmp=EEDR;
      step|=(tmp<<(i<<3));
      k+=EEDR;
    }
    EEAR=4;
    EECR|=0x01;
    if((EEDR!=k)||(step==0)||((PINC&0x10)==0))
    {
      beep=1;
      buffer[0]=99;
      buffer[1]=99;
      buffer[2]=99;
      buffer[3]=99;
      buffer[4]=99;
      buffer[5]=99;
      update();

      while((PINC&0x10)==0);    //ждем отпускания кнопки
      m=0;
      step=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[5]=m;
        update();
      }       
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[5];
      step+=tmp;
      m=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[4]=m;
        update();
      }       
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[4];
      step+=tmp*10;
      m=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[3]=m;
        update();
      }       
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[3];
      step+=tmp*100;
      m=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[2]=m;
        update();
      }       
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[2];
      step+=tmp*1000;
      m=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[1]=m;
        update();
      }       
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[1];
      step+=tmp*10000;
      m=0;
      while(PINC&0x10)        //ждем нажатия кнопки
      {
        buffer[0]=m;
        update();
      }
      while((PINC&0x10)==0);    //ждем отпускания кнопки
      tmp=buffer[0];
      step+=tmp*100000;
      m=0;
      k=0;
      for(i=0;i<4;i++)
      {
        while(EECR&0x02);
        EEAR=i;
        EEDR=(step&(0xFF<<(i<<3)))>>(i<<3);
        k+=EEDR;        
        EECR|=0x04;
        EECR|=0x02;
      }
      while(EECR&0x02);
      EEAR=4;
      EEDR=k;
      EECR|=0x04;
      EECR|=0x02;
      beep=0;
    }    
	return;
}

#pragma vector=TIMER0_OVF_vect
__interrupt void tim0(void)
{
  if(!(PINC&0x10))
    if(!m)
    {
      data[0]=0;
      dir[0]=2;
    }
    else
    {
      data[m]=data[0];      
      dir[m]=dir[0];
    }
  
  enc=PIND&0x03;
            
  if((enc!=encp1)&&(enc!=encp2))
    enccnt++;
  else
    enccnt=0;
      
  if(enccnt>ENCDEL)
  {
    if((encp2==3)&&(encp1==2)&&(enc==0))
      m--;
    if((encp2==3)&&(encp1==1)&&(enc==0))
      m++;
    if((encp2==0)&&(encp1==1)&&(enc==3))
      m--;
    if((encp2==0)&&(encp1==2)&&(enc==3))
      m++;
    if((encp2==1)&&(encp1==0)&&(enc==3))
      m--;
    if((encp2==2)&&(encp1==0)&&(enc==3))
      m++;
    if((encp2==2)&&(encp1==3)&&(enc==0))
      m--;
    if((encp2==1)&&(encp1==3)&&(enc==0))
      m++;
       
    if(m==10)
      m=0;
    if(m==255)
      m=9;
    encp2=encp1;
    encp1=enc;
  }

  return;
}

#pragma vector=TIMER1_OVF_vect
__interrupt void tim1(void)
{
  if(beep)
    PORTC^=0x20;
  else
    PORTC|=0x20;
  return;
}

#pragma vector=INT0_vect
__interrupt void int0(void)
{
  unsigned char e=0;

DDRC=0x20;
PORTD&=0xF7;
__delay_cycles(DELAY);
cnt0=PINC&0x0F;
PORTD|=0x08;
DDRC=0x2F;

if(cnt0!=cnt1)
{
  if(cnt0>cnt1)
  {
    e=cnt0-cnt1;
	dir[0]=0;
	if(e>8)
    {
	  e=16-e;
      dir[0]=1;
    }
	if(e==8)
	  beep=1;
  }
  else
  {
    e=cnt1-cnt0;
	dir[0]=1;
	if(e>8)
    {
	  e=16-e;
      dir[0]=0;
    }
  }
  cnt1=cnt0;
  if(e==8)
	beep=1;
  if(dir[0])
    data[0]+=e*step;
  else
    data[0]-=e*step;
}
  return;
}

void update(void)
{
unsigned char i,k,t,data;
	
for(k=0;k<3;k++)
{
	PORTB=0x08<<k;
			
	for(t=0;t<4;t++)
	{
		PORTC&=0xF0;
        PORTD&=0x0F;
		PORTD|=(t<<6);
		clock();

		PORTB|=0x04;
		for(i=0;i<25;i++)
		{
			if(buffer[k+k]<16)
				data=img[buffer[k+k]][3-t+(i<<2)];
			else
				data=0;
			
			if((t==3)&&(i>19)&&(dot&(0x01<<(k+k))))
				data|=dotim[i-20];
			
            if((t==0)&&(k==0)&&m&&(!beep))
				data|=mem[m-1][i];
            
            if((t==3)&&(k==0)&&(dir[m]<2))
              if(dir[m])
				data|=arrow[i];
              else
                data|=arrow[24-i];
            
			PORTC&=0xF0;
			PORTC|=data&0x0F;
            PORTD&=0x0F;
			PORTD|=data&0xF0;
			clock();
 		}
		
		for(i=0;i<25;i++)
		{
			if(buffer[k+k+1]<16)
				data=img[buffer[k+k+1]][3-t+(i<<2)];
			else
				data=0;
			
			if((t==3)&&(i>19)&&(dot&(0x01<<(k+k+1))))
				data|=dotim[i-20];
            
			PORTC&=0xF0;
			PORTC|=data&0x0F;
            PORTD&=0x0F;
			PORTD|=data&0xF0;
			clock();
 		}
		
		PORTB&=0xFB;
	}
	
		PORTC&=0xF0;
		PORTC|=0x09;
        PORTD&=0x0F;
		PORTD|=0x30;
		clock();		
		PORTB=0x00;
}
return;
}

void clock(void)
{
	__delay_cycles(DELAY);
	PORTB|=0x01;
	__delay_cycles(DELAY);
	PORTB&=0xFE;
	__delay_cycles(DELAY);
return;
}
