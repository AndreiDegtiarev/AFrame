#pragma once
/*
AFrame - Arduino framework library for ASensor and AWind libraries
Copyright (C)2014-2018 Andrei Degtiarev. All right reserved

You can always find the latest version of the library at
https://github.com/AndreiDegtiarev/AFrame

This library is free software; you can redistribute it and/or
modify it under the terms of the MIT license.
Please see the included documents for further information.
*/
#include "ISensor.h"
#include "MeasurementStatus.h"
#include "SensorDataBuffer.h"
#include "ISensorHasDataEventReceiver.h"
#include "ISensorMeasuredEventReceiver.h"
#include "SensorCalibrator.h"
#include "LinkedList.h"

///Manage operations with sensors: triggers measurements, check appplication alarm status and etc.
class SensorManager
{

protected: 
	ISensor *_sensor;
	float _prev_value;                            //!< Prev measured value
	float _last_value;                           //!< Last measured value
	MeasurementStatus _status;                  //!<  Last measuremnt status
	float _low_application_limit;              //!< Defines application limit for measurements. Like inner temperature schould not be below 0 Celcius 
	float _high_application_limit;            //!< Defines application limit for measurements. Like humidity schould not be higher as 65 %
	unsigned long _time_last_measurement;    //!< Contains time as last succefull measurements were done
	unsigned long _pause_length;            //!< Pause between measurements [milliseconds]
	SensorDataBuffer *_secBuffer;          //!< Contains all last data
	SensorDataBuffer *_minBuffer;         //!< Contains last data with minute interval
	SensorDataBuffer *_howrsBuffer;      //!< Contains last data with hower interval
	LinkedList<ISensorHasDataEventReceiver> _eventReceivers;           //!< Receivers for results of meausurements that differs from previos one
	LinkedList<ISensorMeasuredEventReceiver> _eventMeasuredReceivers; //!< Receivers for results of last meausurements
	SensorCalibrator *_sensorCalibrator;                  //!< Recalculate sensor values into phisical ones
	
