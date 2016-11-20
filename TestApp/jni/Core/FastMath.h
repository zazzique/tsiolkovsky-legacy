//
//  FastMath.h
//  Carnivores
//
//  Created by Dmitry Nechay on 18.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#ifndef _FAST_MATH_H_
#define _FAST_MATH_H_

#define DEG2RAD(a) ((a) * 0.01745329251994329576f)
#define RAD2DEG(a) ((a) * 57.2957795130823208767f)

#define pi 3.1415926536f

extern float sin_table[8192];
extern float cos_table[8192];

void FastMath_Init();

void InitFastSinCosTable();

inline float FastSin(float a)
{
	while (a < 0.0f)
		a += 2.0f * pi;
    while (a > 2.0f * pi)
		a -= 2.0f * pi;

	return sin_table[(int)(a * 4096.0f * (1.0f / (2.0f * pi))) & 4095];
}

inline float FastCos(float a)
{
	while (a < 0.0f)
		a += 2.0f * pi;
    while (a > 2.0f * pi)
		a -= 2.0f * pi;

	return cos_table[(int)(a * 4096.0f * (1.0f / (2.0f * pi))) & 4095];
}

inline void FastSinCos(float a, float *sin_r, float *cos_r)
{
	while (a < 0.0f)
		a += 2.0f * pi;
    while (a > 2.0f * pi)
		a -= 2.0f * pi;
	
	register int table_index = (int)(a * 4096.0f * (1.0f / (2.0f * pi))) & 4095;
	*sin_r = sin_table[table_index];
	*cos_r = cos_table[table_index];
}

inline int FastTrunc(float f)
{
	f -= 0.5f;
	f += (3 << 22);
	return *((int*)&f) - 0x4b400000;
}

float AngleDifference(float a, float b);

inline void BoundAngle_0_360(float *a)
{
	while (*a < 0.0f)
		*a += 2.0f * pi;
    while (*a > 2.0f * pi)
		*a -= 2.0f * pi;
}

inline void BoundAngle_0_360_Deg(float *a)
{
	while (*a < 0.0f)
		*a += 360.0f;
    while (*a > 360.0f)
		*a -= 360.0f;
}

inline void BoundAngle_Minus90_90_Deg(float *a)
{
	if (*a < -90.0f)
		*a = -90.0f;
    if (*a > 90.0f)
		*a = 90.0f;
}

#ifndef SQR
    #define SQR(x)     ((x) * (x))
#endif

#ifndef SGN
    #define SGN(x)     ((x) < 0 ? -1 : 1)
#endif

#ifndef MAX
    #define MAX(a, b)  (a > b ? a : b)
#endif

#ifndef MIN
    #define MIN(a, b)  (a < b ? a : b)
#endif

#ifndef ROUNDF
    #define ROUNDF(x)  (floorf(x + 0.5f))
#endif

void DeltaFunc(float *a, float b, float d);

float MathWave(float a);

void RGB2HSV(float *color_hsv, float *color_rgb);
void HSV2RGB(float *color_rgb, float *color_hsv);

float RandFloat();

#endif /* _FAST_MATH_H_ */
