#pragma once

#include "SFML/System.hpp"
#include "SFML/Network.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "SFML/Window.hpp"
#include "Road.h"
#include "PerlinNoise.h"
#include <iostream>
#include <map>
using std::vector;

#define WINDOW_WIDTH 2500
#define WINDOW_HEIGHT 1300

//define position and width of the road segment
struct RoadAttributes
{
	RoadAttributes(sf::Vector2f s = sf::Vector2f(0.0f, 0.0f), sf::Vector2f e = sf::Vector2f(0.0f, 0.0f), float w = 5.f) noexcept(false) :
		start(s), end(e), width(w) {};

	//basic road info without needing to store a road segment
	sf::Vector2f start;
	sf::Vector2f end;
	float width;
};

//all attributes and extra variables to run the Parish-Muller algorithm
struct RoadQuery
{
	RoadQuery(int x, RoadAttributes initRA) noexcept :
		t(x), ra(initRA), success(true) {};

	int t;
	RoadAttributes ra;
	bool success;
};

//an actual road in the city
struct RoadSegment
{
	RoadSegment(sf::Vector2f s, sf::Vector2f e, float w) :
		road(s, e, w) 
	{
		ra.start = s;
		ra.end = e;
		ra.width = w;
	};
	RoadSegment(RoadAttributes nra) :
		road(nra.start, nra.end, nra.width) 
	{
		ra = nra;
	};

	RoadAttributes ra;
	Road road;
};

//biome types
const enum class BiomeType : unsigned char
{
	Grass,
	Sand,
	Rock,
	Snow,
	Num
};

//colours of the biome types
static std::map<BiomeType, sf::Color> BiomeColors
{
	{BiomeType::Grass, sf::Color(0x0, 0x78, 0x0)},
	{BiomeType::Sand, sf::Color(0xE7, 0xB7, 0x44)},
	{BiomeType::Rock, sf::Color(0x80, 0x84, 0x87)},
	{BiomeType::Snow, sf::Color(0xC0, 0xF6, 0xFB)}
};

//biome and some metadata to be used for checks and by the quadtree system
struct Biome
{
	BiomeType type = BiomeType::Grass;
	sf::Vector2i centre;
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;
	vector<sf::Vector2i> pixels;
	sf::Vector2i edgesVert;
	sf::Vector2i edgesHori;
};

//generates cities, re call Init to make more
class CityGen
{
public:
	CityGen() noexcept
	{
		//initialise both vectors to be empty
		R.clear();
		L.clear();

		perlin = nullptr;
	};

	//initialise list based on city centre point
	void Init(sf::Vector2f centre);

	//generate a city from initial list state
	void Generate(int roadLimit);

	//do everything without specification
	void AutomatedGenerate();

	//set up the background and fill pixel vectors here
	void GenerateBackground();

	//draw the cities
	void Draw(sf::RenderWindow& window);

	//tweak position based on obstacles
	void LocalConstraints(RoadQuery& rq, RoadSegment segment);

	//suggest road positions based on various data
	vector<RoadQuery> GlobalGoals(RoadAttributes ra);

	//get biome based on pixel position
	unsigned char GetBiome(int x, int y);

	//tweaks the angle of road segments
	void RandomiseRoadAngle(RoadAttributes& ra);

	//used by GlobalGoals to minimise slope and deviation
	void FindIdealAngle(RoadAttributes& ra);

	//the logic of FindIdealAngle
	float GetCost(sf::Vector2f start, sf::Vector2f end, float offset);

	//checks if a biome is big enough to be split into more biomes
	bool CheckDivisible(Biome biome);

	//divides a large biome into smaller ones
	void DivideRegion(unsigned int index, float threshold);

protected:
	//useful constants
	static constexpr float BASE_ROAD_LENGTH = 25.0f;
	static constexpr float BASE_ROAD_WIDTH = 3.0f;

	//background image, shows heightmap and biomes
	sf::Texture bgtex;
	sf::Sprite background;
	PerlinNoise* perlin;

	//a vector of suggested road segments, actual road segments and biomes
	vector<RoadQuery> L;
	vector<RoadSegment> R;
	vector<Biome> biomes;
};
