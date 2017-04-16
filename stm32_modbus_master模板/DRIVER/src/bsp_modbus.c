/*
*
*	主机服务程序
*
*/

#include "bsp_modbus.h"
#include "timer.h"
#include "bsp_timer4.h"
#include "bsp_usart3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern volatile u8 modbus_com2_over; //接收完成标志

u8 slaveraddr = 1;			//从机地址 
u8 regstartaddr = 0;		//数据开始储存的地址

extern u8 receCount2;
extern u8 sendCount2;	 
extern u8 sendBuf2[32];
extern u8 sendPosi2;
extern u8 checkoutError2;
extern volatile u8 modbus_com2_over;


const u8 auchCRCHi[] = { 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40} ; 


const u8 auchCRCLo[] = { 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,0x43, 0x83, 0x41, 0x81, 0x80, 0x40} ;


//crc16校验
u16 crc16(u8 *puchMsg, u16 usDataLen) 
{ 
	u8 uchCRCHi = 0xFF ; 
	u8 uchCRCLo = 0xFF ; 
	u32 uIndex ; 
	while (usDataLen--) 
	{ 
		uIndex = uchCRCHi ^ *puchMsg++ ; 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ; 
		uchCRCLo = auchCRCLo[uIndex] ; 
	} 
	return (uchCRCHi << 8 | uchCRCLo) ; 
}//uint16 crc16(uint8 *puchMsg, uint16 usDataLen)



/*
*   作用：打包命令
*
*/
void construct_rtu_frm ( u8 *dst_buf,u8 *src_buf,u8 lenth)
{
    unsigned short  crc_tmp; //
    crc_tmp = crc16(src_buf, lenth);	
    *(src_buf+lenth) = crc_tmp >> 8 ;
    *(src_buf+lenth+1) = crc_tmp & 0xff;
    lenth++;
    lenth++;
    while ( lenth--)//赋值
    {
       *dst_buf = *src_buf;
       dst_buf++;
       src_buf++;     
    }
}



/*
*   作用：读保持寄存器 03
*   参数：从机地址、缓存区、起始地址、读取长度
*/
u16  rtu_read_hldreg ( u8 board_adr,u8 *com_buf,u16 start_address,u16 lenth) //03
{
    unsigned char tmp[32], tmp_lenth;   
    tmp[0] = board_adr;//设备地址
    tmp[1] = READ_HLD_REG;//功能号
    tmp[2] = HI(start_address);//起始地址高位
    tmp[3] = LOW(start_address);
    tmp[4] = HI(lenth);//寄存器数量
    tmp[5] = LOW(lenth);
    tmp_lenth = 6;
    construct_rtu_frm ( com_buf,tmp,tmp_lenth);
	sendCount2=8;
	beginSend3();//发送封装后的东西
    return 8;//为了防止编译器报错//？？
}


/*
*   作用：写保持寄存器 06
*   参数：从机地址、缓存区、起始地址、写入值
*/
u16 rtu_set_hldreg( u8 board_adr,u8 *com_buf, u16 start_address, u16 value ) //06
{
    unsigned char tmp[32], tmp_lenth;   
    tmp[0] = board_adr;
    tmp[1] = SET_HLD_REG;
    tmp[2] = HI(start_address);
    tmp[3] = LOW(start_address);
    tmp[4] = HI(value);
    tmp[5] = LOW(value);  
    tmp_lenth = 6;
    construct_rtu_frm ( com_buf, tmp, tmp_lenth);
	sendCount2=8;
	beginSend3();
    return 8 ;
}



