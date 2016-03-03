// Sebastian Evoniak
// The Worley Noise generator. This class generates and 
// stores the generated noise.

#pragma once

#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "RandPoint.h"

#define TYPE_CLOSEST_LINEAR 1
#define TYPE_2NDCLOSEST_LINEAR 2
#define TYPE_1STMINUS2ND_CLAMPED 3
#define TYPE_2NDMINUS1ST_CLAMPED 4
#define TYPE_1STMINUS2ND_UNCLAMPED 5
#define TYPE_2NDMINUS1ST_UNCLAMPED 6

class NoiseGenerator
{
private:
	int numPoints;
	RandPoint* points;
	int* refpts;
	int* refpts2;
	int width;
	int height;
	COLORREF* pixelColors;
	void (*updateProg)();

	void initPoints();
	int findRefPoint(int ptInd, int dist);
	void calculateColors(int type);
	double clamp(double, double, double);

public:
	NoiseGenerator(int w, int h, int pts, void (*func)());
	~NoiseGenerator(void);

	int getNumPoints();
	BYTE getRedOfPixel(int);
	BYTE getGreenOfPixel(int);
	BYTE getBlueOfPixel(int);
	void generateNoise(int type);
};

