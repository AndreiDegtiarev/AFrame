/*
AFrame - Arduino framework library for ASensor and AWind libraries
Copyright (C)2014-2018 Andrei Degtiarev. All right reserved

You can always find the latest version of the library at
https://github.com/AndreiDegtiarev/AFrame

This library is free software; you can redistribute it and/or
modify it under the terms of the MIT license.
Please see the included documents for further information.
*/
#pragma once
#if defined __arm__ //DUE
#include <Arduino.h>
#else
#include "HardwareSerial.h"
#endif
#if defined(ESP8266)|| defined(ESP32)
//#define max(a, b) ((a)>(b) ? (a) : (b))
//#define min(a, b) ((a)<(b) ? (a) : (b))
#define F(s)     (s)
#define __FlashStringHelper char
#endif