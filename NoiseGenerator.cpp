#include "NoiseGenerator.h"

// Contrsutor that takes in a width, height, number of points, and 
// a callback function for updating the GUI's progress bar.
NoiseGenerator::NoiseGenerator(int w, int h, int pts, void (*func)()):
width(w),
height(h)
{
	numPoints = max(pts, 1);
	points = new RandPoint[numPoints];
	refpts = new int[width * height];
	refpts2 = new int[width * height];
	pixelColors = new COLORREF[width * height];
	updateProg = func;
	initPoints();
	calculateColors(TYPE_CLOSEST_LINEAR);
}


NoiseGenerator::~NoiseGenerator(void)
{
	if (points)
		delete[] points;
	if (pixelColors)
		delete[] pixelColors;
	if (refpts)
		delete[] refpts;
	if (refpts2)
		delete refpts2;
}

int NoiseGenerator::getNumPoints() { return numPoints; }

// These 3 functions return the R, G, and B values of a given pixel
BYTE NoiseGenerator::getRedOfPixel(int index) { return GetRValue(pixelColors[index]); }

BYTE NoiseGenerator::getGreenOfPixel(int index) { return GetGValue(pixelColors[index]); }

BYTE NoiseGenerator::getBlueOfPixel(int index) { return GetBValue(pixelColors[index]); }

// Initialises the points to random positions, and determines both
// the closest and second-closest points for each image pixel
void NoiseGenerator::initPoints()
{
	srand((UINT)time(NULL));
	for (int i = 0; i < numPoints; i++)
	{
		points[i].x = rand() % width;
		points[i].y = rand() % height;
	}

	int totalNumCalcs = 2 * width * height;
	int interval = totalNumCalcs / 1000;
	int calcsDone = 0;

	for (int i = 0; i < width * height; i++)
	{
		refpts[i] = findRefPoint(i, 0);
		calcsDone++;
		if (calcsDone % interval == 0)
			updateProg();
	}

	for (int i = 0; i < width * height; i++)
	{
		refpts2[i] = findRefPoint(i, 1);
		calcsDone++;
		if (calcsDone % interval == 0)
			updateProg();
	}
}

// Uses the distances to the closest and second closest points to calculate
// an intensity value for each pixel, according to the requested noise type
void NoiseGenerator::calculateColors(int type)
{
	switch (type)
	{
	case TYPE_CLOSEST_LINEAR:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx = x - points[refpts[i]].x;
				double dy = y - points[refpts[i]].y;
				double dist = sqrt(dx*dx + dy*dy);
				double intensity = clamp(255 * (1.0 - dist / 85.0), 0.0, 255.0);
				BYTE col = (BYTE)intensity;
				pixelColors[i] = RGB(col,col,col);
			}
			break;
		}

	case TYPE_2NDCLOSEST_LINEAR:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx2 = x - points[refpts2[i]].x;
				double dy2 = y - points[refpts2[i]].y;
				double dist2 = sqrt(dx2*dx2 + dy2*dy2);
				double intensity2 = clamp(255 * (1.0 - dist2 / 85.0), 0.0, 255.0);
				BYTE col = (BYTE)intensity2;
				pixelColors[i] = RGB(col,col,col);
			}
			break;
		}

	case TYPE_1STMINUS2ND_CLAMPED:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx = x - points[refpts[i]].x;
				double dy = y - points[refpts[i]].y;
				double dx2 = x - points[refpts2[i]].x;
				double dy2 = y - points[refpts2[i]].y;
				double dist = sqrt(dx*dx + dy*dy);
				double dist2 = sqrt(dx2*dx2 + dy2*dy2);
				double intensity = clamp(255 * (1.0 - dist / 85.0), 0.0, 255.0);
				double intensity2 = clamp(255 * (1.0 - dist2 / 85.0), 0.0, 255.0);
				BYTE col = (BYTE)clamp(intensity - intensity2, 0.0, 255.0);
				pixelColors[i] = RGB(col,col,col);
			}
			break;
		}

	case TYPE_1STMINUS2ND_UNCLAMPED:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx = x - points[refpts[i]].x;
				double dy = y - points[refpts[i]].y;
				double dx2 = x - points[refpts2[i]].x;
				double dy2 = y - points[refpts2[i]].y;
				double dist = sqrt(dx*dx + dy*dy);
				double dist2 = sqrt(dx2*dx2 + dy2*dy2);
				double intensity = 255 * (1.0 - dist / 85.0);
				double intensity2 = 255 * (1.0 - dist2 / 85.0);
				BYTE col = (BYTE)clamp(intensity - intensity2, 0.0, 255.0);
				pixelColors[i] = RGB(col,col,col);
			}
			break;
		}

	case TYPE_2NDMINUS1ST_CLAMPED:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx = x - points[refpts[i]].x;
				double dy = y - points[refpts[i]].y;
				double dx2 = x - points[refpts2[i]].x;
				double dy2 = y - points[refpts2[i]].y;
				double dist = sqrt(dx*dx + dy*dy);
				double dist2 = sqrt(dx2*dx2 + dy2*dy2);
				double intensity = clamp(255 * (1.0 - dist / 85.0), 0.0, 255.0);
				double intensity2 = clamp(255 * (1.0 - dist2 / 85.0), 0.0, 255.0);
				BYTE col = (BYTE)(intensity2 - intensity);
				pixelColors[i] = RGB(col,col,col);
				if (col > 0)
					int f = 0;
			}
			break;
		}

	case TYPE_2NDMINUS1ST_UNCLAMPED:
		{
			for (int i = 0; i < width * height; i++)
			{
				int x = i % width;
				int y = i / width;
				double dx = x - points[refpts[i]].x;
				double dy = y - points[refpts[i]].y;
				double dx2 = x - points[refpts2[i]].x;
				double dy2 = y - points[refpts2[i]].y;
				double dist = sqrt(dx*dx + dy*dy);
				double dist2 = sqrt(dx2*dx2 + dy2*dy2);
				double intensity = 255 * (1.0 - dist / 85.0);
				double intensity2 = 255 * (1.0 - dist2 / 85.0);
				BYTE col = (BYTE)(intensity2 - intensity);
				pixelColors[i] = RGB(col,col,col);
			}
			break;
		}

	default: break;
	}
}

// Finds and returns the index of the point that is
int NoiseGenerator::findRefPoint(int ptInd, int dist)
{
	int iter = 0;
	int foundInd = -1;
	int x = ptInd % width;		//pixel's x coord
	int y = ptInd / width;		//pixle's y coord

	double farthestDist = max(width, height);

	for (int i = 0; i < numPoints; i++)
	{
		double dx = x - points[i].x;
		double dy = y - points[i].y;
		double result = sqrt(dx*dx + dy*dy);

		if (dist == 0)		//find closest point
		{
			if (result < farthestDist)
			{
				farthestDist = result;
				foundInd = i;
			}
		}
		else               //find 2nd closest point
		{
			int closestX = points[refpts[ptInd]].x;
			int closestY = points[refpts[ptInd]].y;
			if (result < farthestDist && closestX != points[i].x && closestY != points[i].y)
			{
				farthestDist = result;
				foundInd = i;
			}
		}
	}
	return foundInd;
}

// public access function to generate noise
void NoiseGenerator::generateNoise(int type)
{
	calculateColors(type);
}

// clamps a value between minVal and maxVal
double NoiseGenerator::clamp(double val, double minVal, double maxVal)
{
	return max(minVal, min(maxVal, val));
}