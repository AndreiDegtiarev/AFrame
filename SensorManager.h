/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2014 Andrei Degtiarev. All right reserved
  

  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame


  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/
#pragma once
#include "ISensor.h"
#include "MeasurementStatus.h"
#include "DataBuffer.h"

class SensorManager
{

protected: 
	ISensor *_sensor;
private:
	float _prev_value;
	float _last_value;
	MeasurementStatus _status;
	float _low_application_limit;
	float _high_application_limit;
	unsigned long _time_last_measurement;
	unsigned long _pause_length;
	DataBuffer<unsigned long,int> *_secBuffer;
	DataBuffer<unsigned int,int> *_minBuffer;
	DataBuffer<unsigned int,int> *_howrsBuffer;
public:
	SensorManager(ISensor *sensor,
				  float low_application_limit,
				  float high_application_limit,
				  unsigned long pause_length)
	{
		_sensor=sensor;
		_prev_value = 0;
		_last_value = 0;
		_low_application_limit=low_application_limit;
		_high_application_limit=high_application_limit;
		_status=OK;
		_time_last_measurement = 0.0;
		_pause_length = pause_length;
		int buf_size=24;
		_secBuffer=new DataBuffer<unsigned long,int>(1,pow(10,sensor->Precission()),buf_size);
		_minBuffer=new DataBuffer<unsigned int,int>(1/60.0,pow(10,sensor->Precission()),buf_size);
		_howrsBuffer=new DataBuffer<unsigned int,int>(1/(60.0*60.0),pow(10,sensor->Precission()),buf_size);
	}
	ISensor *Sensor()
	{
		return _sensor;
	}
	DataBufferBase *SecBuffer()
	{
		return _secBuffer;
	}
	DataBufferBase *MinBuffer()
	{
		return _minBuffer;
	}
	DataBufferBase *HowrsBuffer()
	{
		return _howrsBuffer;
	}
	bool IsReadyForMeasurement()
	{
		return millis()-_time_last_measurement>_pause_length;
	}
	/*void InitMeasurements()
	{
		_status = OK;
	}*/
	bool isDiffer(float prev_value,float value)
	{
		//float max_val=max(fabs(_prev_value),fabs(value));
		//return max_val>0 && fabs(_prev_value-value)/max_val>_change_tolerance;
		return prev_value!=value;
	}
	bool IsChanged()
	{
		//Serial.print(_prev_value);
		//Serial.print(" ");
		//Serial.println(_last_value);
		if(_status != Error && isDiffer(_prev_value,_last_value))
			return true;
		return false;
	}
	virtual float GetData()
	{
		if(isDiffer(_prev_value,_last_value))
		{
			_prev_value=_last_value;
		}
		return _last_value;
	}
	void Measure()
	{
		/*Log::Number("Setting value:",value);
		Log::Number(" low:",_low_limit);
		Log::Number(" hight:",_high_limit,true);*/
		float value;
		_status=Error;
		if(_sensor->Measure(value))
		{
			_status=OK;
			if(value<_sensor->LowMeasurementLimit() || value >_sensor->HighMeasurementLimit())
			{
				_status=Error;
				return;
			}
			if(value<_low_application_limit || value >_high_application_limit)
			{
				_status=ApplicationAlarm;
			}
			if(_sensor->Precission() == 0)
				value=lrint(value);
			else
			{
				float factor=pow(10,_sensor->Precission());
				value=lrint(value*factor)/factor;
			}
			_time_last_measurement = millis();
			_last_value=value;
			//Log::Number("After rounding:",_last_value,true);
			unsigned long cursec=_time_last_measurement/1000;
			_secBuffer->AddValue(cursec,value);
			unsigned long last_sec_time = _minBuffer->X(_minBuffer->Size()-1);
			if(last_sec_time == 0 || (cursec-last_sec_time)>3600/_minBuffer->Size())
				_minBuffer->AddValue(cursec,value);
			unsigned long last_howr_time = _howrsBuffer->X(_howrsBuffer->Size()-1);
			if(last_howr_time == 0 || (cursec-last_howr_time)>3600)
				_howrsBuffer->AddValue(cursec,value);
		}
	}
	MeasurementStatus Status()
	{
		return _status;
	}
};
