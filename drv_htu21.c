/********************************************************************************************/
/* MEAS France, Toulouse													*/
/********************************************************************************************/
/* Controller: dsPIC33FJ128GP802 
/* Compiler: MPLAB C30
/* Brief: This source code is an example of basic commands for HTU21 communication.
/*		eDRV_HTU21_MeasureHumidity
/*		eDRV_HTU21_MeasureTemperature
/*		eDRV_HTU21_Reset
/*		eDRV_HTU21_GetSerialNumber
/*
/********************************************************************************************/

/********************************************************************************************/
/* Name : drv_htu21.c																		*/
/* Date : Sep 26 2011																		*/
/* Author : William Markezana																*/
/********************************************************************************************/

/********************************************************************************************/
/* INCLUDES																					*/
/********************************************************************************************/
#include "drv_htu21.h"

/********************************************************************************************/
/* PRIVATE PROTOTYPES																		*/
/********************************************************************************************/
e_Error eCheckCrc(U8 data[], U8 nbrOfBytes, U8 checksum);
e_Error eReadUserRegister(U8 *pRegisterValue);
e_Error eWriteUserRegister(U8 *pRegisterValue);
e_Error eMeasureHM(etSHT2xMeasureType eSHT2xMeasureType, U16 *pMeasurand);
e_Error eMeasurePOLL(etSHT2xMeasureType eSHT2xMeasureType, U16 *pMeasurand);
F32 f32CalcRH(U16 u16sRH);
F32 f32CalcTemperatureC(U16 u16sT);

/********************************************************************************************/
/* PRIVATE FUNCTIONS																		*/
/********************************************************************************************/

/*------------------------------------------------------------------------------------------*/
e_Error eCheckCrc(U8 data[], U8 nbrOfBytes, U8 checksum)
/*------------------------------------------------------------------------------------------*/
{
	U8 crc = 0;
	U8 _bit;
	U8 byteCtr;

	for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
	{ 
		crc ^= (data[nbrOfBytes-1-byteCtr]);
		for (_bit = 8; _bit > 0; --_bit)
		{ 
			if (crc & 0x80) 
			{
				crc = (crc << 1) ^ 0x0131;
			}
		    else
		    {
			    crc = (crc << 1);
			}
		}
	}
	if (crc != checksum)
	{
		return CHECKSUM_ERROR;
	}
	else
	{
		return NO_ERROR;
	}
}

/*------------------------------------------------------------------------------------------*/
e_Error eReadUserRegister(U8 *pRegisterValue)
/*------------------------------------------------------------------------------------------*/
{	
	U8 checksum; 
	e_Error error = NO_ERROR;  
	
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_R);
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);
	*pRegisterValue = u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);
	error |= eCheckCrc(pRegisterValue,1,checksum);
	vHRD_I2C_Stop();
	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eWriteUserRegister(U8 *pRegisterValue)
/*------------------------------------------------------------------------------------------*/
{
	e_Error error = NO_ERROR;   //variable for error code
	
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_W);
	error |= eHRD_I2C_WriteByte (*pRegisterValue);
	vHRD_I2C_Stop();
	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eMeasureHM(etHTU21MeasureType eHTU21MeasureType, U16 *pMeasurand)
/*------------------------------------------------------------------------------------------*/
{
	U8  checksum;                //checksum
	e_Error error = NO_ERROR;    //error variable

	//-- write I2C sensor address and command --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W); // I2C Adr
	switch(eHTU21MeasureType)
	{ 
	   case HUMIDITY: error |= eHRD_I2C_WriteByte (TRIG_RH_MEASUREMENT_HM); break;
    	   case TEMP    : error |= eHRD_I2C_WriteByte (TRIG_T_MEASUREMENT_HM);  break;
	   default: assert(0);
	}

	//-- wait hold master released --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);
	*pMeasurand = 0;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK)<<8;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);
	vHRD_I2C_Stop();

	//-- verify checksum --
	error |= eCheckCrc((U8*)pMeasurand,2,checksum);
	
	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eMeasurePOLL(etHTU21MeasureType eHTU21MeasureType, U16 *pMeasurand)
/*------------------------------------------------------------------------------------------*/
{
	U8  checksum;                //checksum
	e_Error error = NO_ERROR;    //error variable
	U16 timeout = 200;// 100ms

	//-- write I2C sensor address and command --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W); // I2C Adr
	switch(eHTU21MeasureType)
	{ 
	   case HUMIDITY: error |= eHRD_I2C_WriteByte (TRIG_RH_MEASUREMENT_POLL); break;
    	   case TEMP    : error |= eHRD_I2C_WriteByte (TRIG_T_MEASUREMENT_POLL);  break;
 	   default: assert(0);
 	}

	//-- wait hold master released --
	do
	{
		vHRD_I2C_Start();
		error = eHRD_I2C_WriteByte (I2C_ADR_R);
	}
	while(error &&(--timeout));

	*pMeasurand = 0;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK)<<8;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);

	//-- verify checksum --
	error |= eCheckCrc((U8*)pMeasurand,2,checksum);
	error |= ((timeout == 0)*TIME_OUT_ERROR);
	vHRD_I2C_Stop();

	return error;
}

