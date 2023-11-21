#pragma once
#include "Road.h"

#include <vector>
#include <array>
#include <math.h>
#include <stdlib.h>
#include <ctime>
using std::vector;
using std::array;

#include "SFML/System/Vector2.hpp"

//use seperating axis theorem for collisions
bool CheckRectangleCollision(vector<sf::Vertex> AV, vector<sf::Vertex> BV)
{
	//working from: https://www.gamedev.net/tutorials/_/technical/game-programming/2d-rotated-rectangle-collision-r2604/
	
	//create axes perpendicular to sides
	vector<sf::Vector2f> axes;
	axes.resize(4);

	axes.at(0).x = AV.at(0).position.x - AV.at(1).position.x;
	axes.at(0).y = AV.at(0).position.y - AV.at(1).position.y;

	axes.at(1).x = AV.at(1).position.x - AV.at(2).position.x;
	axes.at(1).y = AV.at(1).position.y - AV.at(2).position.y;

	axes.at(2).x = BV.at(0).position.x - BV.at(1).position.x;
	axes.at(2).y = BV.at(0).position.y - BV.at(1).position.y;

	axes.at(3).x = BV.at(1).position.x - BV.at(2).position.x;
	axes.at(3).y = BV.at(1).position.y - BV.at(2).position.y;

	//project vertices onto axes
	for (int i = 0; i < 4; i++)
	{
		//a little messy, but works
		array<sf::Vector2f, 4> Aproj = {};
		array<sf::Vector2f, 4> Bproj = {};

		array<float, 4> Avals = {};
		array<float, 4> Bvals = {};

		sf::Vector2f Aminmax;
		sf::Vector2f Bminmax;

		//calculate projections
		{
			float temp = 0.f;

			for (int k = 0; k < 4; k++)
			{
				temp = static_cast<float>((axes.at(i).x * AV.at(k).position.x) + (axes.at(i).y * AV.at(k).position.y));
				temp /= static_cast<float>((axes.at(i).x * axes.at(i).x) + (axes.at(i).y * axes.at(i).y));
				Aproj.at(k).x = axes.at(i).x * temp;
				Aproj.at(k).y = axes.at(i).y * temp;

				temp = static_cast<float>((axes.at(i).x * BV.at(k).position.x) + (axes.at(i).y * BV.at(k).position.y));
				temp /= static_cast<float>((axes.at(i).x * axes.at(i).x) + (axes.at(i).y * axes.at(i).y));
				Bproj.at(k).x = axes.at(i).x * temp;
				Bproj.at(k).y = axes.at(i).y * temp;
			}
		}

		//calculate scalar values
		for (int j = 0; j < 4; j++)
		{
			Avals.at(j) = (axes.at(i).x * Aproj.at(j).x) + (axes.at(i).y * Aproj.at(j).y);
			Bvals.at(j) = (axes.at(i).x * Bproj.at(j).x) + (axes.at(i).y * Bproj.at(j).y);
		}

		//calculate minmax values
		Aminmax = sf::Vector2f(Avals.at(0), Avals.at(0));
		Bminmax = sf::Vector2f(Bvals.at(0), Bvals.at(0));

		for (int k = 1; k < 4; k++)
		{
			if (Avals.at(k) < Aminmax.x)
			{
				Aminmax.x = Avals.at(k);
			}
			if (Avals.at(k) > Aminmax.y)
			{
				Aminmax.y = Avals.at(k);
			}

			if (Bvals.at(k) < Bminmax.x)
			{
				Bminmax.x = Bvals.at(k);
			}
			if (Bvals.at(k) > Bminmax.y)
			{
				Bminmax.y = Bvals.at(k);
			}
		}

		//check for no intersection on this axis
		if (Bminmax.x >= Aminmax.y || Bminmax.y <= Aminmax.x)
		{
			//bail without checking other axes
			return false;
		}
	}
	return true;
}

//use seperating axis theorem for collisions
bool CheckRoadCollision(Road A, Road B)
{
	//create vertex vectors
	vector<sf::Vertex> AV = A.getVertices();
	vector<sf::Vertex> BV = B.getVertices();

	return CheckRectangleCollision(AV, BV);
}

//Pythagoras
float Dist(sf::Vector2f A, sf::Vector2f B)
{
	const sf::Vector2f vec = A - B;
	const float dist = sqrt((vec.x * vec.x) + (vec.y * vec.y));

	return dist;
}

//manhattan distance, for ints
int Dist(sf::Vector2i A, sf::Vector2i B)
{
	const sf::Vector2i vec = A - B;

	return abs(vec.x) + abs(vec.y);
}

//check if two segments are identical
bool CheckDuplicateRoad(RoadAttributes A, RoadAttributes B, float threshold)
{
	//check for identical points
	if (Dist(A.start, B.start) < threshold)
	{
		if (Dist(A.end, B.end) < threshold)
		{
			return true;
		}
		return false;
	}
	//check for the reverse direction
	if (Dist(A.start, B.end) < threshold)
	{
		if (Dist(A.end, B.start) < threshold)
		{
			return true;
		}
		return false;
	}
	return false;
}

//debug aid, counts duplicate roads.
void DebugCountDuplicatedRoads(vector<RoadSegment> R)
{
	int n = 0;
	for (unsigned int i = 0; i < R.size(); ++i)
	{
		for (unsigned int k = i + 1; k < R.size(); ++k)
		{
			if (CheckDuplicateRoad(R.at(i).ra, R.at(k).ra, R.at(i).ra.width) == true)
			{
				++n;
			}
		}
	}
	std::cout << "total duplicated roads: " << n << std::endl;
}

//checks if the smallest gap between points is less than some threshold value
bool CheckShortestDistance(vector<sf::Vector2i> points, float threshold)
{
	int min = 10 * WINDOW_WIDTH;

	for (unsigned int i = 0; i < points.size() - 1; ++i)
	{
		for (unsigned int j = i + 1; j < points.size(); ++j)
		{
			const int temp = Dist(points.at(j), points.at(i));

			if (min > temp)
			{
				min = temp;
			}
		}
	}

	if (min < threshold)
	{
		return false;
	}
	else
	{
		return true;
	}
}