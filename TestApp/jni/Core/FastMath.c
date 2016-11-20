//
//  FastMath.m
//  Carnivores
//
//  Created by Dmitry Nechay on 18.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#include "Common.h"
#include "FastMath.h"

float sin_table[8192];
float cos_table[8192];

void FastMath_Init()
{
	InitFastSinCosTable();
}

float AngleDifference(float a, float b)
{
	a -= b;
	if (a < 0)
		a = -a;
	if (a > pi)
		a = 2.0f * pi - a;
	return a;
}

void InitFastSinCosTable()
{
	for (int t = 0; t < 8192; t++)
	{
		sin_table[t] = sinf(t * (2.0f * pi) / 4096.0f);
		cos_table[t] = cosf(t * (2.0f * pi) / 4096.0f);
	}
}

void DeltaFunc(float *a, float b, float d)
{
	if (b > *a)
	{
		*a += d;
		
		if (*a > b)
			*a = b;
	}
	else
	{
		*a -= d;
		
		if (*a < b)
			*a = b;
	}
}

float MathWave(float a)
{
	return (-cosf(a * pi) + 1.0f) * 0.5f;
}

void RGB2HSV(float *color_hsv, float *color_rgb)
{
	float cmin, cmax, cdelta;
	
	cmin = MIN(MIN(color_rgb[0], color_rgb[1]), color_rgb[2]);
	cmax = MAX(MAX(color_rgb[0], color_rgb[1]), color_rgb[2]);
	
	color_hsv[2] = cmax;
	
	cdelta = cmax - cmin;
	
	if (cmax != 0)
		color_hsv[1] = cdelta / cmax;
	else
	{
		color_hsv[1] = 0;
		color_hsv[0] = -1.0f;
		return;
	}
	
	if (color_rgb[0] == cmax)
	{
		color_hsv[0] = (color_rgb[1] - color_rgb[2]) / cdelta;
	}
	else if (color_rgb[1] == cmax)
	{
		color_hsv[0] = 2 + (color_rgb[2] - color_rgb[0]) / cdelta;
	}
	else
	{
		color_hsv[0] = 4 + (color_rgb[0] - color_rgb[1]) / cdelta;
	}
	
	color_hsv[0] *= 60.0f;
	
	if (color_hsv[0] < 0.0f)
		color_hsv[0] += 360.0f;
}


void HSV2RGB(float *color_rgb, float *color_hsv)
{
	float f, p, q, t;
	int i;
	
	if (color_hsv[1] == 0)
	{
		color_rgb[0] = color_hsv[2];
		color_rgb[1] = color_hsv[2];
		color_rgb[2] = color_hsv[2];
		return;
	}
	
	color_hsv[0] /= 60.0f;
	i = (int)floorf(color_hsv[0]);
	f = color_hsv[0] - i;
	p = color_hsv[2] * (1.0f - color_hsv[1]);
	q = color_hsv[2] * (1.0f - color_hsv[1] * f);
	t = color_hsv[2] * (1.0f - color_hsv[1] * (1.0f - f));
	
	switch (i)
	{
		case 0:
			color_rgb[0] = color_hsv[2];
			color_rgb[1] = t;
			color_rgb[2] = p;
			break;
		case 1:
			color_rgb[0] = q;
			color_rgb[1] = color_hsv[2];
			color_rgb[2] = p;
			break;
		case 2:
			color_rgb[0] = p;
			color_rgb[1] = color_hsv[2];
			color_rgb[2] = t;
			break;
		case 3:
			color_rgb[0] = p;
			color_rgb[1] = q;
			color_rgb[2] = color_hsv[2];
			break;
		case 4:
			color_rgb[0] = t;
			color_rgb[1] = p;
			color_rgb[2] = color_hsv[2];
			break;
		default:
			color_rgb[0] = color_hsv[2];
			color_rgb[1] = p;
			color_rgb[2] = q;
			break;
	}
}

float RandFloat()
{
	return (float)rand()/(float)RAND_MAX;
}