/*------------------------------------------------------------------------------------------*/
F32 f32CalcRH(U16 u16sRH)
/*------------------------------------------------------------------------------------------*/
{
	F32 humidityRH;              // variable for result

	u16sRH &= ~0x0003;           // clear bits [1..0] (status bits)

	//-- calculate relative humidity [%RH] --
	humidityRH = -6.0 + 125.0/65536 * (float)u16sRH; // RH= -6 + 125 * SRH/2^16
	return humidityRH;
}

/*------------------------------------------------------------------------------------------*/
F32 f32CalcTemperatureC(U16 u16sT)
/*------------------------------------------------------------------------------------------*/
{
	float temperatureC;            // variable for result

	u16sT &= ~0x0003;              // clear bits [1..0] (status bits)

	//-- calculate temperature [°C] --
	temperatureC= -46.85 + 175.72/65536 *(float)u16sT; //T= -46.85 + 175.72 * ST/2^16
	return temperatureC;
}

/********************************************************************************************/
/* PUBLIC FUNCTIONS																			*/
/********************************************************************************************/

/*------------------------------------------------------------------------------------------*/
e_Error eDRV_HTU21_MeasureHumidity(F32 *pHumidity)
/*------------------------------------------------------------------------------------------*/
{
	U16 measure;
   	//e_Error error = eMeasureHM(HUMIDITY, &measure);
   	e_Error error = eMeasurePOLL(HUMIDITY, &measure);
   	*pHumidity = f32CalcRH(measure);
   	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eDRV_HTU21_MeasureTemperature(F32 *pTemperature)
/*------------------------------------------------------------------------------------------*/
{
	U16 measure;
   	//e_Error error = eMeasureHM(TEMP, &measure);
   	e_Error error = eMeasurePOLL(TEMP, &measure);
   	*pTemperature = f32CalcTemperatureC(measure);
   	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eDRV_HTU21_Reset(void)
/*------------------------------------------------------------------------------------------*/
{
	e_Error error = NO_ERROR;                    //error variable
	U16 i;
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     // I2C Adr
	error |= eHRD_I2C_WriteByte (SOFT_RESET);    // Command
	vHRD_I2C_Stop();
	for(i=0;i<=12000;i++);
	return error;
}

/*------------------------------------------------------------------------------------------*/
e_Error eDRV_HTU21_GetSerialNumber(U8 u8SerialNumber[])
/*------------------------------------------------------------------------------------------*/
{
	e_Error error = NO_ERROR;                    //error variable

	//Read from memory location 1
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     //I2C address
	error |= eHRD_I2C_WriteByte (0xFA);          //Command for readout on-chip memory
	error |= eHRD_I2C_WriteByte (0x0F);          //on-chip memory address
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);     //I2C address
	u8SerialNumber[5] = u8HRD_I2C_ReadByte(ACK); //Read SNB_3
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_3 (CRC is not analyzed)
	u8SerialNumber[4] = u8HRD_I2C_ReadByte(ACK); //Read SNB_2
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_2 (CRC is not analyzed)
	u8SerialNumber[3] = u8HRD_I2C_ReadByte(ACK); //Read SNB_1
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_1 (CRC is not analyzed)
	u8SerialNumber[2] = u8HRD_I2C_ReadByte(ACK); //Read SNB_0
	u8HRD_I2C_ReadByte(NACK);                    //Read CRC SNB_0 (CRC is not analyzed)
	vHRD_I2C_Stop();

	//Read from memory location 2
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     //I2C address
	error |= eHRD_I2C_WriteByte (0xFC);          //Command for readout on-chip memory
	error |= eHRD_I2C_WriteByte (0xC9);          //on-chip memory address
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);     //I2C address
	u8SerialNumber[1] = u8HRD_I2C_ReadByte(ACK); //Read SNC_1
	u8SerialNumber[0] = u8HRD_I2C_ReadByte(ACK); //Read SNC_0
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNC0/1 (CRC is not analyzed)
	u8SerialNumber[7] = u8HRD_I2C_ReadByte(ACK); //Read SNA_1
	u8SerialNumber[6] = u8HRD_I2C_ReadByte(ACK); //Read SNA_0
	u8HRD_I2C_ReadByte(NACK);                    //Read CRC SNA0/1 (CRC is not analyzed)
	vHRD_I2C_Stop();
	return error;
}

/********************************************************************************************/
/* END OF FILE																				*/
/********************************************************************************************/