/*
*   作用：解析函数(校验、区分不同功能码)
*   参数：解析后数据存放、接收缓存区、起始地址、长度(校验位数)貌似
*/
int rtu_data_anlys( u16  *dest_p, u8 *source_p, u16 data_start_address, u16 fr_lenth)//rtu 接收分析//
{
    u16 crc_result, crc_tmp;
    u8 tmp1, tmp2, shift;  
	crc_tmp = *(source_p + fr_lenth-2); // crc  第一字节//
 	crc_tmp = crc_tmp * 256 + *( source_p+fr_lenth-1); // CRC 值// 
	crc_result = crc16(source_p, fr_lenth-2); // 计算CRC 值
	
    if ( crc_tmp == crc_result ) // CRC 校验正确//
    {   
        printf("\r\n校验正确\r\n");
		switch ( *(source_p+1) ) // 功能码//
		{
		   case READ_COIL:                   //读取继电器状态//
               for ( tmp1=0; tmp1<*( source_p+2); tmp1++)
               {
                    shift = 1;
                    for ( tmp2=0; tmp2<8; tmp2++)
                    { 
                         *(dest_p+data_start_address+tmp1*8+tmp2) = shift & *( source_p+3);
                         *( source_p+3) >>= 1;          
                    }
               }
		   break;
		   case READ_DI: //读取开关量输入//
               for ( tmp1=0; tmp1<*( source_p+2); tmp1++)
               {
                    shift = 1;
                    for (tmp2=0; tmp2<8; tmp2 ++)
                    { 
                        *(dest_p+data_start_address+tmp1*8+tmp2) = shift & *( source_p+3);
                        *( source_p+3)>>=1;             
                    }
               }
		   break;
		   case READ_HLD_REG:  //读取保持寄存器//
               printf("\r\n解析03命令\r\n");
               for ( tmp1=0; tmp1<*( source_p+2); tmp1+=2)
               {
                    //relay17_he;
                    *(dest_p + data_start_address+ tmp1/2)= *( source_p+tmp1+3)*256 +  *( source_p+tmp1+4) ;
                    //relay17_he;        
               }
		   break ;
		   case 4:      //读取模拟量输入//
               for ( tmp1=0; tmp1<*( source_p+2); tmp1+=2)
               {
                    *(dest_p + data_start_address+ tmp1/2) = *( source_p+tmp1+3)*256 +  *( source_p+tmp1+4) ;      
               }
		   break;
		   case PROTOCOL_EXCEPTION:
                return -1*PROTOCOL_ERR;     //整数转换导致类型变化的迹象
		   //break;
		   default:
		   return -1*PROTOCOL_ERR;		//整数转换导致类型变化的迹象
		   // break;
		}
		receCount2=0;//收到信息计数器清零
	}
    else
    {
        printf("\r\n校验失败\r\n");
        receCount2=0;//收到信息计数器清零
        return -1*PROTOCOL_ERR;
    }
	return 0;	
}


//可变量：从机地址、起始地址、寄存器数量、结果存放
void modbus_rtu_dy(void) //电压读取命令
{
    u16 data_save[20];
    memset(data_save,0,sizeof(data_save));
    slaveraddr = 0x02;
//RE_RUN:
	rtu_read_hldreg(slaveraddr,sendBuf2,0,1);//读取多个，怎么处理？
    printf("Senddata: ");
    for(int i=0;i<8;i++)
        printf("%02x ",sendBuf2[i]);
    printf("\r\n");
    delay_ms(1000);
    delay_ms(1000);delay_ms(1000);delay_ms(1000);
    delay_ms(1000);
    printf("modbus_com2_over=%d,slaveraddr=%d,USART3_RX_BUF=%d\r\n",modbus_com2_over,slaveraddr,USART3_RX_BUF[0]);
    
    if(modbus_com2_over ==2&& USART3_RX_BUF[0] == slaveraddr)//上次发送地址的回文
	{
        modbus_com2_over = 0;
//        printf("\r\nUSART3_RX_BUF =\r\n");
		if( -1 == rtu_data_anlys(data_save,USART3_RX_BUF,0,12))//校验等出错
        {
            memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存
            //goto RE_RUN;
        }            
	}
    else if(modbus_com2_over!=2 || USART3_RX_BUF[0] != slaveraddr)
	{
		receCount2=0;
        memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存
	}
    
    for(int i = 0;i<5;i++)
        printf("* = %d ",data_save[i]);
    printf("\r\n");
    
    //slaveraddr++;
}