	const __FlashStringHelper* _applicationName;

public:
	///Constructor
	/**
	\param sensor derived from ISensor class
	\param low_application_limit defines application limit for measurements. Like inner temperature schould not be below 0 Celcius 
	\param high_application_limit defines application limit for measurements. Like humidity schould not be higher as 65 %
	\param pause_length pause between measurements [milliseconds]
	*/
	SensorManager(ISensor *sensor,
				  float low_application_limit,
				  float high_application_limit,
				  unsigned long pause_length_ms,bool withBuffer=true, const __FlashStringHelper* applicationName=NULL, SensorCalibrator *sensorCalibrator=NULL)
	{
		_sensor=sensor;
		_low_application_limit=low_application_limit;
		_high_application_limit=high_application_limit;
		_pause_length = pause_length_ms;
		_status= MeasurementStatus::MStOK;
		_prev_value = 0;
		_last_value = 0;
		_time_last_measurement = 0.0;
		_applicationName = applicationName;
		_sensorCalibrator = sensorCalibrator;
		//reduce this value if you have problem with SRAM
#ifdef DEBUG_AWIND
		const int buf_size=20;
#else
		const int buf_size=24;
#endif
		if (withBuffer)
		{
			_secBuffer = new SensorDataBuffer(1, pow(10, sensor->Precission()), buf_size);
			_minBuffer = new SensorDataBuffer(1 / 60.0, pow(10, sensor->Precission()), buf_size);
			_howrsBuffer = new SensorDataBuffer(1 / (60.0*60.0), pow(10, sensor->Precission()), buf_size);
		}
		else
		{
			_secBuffer = NULL;
			_minBuffer = NULL;
			_howrsBuffer = NULL;
		}
	}
	///Initialize buffer for results with second time steps
	void initSecondsBuffer(int buf_size)
	{
		_secBuffer = new SensorDataBuffer(1, pow(10, _sensor->Precission()), buf_size);
	}
	///Initialize buffer for results with minute time steps
	void initMinutesBuffer(int buf_size)
	{
		_minBuffer = new SensorDataBuffer(1 / 60.0, pow(10, _sensor->Precission()), buf_size);
	}
	///Initialize buffer for results with howr time steps
	void initHowrsBuffer(int buf_size)
	{
		_howrsBuffer = new SensorDataBuffer(1 / (60.0*60.0), pow(10, _sensor->Precission()), buf_size);
	}
	///Registers receiver for sensor measurement if it differs from previos one
	void RegisterHasDataEventReceiver(ISensorHasDataEventReceiver *eventReceiver)
	{
		_eventReceivers.Add(eventReceiver);
	}
	///Registers receiver for sensor measurement for every measurement event
	void RegisterMeasuredEventReceiver(ISensorMeasuredEventReceiver *eventReceiver)
	{
		_eventMeasuredReceivers.Add(eventReceiver);
	}
	///Returns applicationspecific name
	const __FlashStringHelper* AppName()
	{
		return _applicationName;
	}
	///Returns low application limit for measured values
	float LowApplicationLimit()
	{
		return _low_application_limit;
	}
	///Returns hight application limit for measured values
	float HightApplicationLimit()
	{
		return _high_application_limit;
	}
	///set low application limit for measured values
	void SetLowApplicationLimit(float limit)
	{
		_low_application_limit = limit;
	}
	///Returns hight application limit for measured values
	void SetHightApplicationLimit(float limit)
	{
		_high_application_limit = limit;
	}
	///Returns associated sensor
	ISensor *Sensor()
	{
		return _sensor;
	}
	///Sets pause between measurements
	/**
	\param pause_ms pause between measurements [milliseconds]
	*/
	void SetPause(unsigned long pause_ms)
	{
		_pause_length=pause_ms;
	}
	///Returns pause between measurements [milliseconds]
	unsigned long GetPause()
	{
		return _pause_length;
	}
	///Return internal buffer for all last measurements (check buf_size parameter)
	SensorDataBuffer *SecBuffer()
	{
		return _secBuffer;
	}
	///Return internal buffer for last measurements with minutes interval (check buf_size parameter)
	SensorDataBuffer *MinBuffer()
	{
		return _minBuffer;
	}
	///Return internal buffer for last measurements with howrs interval (check buf_size parameter)
	SensorDataBuffer *HowrsBuffer()
	{
		return _howrsBuffer;
	}
	///Returns true if measurements need to be performed
	bool IsReadyForMeasurement()
	{
		return millis()-_time_last_measurement>_pause_length;
	}
	///Returns true if there are new measurements
	bool IsChanged()
	{
		if(_status != Error && isDiffer(_prev_value,_last_value))
			return true;
		return false;
	}
	///Returns last measurements
	float GetData()
	{
		if(isDiffer(_prev_value,_last_value))
		{
			_prev_value=_last_value;
		}
		return _last_value;
	}
	///Triggers sensor measurements
	void Measure()
	{
		float value=0;
		_status=Error;
		if(_sensor->Measure(value))
		{
			if (_sensorCalibrator)
				value = _sensorCalibrator->Value(value);
			_status= MeasurementStatus::MStOK;
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
			unsigned long cursec=_time_last_measurement/1000;
			if(_secBuffer!=NULL)
				_secBuffer->AddValue(cursec,value);
			if (_minBuffer != NULL)
			{
				unsigned long last_sec_time = _minBuffer->X(_minBuffer->Size() - 1);
				if (last_sec_time == 0 || (cursec - last_sec_time) > 3600 / _minBuffer->Size())
					_minBuffer->AddValue(cursec, value);
			}
			if (_howrsBuffer)
			{
				unsigned long last_howr_time = _howrsBuffer->X(_howrsBuffer->Size() - 1);
				if (last_howr_time == 0 || (cursec - last_howr_time) > 3600)
					_howrsBuffer->AddValue(cursec, value);
			}
			if (IsChanged())
			{
				for(int i=0;i<_eventReceivers.Count();i++)
					_eventReceivers[i]->NotifySensorHasData(this);
			}
			for (int i = 0; i<_eventMeasuredReceivers.Count(); i++)
				_eventMeasuredReceivers[i]->NotifySensorMeasured(this);

		}
	}
	///Return status of last measurements
	MeasurementStatus Status()
	{
		return _status;
	}
private:
	///Return true if two values are differ
	bool isDiffer(float prev_value,float value)
	{
		return prev_value!=value;
	}

};
