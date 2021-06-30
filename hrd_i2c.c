/********************************************************************************************/
/* 2011 MEAS France													*/
/********************************************************************************************/

/********************************************************************************************/
/* Name : hrd_i2c.c																			*/
/* Date : Sep 26 2011																		*/
/* Author : William Markezana																*/
/********************************************************************************************/

/********************************************************************************************/
/* INCLUDES																					*/
/********************************************************************************************/
#include "hrd_i2c.h"

/********************************************************************************************/
/* PUBLIC FUNCTIONS																			*/
/********************************************************************************************/
static U16 _u16Timeout;
#define TIMEOUT	(5000)

/*------------------------------------------------------------------------------------------*/
void vHRD_I2C_Initialize(void)
/*------------------------------------------------------------------------------------------*/
{
   OpenI2C1(I2C_ON, I2C_BRG);
}

/*------------------------------------------------------------------------------------------*/
void vHRD_I2C_Start(void)
/*------------------------------------------------------------------------------------------*/
{
   StartI2C1();
   _u16Timeout = TIMEOUT;
   while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || I2C1CONbits.RCEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT) && --_u16Timeout);	
}

/*------------------------------------------------------------------------------------------*/
void vHRD_I2C_Stop(void)
/*------------------------------------------------------------------------------------------*/
{
   StopI2C1();	
   _u16Timeout = TIMEOUT;
   while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || I2C1CONbits.RCEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT) && --_u16Timeout);	
}

/*------------------------------------------------------------------------------------------*/
e_Ack eHRD_I2C_WriteByte(U8 txByte)
/*------------------------------------------------------------------------------------------*/
{
   I2C1TRN = txByte;

   if(I2C1STATbits.IWCOL)        /* If write collision occurs,return -1 */
      return 8;
   else
   {
      _u16Timeout = TIMEOUT;
      while( I2C1STATbits.TBF && --_u16Timeout);   // wait until write cycle is complete                 
   }
	
   _u16Timeout = TIMEOUT;
   while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || I2C1CONbits.RCEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT) && --_u16Timeout);	
   return I2C1STATbits.ACKSTAT;	
}

/*------------------------------------------------------------------------------------------*/
U8 u8HRD_I2C_ReadByte(e_Ack ack)
/*------------------------------------------------------------------------------------------*/
{
   U8 rxByte=0;
	
   I2C1CONbits.RCEN = 1;
   _u16Timeout = TIMEOUT;
   while(I2C1CONbits.RCEN && --_u16Timeout);
   I2C1STATbits.I2COV = 0;
   rxByte = I2C1RCV;	
	
   I2C1CONbits.ACKDT = ack;
    I2C1CONbits.ACKEN = 1;
   _u16Timeout = TIMEOUT;
   while((I2C1CONbits.SEN || I2C1CONbits.PEN || I2C1CONbits.RSEN || I2C1CONbits.RCEN || I2C1CONbits.ACKEN || I2C1STATbits.TRSTAT) && --_u16Timeout);	
   return rxByte;            
}

/********************************************************************************************/
/* END OF FILE																				*/
/********************************************************************************************/
