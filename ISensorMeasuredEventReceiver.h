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
class SensorManager;
///Receiver for events that occurs after each measurement. Event will always generated event if measurements produce same results
class ISensorMeasuredEventReceiver
{
public:
	///In order to get event the function has to be overriden in the derived class
	virtual void NotifySensorMeasured(SensorManager *sensorManager)=0;
};