void test_modbus_master_read(void)
{
    //温度、湿度、大气压 一个地址3个寄存器
	//风速 一个地址，一个寄存器
	//风向 一个地址，一个寄存器
	int slaveraddr = 0x02;
	//int startaddr = 0;
    //int reg_num = 0;
	uint16_t data_save[20];
    memset(data_save,0,sizeof(data_save));
	
    
    for(int i = 1;i < 3;i++)
	{
		switch(i+1)
		{
			case 1://
//				rtu_read_hldreg(slaveraddr,sendBuf2,0,1);
//				rtu_data_anlys(&data_save[0],USART3_RX_BUF,0,12);
//				
//				rtu_read_hldreg(slaveraddr,sendBuf2,1,1);
//				rtu_data_anlys(&data_save[1],USART3_RX_BUF,0,12);
//				
//				rtu_read_hldreg(slaveraddr,sendBuf2,2,1);
//				rtu_data_anlys(&data_save[2],USART3_RX_BUF,0,12);
				break;
			case 2:
                rtu_read_hldreg(slaveraddr,sendBuf2,0,1);
                delay_ms(1000);
                
                if(modbus_com2_over ==2&&USART3_RX_BUF[0] == slaveraddr)//上次发送地址的回文
                {
                    modbus_com2_over = 0;
                    if( -1 == rtu_data_anlys(&data_save[3],USART3_RX_BUF,0,12))//校验等出错
                    {
                        memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存
                        receCount2=0;
                    }                    
                }
                else if(modbus_com2_over!=2 || USART3_RX_BUF[0] != slaveraddr)
                {
                    receCount2=0;
                    memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存
                }
				break;
			case 3:
                rtu_read_hldreg(slaveraddr,sendBuf2,0,1);
                delay_ms(1000);
                
                if(modbus_com2_over ==2&&USART3_RX_BUF[0] == slaveraddr)//上次发送地址的回文
                {
                    modbus_com2_over = 0;
                    if( -1 == rtu_data_anlys(&data_save[4],USART3_RX_BUF,0,12))//校验等出错
                    {
                        receCount2=0;
                        memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存   
                    }                    
                }
                else if(modbus_com2_over!=2 || USART3_RX_BUF[0] != slaveraddr)
                {
                    receCount2=0;
                    memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));//清空缓存
                }
			default: ;	
		}
		slaveraddr++;
	}
    
    printf("\r\n");
    for(int i = 0;i<10;i++)
    printf(" %d ",data_save[i]);
}






//	regstartaddr=regstartaddr+10;
//	if(slaveraddr > boardnum) //轮询11个地址 不超过
//	{
//		//FLAG_caiji=1;
//		slaveraddr=1;
//		regstartaddr=0;
//	}//发命令
    //解析命令
//	if(modbus_com2_over==2&&USART3_RX_BUF[0]==11) //上次遗留的
//	{
//		modbus_com2_over=0;//串口2发送完成
//		//rtu_data_anlys(nzval,USART3_RX_BUF,100,25);		
//	}
//	else if(modbus_com2_over==2&&USART3_RX_BUF[0]+1==slaveraddr)//??12 0C 0000_1100//上次发送地址的回文
//	{
//		modbus_com2_over=0;
//		//rtu_data_anlys(adcval,USART3_RX_BUF,regstartaddr-10,25);		
//	}
//	else if(modbus_com2_over!=2 || USART3_RX_BUF[0]+1 != slaveraddr)
//	{
//		//FLAG_comerr=1;
//		receCount2=0;
//	}




//u16  rtu_neizu_order ( u8 board_adr,u8 *com_buf,u16 start_address,u16 lenth) //开启内阻采集命令//
//{
//    unsigned char tmp[32], tmp_lenth;  
//    tmp[0] = board_adr;
//	tmp[1] = NEIZU_CAIJI;
//	tmp[2] = HI(start_address);
//	tmp[3] = LOW(start_address);
//	tmp[4] = HI(lenth);
//	tmp[5] = LOW(lenth);   
//    tmp_lenth = 6;
//    construct_rtu_frm ( com_buf,tmp,tmp_lenth);
//	sendCount2=8;
//	beginSend3();
//    return 8;
//}


//void modbus_rtu_nz(void)	//内阻读取命令
//{
//	if(modbus_com2_over==2&&receBuf2[0]==11)
//	{
//		modbus_com2_over=0;
//		rtu_data_anlys(adcval,receBuf2,100,25);		
//	}
//	else if(modbus_com2_over==2&&receBuf2[0]+1==slaveraddr)
//	{
//		modbus_com2_over=0;
//		rtu_data_anlys(nzval,receBuf2,regstartaddr-10,25); 
//	}
//	else if(modbus_com2_over!=2 || receBuf[0]+1 != slaveraddr)
//	{
//		FLAG_comerr=1;
//		receCount2=0;
//	}
//	rtu_read_hldreg(slaveraddr,sendBuf2,16,10);
//	slaveraddr++;
//	regstartaddr=regstartaddr+10;
//	if(slaveraddr>boardnum)//11以内查询
//	{
//		FLAG_caiji=0;
//		slaveraddr=1;
//		regstartaddr=0;
//	}
//}

//void modbus_rtu(void)  //数据采集命令
//{
//	if(FLAG_caiji==0)
//	{modbus_rtu_dy();}
//	else
//	{modbus_rtu_nz();}
//}

















