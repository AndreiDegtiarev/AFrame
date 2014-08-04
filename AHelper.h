#pragma once
/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2014 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.
*/
#include "Log.h"
///Implements number of different helper functions
class AHelper
{
public:
	///Logs free SRAM memory. SRAM memory is a critical issue especially for application that perform data logging
	static int LogFreeRam ()
	{
		extern int __heap_start, *__brkval; 
		int v; 
		int fr = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
		out<<F("Free ram: ");
		out<<fr<<endl;

	}
	///Calculates number of charachters in a number.
	/**
	\param number target number
	\param prec number of decimal points
	\return number of charachters that represents input value
	*/
	static int GetNumberLength(float number,int prec)
	{
		return (number==0?0:log10(number))+2+prec+(number<0?1:0);
	}
};