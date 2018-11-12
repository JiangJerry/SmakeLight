#include "ht32.h"
#include "ht32_board.h"
#include "main.h"
#include "BMP280Drive.h"

u8 LEDStatus[72];
static u8 Mode4StartPoll = 0;
extern LED_RGB_Data_TypeDef  LEDLightAllBuffer[72],LEDLightAllBufferBak[72],*PtrBak;
extern vu8  wLED_Length;
//第一种模式4种状态机
#define     ORG     0        
#define     GRE     1        
#define     BLU     2        
#define     PINK    3   
//第一种模式各色最大值定义
u8 ORGRedMax,ORGGreenMax,GreenMax,PinkRedMax,PinkBlueMax,BlueMax,WhiteBlueMax,WhiteGreenMax,WhiteRedMax;
//第一种模式基色结构体定义


extern LED_RGB_Data_TypeDef   *LEDDispBuffPtr;
void ColorDataRdy4(u8 Ratio)
{
    u8 i,j;
    float  TempFloat;
    LED_RGB_Data_TypeDef LEDType2[4] =
    { 
        {     0xff,      0x80,   0x00},     //橙
        {     0x00,      0x00,   0x00},     //灭  
        {     0x80,      0x00,   0x00},     //红
        {     0x00,      0x00,   0x00},     //灭
    };
    LED_RGB_Data_TypeDef   *TempPtr;
    LED_RGB_Data_TypeDef  Temp[4];
    u8 *Ptr,*Ptr1;
    Ptr1 = &Temp[0].uRed;
    Ptr = &LEDType2[0].uRed;
    for(i=0;i<12;i++)       //基色比例赋初值,为防止出现修改源基色，新建一个结构体赋值
    {
        *Ptr1 = *Ptr;
        if(*Ptr1 != 0)
        {
            TempFloat = *Ptr1;
            TempFloat = (TempFloat * Ratio)/100;
            *Ptr1 = (u8)TempFloat;
        }
        Ptr++;
        Ptr1++;
    }

    ORGRedMax = Temp[0].uRed;       //提取颜色最大值
    ORGGreenMax = Temp[0].uGreen;
    PinkRedMax = Temp[2].uRed;

    
    TempPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
    Ptr1 = &LEDStatus[0];
    
    for(i=0;i<18;i++)
    {
        for(j=0;j<5;j++)
        {
            TempPtr->uRed = Temp[i%4].uRed;        //指针内容赋值
            TempPtr->uGreen = Temp[i%4].uGreen ;
            TempPtr->uBlue =Temp[i%4].uBlue;
            *Ptr1 = i%4;                            //记忆当前状态
            TempPtr++;                              //指向下一组元素
            Ptr1++;
            if((i&0x01) == 0x01)
            {
                if(j==2)
                    break;
            }
        }
    }
    Mode4StartPoll = 0;     //旋转从第一个开始
}
/*********************************************************************************************************
** 函数名称: ColorDataRdy1
** 功能描述: 模式1数据准备
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
void ColorDataRdy1(u8 Ratio,u8 Type)
{
    u8 i,j;
    float  RedStep, GreenStep, BlueStep,TempFloat;               // 各色步进值
    LED_RGB_Data_TypeDef LEDType1[4] =
    { 
        {   0xab,       0x55,     0},   //橙
        {      0,       0xff,     0},   //绿
        {      0,       0,     0xff},   //蓝 
        {  0xab,         0,    0x55},   //粉
    };  
    LED_RGB_Data_TypeDef   *BufferPtr;
    LED_RGB_Data_TypeDef  Temp[4];
    u8 *Ptr, *TempPtr;
    Ptr = &LEDType1[0].uRed;
    TempPtr = &Temp[0].uRed;
    for(i=0;i<12;i++)
    {
        *TempPtr = *Ptr;
        if(*Ptr != 0)
        {
            TempFloat = *TempPtr;
            TempFloat = (TempFloat * Ratio)/100;
            *TempPtr = TempFloat;
        }
        Ptr++;
        TempPtr++;
    }
    ORGRedMax = Temp[0].uRed;       //提取变化最大值
    ORGGreenMax = Temp[0].uGreen;
    GreenMax = Temp[1].uGreen;
    BlueMax = Temp[2].uBlue;
    PinkRedMax = Temp[3].uRed;
    PinkBlueMax = Temp[3].uBlue;
	if(Type == 1)
	{
		return;
	}

    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<12;i++)
    {
        RedStep = Temp[i%4].uRed/6;                 //亮度步进值
        GreenStep = Temp[i%4].uGreen/6;
        BlueStep = Temp[i%4].uBlue/6;
        for(j=0;j<6;j++)
        {
            BufferPtr->uRed = (u8)(RedStep*(j+1));        //指针内容赋值
            BufferPtr->uGreen = (u8)(GreenStep*(j+1));
            BufferPtr->uBlue = (u8)(BlueStep*(j+1));
            BufferPtr++;                                  //指向下一组元素
            LEDStatus[(i*6)+j] = i%4;           //记忆当前状态
        }   
    }
}


/*********************************************************************************************************
** 函数名称: ColorDataRdy5
** 功能描述: 模式5数据准备  蓝 白相间
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/

void ColorDataRdy5(u8 Ratio)
{
    u8 i,j;
    float  RedStep, GreenStep, BlueStep,TempFloat;               // 各色步进值


    LED_RGB_Data_TypeDef LEDType5[4] =
    { 
        {   0xab,       0x55,     0},   //橙
        {      0,          0,  0x80},   //蓝
        {0x80,       0x80,     0x80},   //白  
        {      0,          0,  0x80},   //蓝
    };  

    LED_RGB_Data_TypeDef   *BufferPtr;
    LED_RGB_Data_TypeDef  Temp[4];
    u8 *Ptr, *TempPtr;
    Ptr = &LEDType5[0].uRed;
    TempPtr = &Temp[0].uRed;
    for(i=0;i<12;i++)
    {
        *TempPtr = *Ptr;
        if(*Ptr != 0)
        {
            TempFloat = *TempPtr;
            TempFloat = (TempFloat * Ratio)/100;
            *TempPtr = TempFloat;
        }
        Ptr++;
        TempPtr++;
    }

    ORGRedMax = Temp[0].uRed;       //提取变化最大值
    ORGGreenMax = Temp[0].uGreen;
    WhiteBlueMax = Temp[2].uBlue;
    WhiteRedMax = Temp[2].uRed;
    WhiteGreenMax = Temp[2].uGreen;
    BlueMax = Temp[1].uBlue;


    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<12;i++)
    {
        RedStep = Temp[i%4].uRed/6;                 //亮度步进值
        GreenStep = Temp[i%4].uGreen/6;
        BlueStep = Temp[i%4].uBlue/6;
        for(j=0;j<6;j++)
        {
            BufferPtr->uRed = (u8)(RedStep*(j+1));        //指针内容赋值
            BufferPtr->uGreen = (u8)(GreenStep*(j+1));
            BufferPtr->uBlue = (u8)(BlueStep*(j+1));
            BufferPtr++;                                  //指向下一组元素
            LEDStatus[(i*6)+j] = i%4;           //记忆当前状态
        }   
    }
}
/*********************************************************************************************************
** 函数名称: ColorDataRdy4
** 功能描述: 模式1数据准备  蓝 白相间
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/

void ColorDataRdy2(u8 Ratio)
{
    u8 i,j;
    float  RedStep, GreenStep, BlueStep,TempFloat;               // 各色步进值
    LED_RGB_Data_TypeDef LEDType4[2] =
    { 
        {     0x80,       0x80,     0x80},   //白
        {        0,          0,     0xff},   //蓝  
    };  
    LED_RGB_Data_TypeDef   *BufferPtr;
    LED_RGB_Data_TypeDef  Temp[4];
    u8 *Ptr, *TempPtr;
    Ptr = &LEDType4[0].uRed;
    TempPtr = &Temp[0].uRed;
    for(i=0;i<6;i++)
    {
        *TempPtr = *Ptr;
        if(*Ptr != 0)
        {
            TempFloat = *TempPtr;
            TempFloat = (TempFloat * Ratio)/100;
            *TempPtr = TempFloat;
        }
        Ptr++;
        TempPtr++;
    }

    WhiteRedMax = Temp[0].uRed;       //提取变化最大值
    WhiteGreenMax = Temp[0].uGreen;
    WhiteBlueMax = Temp[0].uBlue;
    BlueMax = Temp[1].uBlue;


    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<6;i++)
    {
        RedStep = Temp[i%2].uRed/12;                 //亮度步进值
        GreenStep = Temp[i%2].uGreen/12;
        BlueStep = Temp[i%2].uBlue/12;
        for(j=0;j<12;j++)
        {
            BufferPtr->uRed = (u8)(RedStep*(12-j));        //指针内容赋值
            BufferPtr->uGreen = (u8)(GreenStep*(12-j));
            BufferPtr->uBlue = (u8)(BlueStep*(12-j));
            BufferPtr++;                                  //指向下一组元素
            LEDStatus[(i*12)+j] = i%2;           //记忆当前状态
        }   
    }
}
/*********************************************************************************************************
** 函数名称: ModifyColorTable2
** 功能描述: 模式2数据变换
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
void ModifyColorTable4(void)
{
    u8    bRED, bGREEN;
    u8 Temp,i;
    for (i = 0; i < (wLED_Length/8); i++)   //9个单元
    {
        Temp = (i<<3) + Mode4StartPoll;
        if(Temp >= wLED_Length)     //实现旋转
        {
            Temp = Temp % 72;
        }      
        bRED     = LEDDispBuffPtr[Temp].uRed;   //取值
        bGREEN   = LEDDispBuffPtr[Temp].uGreen;        
        if((i&0x01) == 0x01)    //仅减红色灯
        {
            if(bRED > 0)
                bRED = bRED - 1;
            LEDDispBuffPtr[Temp].uRed = bRED;            
        }
        else
        {
            if(bGREEN > 0)
            bGREEN = bGREEN - 1;
            if(bRED > 0)
                bRED = bRED - 1;          
            if(bRED > 0)
                bRED = bRED - 1;
            LEDDispBuffPtr[Temp].uRed = bRED;            
            LEDDispBuffPtr[Temp].uGreen = bGREEN;
        }        
                    

        Temp = Temp + 5;
        if(Temp >= wLED_Length)
        {
            Temp = Temp % 72;
        }
        bRED     = LEDDispBuffPtr[(Temp)].uRed;
        bGREEN   = LEDDispBuffPtr[(Temp)].uGreen;      
        if((i&0x01) == 0x01)    //仅加红
        {
            if(bRED<ORGRedMax)
                bRED = bRED +1;
        }
        else
        {
            if(bGREEN < ORGGreenMax)
                bGREEN = bGREEN+1;
            if(bRED<ORGRedMax)
                bRED = bRED +1;
            if(bRED<ORGRedMax)
                bRED = bRED +1;
           
        }
        
        LEDDispBuffPtr[Temp].uRed = bRED;            
        LEDDispBuffPtr[Temp].uGreen = bGREEN;    
    }       
    if((bRED == ORGRedMax)&&(bGREEN == ORGGreenMax))
    {
        Mode4StartPoll = Mode4StartPoll++;
        Mode4StartPoll = Mode4StartPoll % 72;
    }
}
/*********************************************************************************************************
** 函数名称: ModifyColorTable2
** 功能描述: 模式1数据变换
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
float Temp3;
extern u8 PressDn,PressDnBak;
extern u8 PressDnFlag;
u8 PressDnSta = 1;
#define FLICK_MIN 	3
void ModityDnUpTable(void)
{
    u8 i;
	u8 Flag = 1;
	
	for (i = 0; i < wLED_Length; i++)
	{
		if(PressDnSta == 1)
		{
			if(LEDDispBuffPtr[i].uRed <= FLICK_MIN)
			{
				Flag++;
			}
			else
			{
				LEDDispBuffPtr[i].uRed--;
			}
			if(LEDDispBuffPtr[i].uGreen <= FLICK_MIN)
			{
				Flag++;					
			}
			else
			{
				LEDDispBuffPtr[i].uGreen--;
			}
			if(LEDDispBuffPtr[i].uBlue <= FLICK_MIN)
			{
				Flag++;
			}
			else
			{
				LEDDispBuffPtr[i].uBlue--;
			}
			if(Flag >= 217)
			{
				PressDnSta = 2;
			}
		}
		else if(PressDnSta == 2)
		{
			if(LEDDispBuffPtr[i].uRed < LEDLightAllBufferBak[i].uRed)
			{
				LEDDispBuffPtr[i].uRed++;
				Flag = 0;
			}
			if(LEDDispBuffPtr[i].uGreen < LEDLightAllBufferBak[i].uGreen)
			{
				LEDDispBuffPtr[i].uGreen++;
				Flag = 0;					
			}
			if(LEDDispBuffPtr[i].uBlue < LEDLightAllBufferBak[i].uBlue)
			{
				LEDDispBuffPtr[i].uBlue++;
				Flag = 0;
			}				
			if((i == (wLED_Length - 1))&&(Flag == 1))
			{
				PressDnFlag = 0;
				PressDnSta = 1;
			}
		}
	}
}	

void ModifyColorTable1(void)
{
    u8 i;
    u8 bRED, bGREEN, bBLUE;

	for (i = 0; i < wLED_Length; i++)
	{
		bRED     = LEDDispBuffPtr[i].uRed;
		bGREEN   = LEDDispBuffPtr[i].uGreen;
		bBLUE    = LEDDispBuffPtr[i].uBlue; 		
		switch(LEDStatus[i])
		{
			case ORG:
				if(bBLUE == 0)
				{
					if(bRED < ORGRedMax)     //增加到此状态最大值
						bRED++;
					if(bRED < ORGRedMax)     //增加到此状态最大值
						bRED++;
					if(bGREEN < ORGGreenMax)
						bGREEN++;
					if(bRED > ORGRedMax)     //增加到此状态最大值
						bRED--;
					if(bGREEN > ORGGreenMax)
						bGREEN--;
				}
				else
				{
					bBLUE--;
				}
				if((bRED == ORGRedMax) && (bGREEN == ORGGreenMax))
					LEDStatus[i] = GRE;
				break;
			case GRE:
				if(bRED == 0)
				{
					if((bGREEN < GreenMax))   //增加到此状态最大值
						bGREEN++;
					if((bGREEN < GreenMax))   //增加到此状态最大值
						bGREEN++;
					if((bGREEN < GreenMax))   //增加到此状态最大值
						bGREEN++;
					if(bGREEN > GreenMax)
						bGREEN--;					
				}
				else
				{
					if(bRED > 0) 
						bRED--;
					if(bRED > 0) 
						bRED--;
				}
				if((bGREEN == GreenMax)&&(bRED == 0))
					LEDStatus[i] = BLU;                
				break;
			case BLU:               
				if(bBLUE < BlueMax)    //考虑到时间上的一致性，0x55 每次执行 0xab每二次执行 0xff 每三次执行
				{
					bBLUE++;
				}
				if(bBLUE < BlueMax)
				{
					bBLUE++;
				}
				if(bBLUE < BlueMax)
				{
					bBLUE++;
				}
				if(bBLUE > BlueMax)    //考虑到时间上的一致性，0x55 每次执行 0xab每二次执行 0xff 每三次执行
				{
					bBLUE--;
				}
				else
				{
					if(bGREEN >0)
					{
						bGREEN--;                   
					}                  
					if(bGREEN >0)
					{
						bGREEN--;                   
					}                  
					if(bGREEN >0)
					{
						bGREEN--;                   
					}  
				}					
				if((bBLUE == BlueMax)&&(bGREEN == 0))
					 LEDStatus[i] = PINK; 
				break;
			case PINK:
				if(bBLUE > PinkBlueMax)    //粉色A
				{
					 bBLUE--;
				}
				if(bBLUE < PinkBlueMax)
				{
					bBLUE++;
				}
				if(bRED < PinkRedMax)       //粉色B
				{
					bRED++;
				}
				if(bRED < PinkRedMax)       //粉色B
				{
					bRED++;
				}
				if(bRED > PinkRedMax)       //粉色B
				{
					bRED--;
				}
				if((bRED == PinkRedMax)&&(bBLUE == PinkBlueMax))
					 LEDStatus[i] = ORG;                 
				break;

		}
		LEDDispBuffPtr[i].uRed = bRED;            
		LEDDispBuffPtr[i].uGreen = bGREEN;
		LEDDispBuffPtr[i].uBlue  = bBLUE;
	}
}

u8 WhiteRedMax,WhiteGreenMax,WhiteBlueMax;
void ColorDataRdy6(u8 Radio)
{
    u8 i;
    u16 PinkRedMax = 0xaa,PinkBlueMax = 0x55;
    PinkRedMax = (PinkRedMax * Radio) / 100;
    PinkBlueMax = (PinkBlueMax * Radio) / 100;
    
    
    LED_RGB_Data_TypeDef   *BufferPtr;    
    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<72;i++)
    {
        BufferPtr->uRed = (u8)PinkRedMax;        //指针内容赋值
        BufferPtr->uGreen = 0;
        BufferPtr->uBlue = (u8)PinkBlueMax;
        BufferPtr++;                                  //指向下一组元素
    }
}
/*********************************************************************************************************
** 函数名称: ColorDataRdy3
** 功能描述: 模式3数据准备
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
void ColorDataRdy3(u8 Ratio)
{
    u8 i,j;  
    LED_RGB_Data_TypeDef LEDType3[4] =
    { 
        {   0xff,       0x80,     0},       //橙
        {      0,       0x80,     0x00},   //绿  
        {     0x22,       0x80,     0x22},   //白
        {      0,       0x80,     0x00},   //绿  
    };  
    
    float  RedStep, GreenStep, BlueStep,TempFloat;               // 各色步进值
    LED_RGB_Data_TypeDef   *BufferPtr;
    LED_RGB_Data_TypeDef  Temp[4];
    u8 *Ptr, *TempPtr;
    Ptr = &LEDType3[0].uRed;
    TempPtr = &Temp[0].uRed;
    for(i=0;i<12;i++)
    {
        *TempPtr = *Ptr;
        if(*Ptr != 0)
        {
            TempFloat = *TempPtr;
            TempFloat = (TempFloat * Ratio)/100;
            *TempPtr = TempFloat;
        }
        Ptr++;
        TempPtr++;
    }

    ORGRedMax = Temp[0].uRed;       //提取变化最大值
    ORGGreenMax = Temp[0].uGreen;
    GreenMax = Temp[1].uGreen;
    WhiteRedMax = Temp[2].uRed;
    WhiteGreenMax = Temp[2].uGreen;
    WhiteBlueMax = Temp[2].uBlue;
    
    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<12;i++)
    {
        RedStep = Temp[i%4].uRed/6;                 //亮度步进值
        GreenStep = Temp[i%4].uGreen/6;
        BlueStep = Temp[i%4].uBlue/6;
        for(j=0;j<6;j++)
        {
            BufferPtr->uRed = (u8)(RedStep*(j+1));        //指针内容赋值
            BufferPtr->uGreen = (u8)(GreenStep*(j+1));
            BufferPtr->uBlue = (u8)(BlueStep*(j+1));
            BufferPtr++;                                  //指向下一组元素
            LEDStatus[(i*6)+j] = i%4;           //记忆当前状态
        }   
    }
}
/*********************************************************************************************************
** 函数名称: ModifyColorTable3
** 功能描述: 模式3数据变换
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
u8 bRED, bGREEN, bBLUE;
void ModifyColorTable3(void)
{
    u8 i;
    for (i = 0; i < wLED_Length; i++)
    {
        bRED     = LEDDispBuffPtr[i].uRed;
        bGREEN   = LEDDispBuffPtr[i].uGreen;
        bBLUE    = LEDDispBuffPtr[i].uBlue;   
        switch(LEDStatus[i])
        {
            case ORG:
                if(bBLUE == 0)
                {
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bGREEN < ORGGreenMax)
                        bGREEN++;
                }
                else
                {
                    bBLUE--;
                }
                if((bRED == ORGRedMax) && (bGREEN == ORGGreenMax))
                {
                    LEDStatus[i] = GRE;                    
                }
                break;
            case GRE:
        
                if(bGREEN < GreenMax)   //增加到此状态最大值
                    bGREEN++;
                if(bGREEN > GreenMax)   //增加到此状态最大值
                    bGREEN--;

                if(bRED > 0) 
                    bRED--;
                if((bGREEN == GreenMax)&&(bRED == 0))
                    LEDStatus[i] = BLU;                
                break;
            case BLU:               
                if(bRED < WhiteRedMax)    //考虑到时间上的一致性，0x55 每次执行 0xab每二次执行 0xff 每三次执行
                {
                    bRED++;
                }
                if(bGREEN < WhiteGreenMax)
                {
                    bGREEN++;
                }
                if(bBLUE < WhiteBlueMax)
                {
                    bBLUE++;
                }
                if(bGREEN >WhiteGreenMax)
                {
                    bGREEN--;                   
                }                  
             
                if((bBLUE == WhiteBlueMax)&&(bGREEN == WhiteGreenMax)&&(bRED == WhiteRedMax))
                     LEDStatus[i] = PINK; 
                break;
            case PINK:

                if(bGREEN < GreenMax)   //增加到此状态最大值
                    bGREEN++;
                if(bGREEN > GreenMax)   //增加到此状态最大值
                    bGREEN--;

                if(bRED > 0) 
                    bRED--;
                if((bGREEN == GreenMax)&&(bRED == 0))
                    LEDStatus[i] = ORG;                
               
                break;

        }
        LEDDispBuffPtr[i].uRed = bRED;            
        LEDDispBuffPtr[i].uGreen = bGREEN;
        LEDDispBuffPtr[i].uBlue  = bBLUE;
    }       
}


/*********************************************************************************************************
** 函数名称: ModifyColorTable4
** 功能描述: 模式3数据变换
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
u8 bRED, bGREEN, bBLUE;

void ModifyColorTable2(void)
{
#define TAB2_WHITE      0    
#define TAB2_BLUE       1
    u8 i;
    for (i = 0; i < wLED_Length; i++)
    {
        bRED     = LEDDispBuffPtr[i].uRed;
        bGREEN   = LEDDispBuffPtr[i].uGreen;
        bBLUE    = LEDDispBuffPtr[i].uBlue;   
        switch(LEDStatus[i])
        {
            case TAB2_WHITE:
                if(bRED < WhiteRedMax)        //增加到此状态最大值
                    bRED++;
                if(bBLUE < WhiteBlueMax)         //增加到此状态最大值
                    bBLUE++;
                if(bGREEN < WhiteGreenMax)
                    bGREEN++;                
                if(bBLUE > WhiteBlueMax)
                    bBLUE--;
                if((bRED == WhiteRedMax) && (bGREEN == WhiteGreenMax)&& (bBLUE == WhiteBlueMax))
                {
                    LEDStatus[i] = GRE;                    
                }
                break;
            case TAB2_BLUE:
                if(bBLUE < BlueMax)
                {
                    bBLUE ++;
                }
//                else
//                {
                    if(bRED > 0)     //增加到此状态最大值
                        bRED--;
                    if(bGREEN > 0)
                        bGREEN--;

//                }
                
//                if(bBLUE < BlueMax)
//                {
//                    bBLUE ++;
//                }
//                else
//                {
//                    if(bRED > 0)     //增加到此状态最大值
//                        bRED--;
//                    if(bGREEN > 0)
//                        bGREEN--;

//                }
                if((bGREEN == 0)&&(bRED == 0)&&(bBLUE == BlueMax))
                    LEDStatus[i] = ORG;                
                break;
        }
        LEDDispBuffPtr[i].uRed = bRED;            
        LEDDispBuffPtr[i].uGreen = bGREEN;
        LEDDispBuffPtr[i].uBlue  = bBLUE;
    }       
}

/*********************************************************************************************************
** 函数名称: ModifyColorTable5
** 功能描述: 模式1数据变换
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/
void ModifyColorTable5(void)
{
#define TAB5_ORG            0
#define TAB5_BLUE1          1
#define TAB5_WHITE          2
#define TAB5_BLUE2          3
    u8 i;
    u8 bRED, bGREEN, bBLUE;
    for (i = 0; i < wLED_Length; i++)
    {
        bRED     = LEDDispBuffPtr[i].uRed;
        bGREEN   = LEDDispBuffPtr[i].uGreen;
        bBLUE    = LEDDispBuffPtr[i].uBlue;   
        switch(LEDStatus[i])
        {
            case TAB5_ORG:
                if(bBLUE == 0)
                {
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bGREEN < ORGGreenMax)
                        bGREEN++;
                }
                else
                {
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bRED < ORGRedMax)     //增加到此状态最大值
                        bRED++;
                    if(bGREEN < ORGGreenMax)
                        bGREEN++;
                    if(bBLUE > 0)
                        bBLUE--;
                }
                if((bRED == ORGRedMax) && (bGREEN == ORGGreenMax)&&(bBLUE == 0))
                    LEDStatus[i] = TAB5_BLUE1;
                break;
            case TAB5_BLUE1:
                if(bRED == 0)
                {
                    if((bBLUE < BlueMax))   //增加到此状态最大值
                        bBLUE++;
                }
                else
                {
                    if(bRED > 0) 
                        bRED--;
                    if(bRED > 0) 
                        bRED--;
                    if(bGREEN > 0) 
                        bGREEN--;
                }
                if((bGREEN == 0)&&(bBLUE == BlueMax))
                    LEDStatus[i] = TAB5_WHITE;                
                break;

            case TAB5_WHITE:               
                if(bBLUE < WhiteBlueMax)    //考虑到时间上的一致性，0x55 每次执行 0xab每二次执行 0xff 每三次执行
                {
                    bBLUE++;
                }
                if(bBLUE > WhiteBlueMax)
                {
                    bBLUE--;
                }
                if(bRED < WhiteRedMax)
                {
                    bRED++;
                }
                if(bGREEN <  WhiteGreenMax)
                {
                    bGREEN++;                   
                }                                 
                if((bBLUE == WhiteBlueMax)&&(bGREEN == WhiteGreenMax))
                     LEDStatus[i] = TAB5_BLUE2; 
                break;
            case TAB5_BLUE2:
                if(bRED == 0)
                {
                    if((bBLUE < BlueMax))   //增加到此状态最大值
                        bBLUE++;    
                }
                else
                {
                    if(bRED > 0) 
                        bRED--;
                    if(bGREEN > 0) 
                        bGREEN--;
                }
                if((bGREEN == 0)&&(bBLUE == BlueMax))
                    LEDStatus[i] = TAB5_ORG;                
                break;

        }
        LEDDispBuffPtr[i].uRed = bRED;            
        LEDDispBuffPtr[i].uGreen = bGREEN;
        LEDDispBuffPtr[i].uBlue  = bBLUE;
    }       
}
/*********************************************************************************************************
** 函数名称: AlterDataRdy
** 功能描述: 报警时数据内容
** 输　入: 	 i
** 输　出 :  无
** 全局变量: 无							  
** 调用模块: 无
********************************************************************************************************/

void AlterDataRdy(u8 Radio)
{
    u8 i;
    u16 RedMax = 0xaa;
    RedMax = (RedMax * Radio) / 100;
       
    LED_RGB_Data_TypeDef   *BufferPtr;    
    BufferPtr = (LED_RGB_Data_TypeDef*)&LEDLightAllBuffer[0];
        
    for(i=0;i<72;i++)
    {
        BufferPtr->uRed = (u8)RedMax;        //指针内容赋值
        BufferPtr->uGreen = 0;
        BufferPtr->uBlue = 0;
        BufferPtr++;                        //指向下一组元素
    }
}

void ColorRadioDnBak(void)
{
    u8 i;
    for(i=0;i<72;i++)
    {
		LEDLightAllBufferBak[i].uRed = LEDLightAllBuffer[i].uRed;
		LEDLightAllBufferBak[i].uGreen = LEDLightAllBuffer[i].uGreen;		
		LEDLightAllBufferBak[i].uBlue =  LEDLightAllBuffer[i].uBlue;		
    }
}
