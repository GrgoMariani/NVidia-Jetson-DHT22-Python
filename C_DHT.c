/*
 * Copyright (c) 2017 Grgo Mariani
 * Gnu GPL license
 * This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jetsonGPIO/jetsonGPIO.h"
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <Python.h>


// This is currently set to work with Jetson Xavier
#define PIN0 jetsonxavier_pin37                                  //Define which pin on your Jetson is connected to DHT22 as PIN0 when you use C_DHT.readSensor(0)
#define PIN1 jetsonxavier_pin29								     //Define which pin on your Jetson is connected to DHT22 as PIN1 when you use C_DHT.readSensor(1),    Check jetsonGPIO.h for more

typedef struct	{
	int *byte0;	int *byte1;	int *byte2;	int *byte3;	int *byteCheckSum;
}	five_bytes;

int isCheckSumOK(five_bytes *f)	{	return ( ( ((int)f->byte0 + (int)f->byte1 + (int)f->byte2 + (int)f->byte3) & 0xff )  == (int)f->byteCheckSum );	}
//! MODIFY THESE TWO FUNCTIONS IF YOU WANT TO USE DHT11 SENSOR
float getHumid(five_bytes *f)	{	return  (double)( ( ( (int)f->byte0)<<8 ) | (int)f->byte1 )/10.;												}
float getTemp(five_bytes *f)	{
	if( ( ((int)f->byte2)>>6)&0x01 ){
		int temperature = ( ( ( (int)f->byte2)<<8 ) | (int)f->byte3 );
		for(int i=0; i<sizeof(temperature)-2; i++)
			temperature |= ((0xff)<<(16+8*i));
		return (double)temperature/10.;
	}
	else return  (double)( ( ( (int)f->byte2)<<8 ) | (int)f->byte3 )/10.;
}

float getHumidDHT11(five_bytes *f)	{	return  (double)( (int)f->byte0 ); }
float getTempDHT11(five_bytes *f)	{	return  (double)( (int)f->byte2 ); }

void doublecheck(int ivar[] ){
	ivar[0] &= 0x03;
	int count1s=0;
	int temporary=ivar[2] & 0xfc;
	for(int i=2; i<8; i++)
		if( (temporary>>i)&0x01 )
			count1s++;
	if(count1s>=3) 	ivar[2] |= 0xfc;
	else 			ivar[2] &= 0x03;
}

five_bytes *getbytes(int size, clock_t times[]){
	five_bytes *result=malloc(sizeof(*result));
	result->byte0 = 0, result->byte1=0, result->byte2=0, result->byte3=0, result->byteCheckSum=0;
	int ivar[5]={0};
	if(size<80) {result->byteCheckSum=1; return result;}
	for(int i=size-80; i<size; i=i+2)
		if((double)times[i]/CLOCKS_PER_SEC >= 0.000053)
			ivar[(int)((i-size+80)/2)/8] |= (0x01<<(  7-((i-size+80)/2)%8  ));
	//doublecheck(ivar);
	result->byte0=(int)ivar[0],	result->byte1=(int)ivar[1],	result->byte2=(int)ivar[2],	result->byte3=(int)ivar[3],	result->byteCheckSum=(int)ivar[4];
	return result;
}

void *communicate(void *param){
	int *sensor=param;
	int counter=0;
	clock_t times_all[84]={0};
	clock_t time_over = CLOCKS_PER_SEC*0.0055;										// The loop takes 5.5ms at most
	clock_t begin = clock(), end=clock();
	unsigned int val1=high, val2=high;
	gpioExport(*sensor);
	gpioSetDirection(*sensor,outputPin) ;
	gpioSetValue(*sensor, low);
	usleep(2000);
	gpioSetValue(*sensor, high);
	gpioSetDirection(*sensor,inputPin) ;
	begin = clock();
	while(1){
		end = clock();
		gpioGetValue(*sensor, &val1) ;
		if (val1!=val2)
			{
				times_all[counter]=end-begin;
				counter++;
				val2=val1;
			}
		if(end-begin > time_over || counter>84)
			break;
	}
	gpioSetDirection(*sensor,outputPin) ;
	gpioSetValue(*sensor, high);
	gpioUnexport(*sensor);
	
	for(int i=counter-1; i>0;i--)
		times_all[i]=times_all[i]-times_all[i-1];
		
	five_bytes *result=getbytes(counter, times_all);
	return (void *) result;
}

static PyObject * readSensor(PyObject *self, PyObject *args){
	int sensor;
	if(!PyArg_ParseTuple(args, "i", &sensor))
		return NULL;
	sensor = (sensor==0 ? PIN0 : PIN1);
	void *result;
	pthread_t thread=(pthread_t)malloc( sizeof(pthread_t) );
	pthread_create(&thread, NULL, communicate, &sensor);
	pthread_join(thread,&result);
	PyObject * ret;
	five_bytes *bytes=result;
	float temperature=-39909, humidity=-39909;
	int valid_message=0;
	if(isCheckSumOK(bytes)){
		valid_message=1;
		temperature=getTemp(bytes);
		humidity=getHumid(bytes);
		if(temperature>80 || temperature<-40 || humidity>100 || humidity<0) temperature=-39909, humidity=-39909, valid_message=0;        //WHEN CHECKSUM FAILS, Read Documentation
	}
	free(bytes);
	ret=Py_BuildValue("ffi",temperature,humidity,valid_message);
	return ret;
}


static PyObject * readSensorDHT11(PyObject *self, PyObject *args){
	int sensor;
	if(!PyArg_ParseTuple(args, "i", &sensor))
		return NULL;
	sensor = (sensor==0 ? PIN0 : PIN1);
	void *result;
	pthread_t thread=(pthread_t)malloc( sizeof(pthread_t) );
	pthread_create(&thread, NULL, communicate, &sensor);
	pthread_join(thread,&result);
	PyObject * ret;
	five_bytes *bytes=result;
	float temperature=-39909, humidity=-39909;
	int valid_message=0;
	if(isCheckSumOK(bytes)){
		valid_message=1;
		temperature=getTempDHT11(bytes);
		humidity=getHumidDHT11(bytes);
		if(temperature>80 || temperature<-40 || humidity>100 || humidity<0) temperature=-39909, humidity=-39909, valid_message=0;        //WHEN CHECKSUM FAILS, Read Documentation
	}
	free(bytes);
	ret=Py_BuildValue("ffi",temperature,humidity,valid_message);
	return ret;
}



static PyMethodDef C_DHT22Methods[] = {
	{"readSensor", 		readSensor, 		METH_VARARGS, "Read all sensor values and return (float)temperature, (float)humidity and (0 or 1)message validity. If checksum not OK return -39909 (ERROR) as Humid and Temperature"},
	{"readSensorDHT11", readSensorDHT11, 	METH_VARARGS, "Read all sensor values and return (float)temperature, (float)humidity and (0 or 1)message validity. If checksum not OK return -39909 (ERROR) as Humid and Temperature. (Only for DHT11)"},
	{NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC 
initC_DHT(void){
	(void)Py_InitModule("C_DHT", C_DHT22Methods);
};
