#include "CityGen.h"
#include "Helpers.h"

//Initialises L to a four way intersection.
void CityGen::Init(sf::Vector2f centre)
{
	//set up initial cross shape
	vector<RoadAttributes> initRA;
	initRA.push_back(*std::make_unique<RoadAttributes>(centre, centre + sf::Vector2f(0.0f, BASE_ROAD_LENGTH), BASE_ROAD_WIDTH));
	initRA.push_back(*std::make_unique<RoadAttributes>(centre, centre + sf::Vector2f(0.0f, -BASE_ROAD_LENGTH), BASE_ROAD_WIDTH));
	initRA.push_back(*std::make_unique<RoadAttributes>(centre, centre + sf::Vector2f(BASE_ROAD_LENGTH, 0.0f), BASE_ROAD_WIDTH));
	initRA.push_back(*std::make_unique<RoadAttributes>(centre, centre + sf::Vector2f(-BASE_ROAD_LENGTH, 0.0f), BASE_ROAD_WIDTH));

	//create the vector of RoadQueries
	vector<RoadQuery> init;
	init.push_back(RoadQuery(0, initRA.at(0)));
	init.push_back(RoadQuery(0, initRA.at(1)));
	init.push_back(RoadQuery(0, initRA.at(2)));
	init.push_back(RoadQuery(0, initRA.at(3)));

	//initialize list of RoadQuery L to an initial set of possible roads
	L.clear();
	L = init;
}

void CityGen::Generate(int roadLimit)
{
	//how many road segments have been generated
	int roadCurrent = 0;
	//how many calls to LocalConstraints there have been
	int roadAttempts = 0;

	//generate until no proposed roads are left
	while ((L.size() != 0) && (roadCurrent < roadLimit))
	{
		//loop over all possible new roads
		for (unsigned int i = 0; i < L.size(); ++i)
		{
			//decrement x until it is 0
			if (L.at(i).t > 0)
			{
				L.at(i).t--;
			}
			else if (roadCurrent < roadLimit)
			{
				//DO NOT move inside LocalConstraints, it breaks for an unknown reason
				for (unsigned int k = 0; k < R.size(); ++k)
				{
					//localConstraints are things like other roads, buildings etc
					LocalConstraints(L.at(i), R.at(k));
					++roadAttempts;

					//double check for duplicates, the near miss checks may have created one
					if (CheckDuplicateRoad(L.at(i).ra, R.at(k).ra, BASE_ROAD_WIDTH) == true)
					{
						L.at(i).success = false;
					}
				}

				//if road placement is valid, create three proposed roads and replace the current proposed road with an actual road
				if (L.at(i).success == 1)
				{
					//globalGoals tries to base road positions on various kinds of metadata
					vector<RoadQuery> S = GlobalGoals(L.at(i).ra);
					R.push_back(RoadSegment(L.at(i).ra));
					++roadCurrent;

					if (S.at(1).t >= 0) { L.push_back(S.at(1)); }
					if (S.at(2).t >= 0) { L.push_back(S.at(2)); }
					if (S.at(0).t >= 0) { L.push_back(S.at(0)); }

					//mark RoadQuery for deletion
					L.at(i).success = false;
				}
			}
		}
		//remove all failed road segments
		bool done = false;
		while (!done)
		{
			//set done to true unless a Query is deleted
			unsigned int k = 0;
			done = true;

			while (k < L.size())
			{
				//if no valid road can be created delete proposed road
				if (L.at(k).success == 0)
				{
					L.erase(L.begin() + k);
					k = L.size();
					done = false;
				}
				++k;
			}
		}
	}
	L.clear();
}

void CityGen::AutomatedGenerate()
{
	biomes.clear();

	//calculate limits
	constexpr int rangeX = WINDOW_WIDTH - 300;
	constexpr int rangeY = WINDOW_HEIGHT - 300;

	//setup loop
	bool valid = false;
	vector<sf::Vector2i> centres;
	unsigned int attempts = 0;

	//loop until valid setup is found
	while (!valid)
	{
		std::cout << "Biome Setup Attempt: " << attempts << std::endl;

		centres.clear();

		//between 4 and 8 cities
		const unsigned int citynum = 4 +(rand() % 5);
		
		for (unsigned int i = 0; i < citynum; ++i)
		{
			centres.push_back(sf::Vector2i(rand() % rangeX + 150, rand() % rangeY + 150));
		}

		//check biome centres arent too close
		valid = CheckShortestDistance(centres, 30.f * BASE_ROAD_LENGTH);

		if (attempts >= 100)
		{
			//valid = true;
		}

		++attempts;
	}

	//setup biomes vector and set centres
	for (unsigned char i = 0; i < centres.size(); ++i)
	{
		Biome temp;
		temp.centre = centres.at(i);
		
		const unsigned char type = rand() % static_cast<unsigned char>(BiomeType::Num);
		temp.type = static_cast<BiomeType>(type);

		biomes.push_back(temp);
	}

	//set first 4 biomes to distinct types
	for (unsigned char i = 0; i < static_cast<unsigned char>(BiomeType::Num); ++i)
	{
		if (biomes.size() > i)
		{
			biomes.at(i).type = static_cast<BiomeType>(i);
		}
	}

	//generate background and set pixel vectors
	GenerateBackground();

	//set width, height and extreme values for each biome
	for (unsigned char i = 0; i < biomes.size(); ++i)
	{
		sf::Vector2i minvals = sf::Vector2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		sf::Vector2i maxvals = sf::Vector2i(0, 0);

		for (unsigned int j = 0; j < biomes.at(i).pixels.size(); ++j)
		{
			if (biomes.at(i).pixels.at(j).x < minvals.x) { minvals.x = biomes.at(i).pixels.at(j).x; }
			if (biomes.at(i).pixels.at(j).x > maxvals.x) { maxvals.x = biomes.at(i).pixels.at(j).x; }

			if (biomes.at(i).pixels.at(j).y < minvals.y) { minvals.y = biomes.at(i).pixels.at(j).y; }
			if (biomes.at(i).pixels.at(j).y > maxvals.y) { maxvals.y = biomes.at(i).pixels.at(j).y; }
		}

		biomes.at(i).width = maxvals.x - minvals.x;
		biomes.at(i).height = maxvals.y - minvals.y;

		biomes.at(i).edgesVert = sf::Vector2i(minvals.y, maxvals.y);
		biomes.at(i).edgesHori = sf::Vector2i(minvals.x, maxvals.x);
	}

	//check if any biomes are large enough to be divided
	for (unsigned char i = 0; i < biomes.size(); ++i)
	{
		if (CheckDivisible(biomes.at(i)))
		{
			//DivideRegion(i, 300.f);
		}
	}

	//generate highways
	for (unsigned int i = 0; i < biomes.size() - 1; ++i)
	{
		for (unsigned int j = 1; j < biomes.size(); ++j)
		{
			if (Dist(sf::Vector2f(biomes.at(i).centre), sf::Vector2f(biomes.at(j).centre)) < 1000.f)
			{
				valid = true;
				RoadSegment temp = RoadSegment(sf::Vector2f(biomes.at(i).centre), sf::Vector2f(biomes.at(j).centre), 1.5f);

				if (valid == true)
				{
					//R.push_back(temp);
				}
			}
		}
	}

	//generate roads
	for (unsigned char i = 0; i < biomes.size(); ++i)
	{
		Init(sf::Vector2f(biomes.at(i).centre));
		
		const unsigned int size = static_cast<unsigned int>(static_cast<float>(biomes.at(i).width + biomes.at(i).height) / 4.f);
		Generate(size);
	}
}

void CityGen::GenerateBackground()
{
	//setup noise and image
	perlin = new PerlinNoise(rand());
	sf::Image bg;

	bg.create(WINDOW_WIDTH, WINDOW_HEIGHT);

	//use the perlin heightmap and biomes to shade and colour
	for (int j = 0; j < WINDOW_WIDTH; ++j)
	{
		for (int i = 0; i < WINDOW_HEIGHT; ++i)
		{
			//calculate x and y points on the perlin map
			const float xMap = static_cast<float>(i * 5.f + 0.5f) / static_cast<float>(WINDOW_WIDTH);
			const float yMap = static_cast<float>(j * 5.f + 0.5f) / static_cast<float>(WINDOW_HEIGHT);

			//calculate height of point on map
			const float height = perlin->noise(xMap, yMap, 0.0f);

			//tweak greyscale value for aesthetics
			const char greyscale = static_cast<char>((height / 1.5f + 0.33f) * 255.f);

			const unsigned char biome = GetBiome(j, i);
			biomes.at(biome).pixels.push_back(sf::Vector2i(j, i));

			//show heightmap on background
			const sf::Color color = sf::Color(greyscale, greyscale, greyscale) * BiomeColors.at(biomes.at(biome).type);
			bg.setPixel(j, i, color);
		}
	}
	bgtex.loadFromImage(bg);
	background.setTexture(bgtex);
}

void CityGen::Draw(sf::RenderWindow& window)
{
	//draw the background
	window.draw(background);

	//simply draw every road
	for (unsigned int i = 0; i < R.size(); ++i)
	{
		window.draw(R.at(i).road);
	}
}

void CityGen::LocalConstraints(RoadQuery& rq, RoadSegment segment)
{
	Road road(rq.ra.start, rq.ra.end, rq.ra.width);

	//check if a collision is possible
	if ((Dist(road.position, segment.road.position) <= BASE_ROAD_LENGTH))
	{
		//remove duplicate segments
		if (CheckDuplicateRoad(rq.ra, segment.ra, BASE_ROAD_WIDTH) == true)
		{
			rq.success = false;
			return;
		}

		//threshold for snapping to an existing segment
		constexpr float threshold = BASE_ROAD_LENGTH / 10.0f;
		
		//check for near misses, then make a junction if true
		if (Dist(rq.ra.end, segment.ra.start) <= threshold)
		{
			rq.ra.end = segment.ra.start;
			return;
		}
		if (Dist(rq.ra.end, segment.ra.end) <= threshold)
		{
			rq.ra.end = segment.ra.end;
			return;
		}
		if (Dist(rq.ra.end, segment.road.position) <= threshold)
		{
			rq.ra.end = segment.road.position;
			return;
		}
			
		//check for the parent segment, these will collide by design
		if (rq.ra.start == segment.ra.end)
		{
			return;
		}
		//check for sibling segments, these will collide by design
		if (rq.ra.start == segment.ra.start)
		{
			return;
		}
			
		//check for collisions using seperating axis theorem
		if (CheckRoadCollision(road, segment.road))
		{
			//if a collision is found, set success to false and cease calculation
			rq.success = false;
			return;
		}
	}
}

vector<RoadQuery> CityGen::GlobalGoals(RoadAttributes ra)
{
	//init output
	vector<RoadQuery> t;

	//create forward and side vectors
	const sf::Vector2f direction = ra.end - ra.start;
	const sf::Vector2f unitDirection = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
	const sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

	RoadAttributes nra = ra;

	//suggest an intersection
	nra.start = nra.end;
	nra.end = ra.end + (BASE_ROAD_LENGTH * unitDirection);

	//some of the time tweak the angle of the forward segment
	if (rand() % 100 > 60)
	{
		//RandomiseRoadAngle(nra);
		FindIdealAngle(nra);
	}

	t.push_back(RoadQuery(1, nra));

	nra.end = nra.start + (BASE_ROAD_LENGTH * unitPerpendicular);

	//tweak the side road more often
	if (rand() % 100 > 40)
	{
		//RandomiseRoadAngle(nra);
		FindIdealAngle(nra);
	}

	t.push_back(RoadQuery(1, nra));

	nra.end = nra.start - (BASE_ROAD_LENGTH * unitPerpendicular);

	//tweak the other side road
	if (rand() % 100 > 40)
	{
		//RandomiseRoadAngle(nra);
		FindIdealAngle(nra);
	}

	t.push_back(RoadQuery(1, nra));

	return t;
}

unsigned char CityGen::GetBiome(int x, int y)
{
	//find what biome is closest to the coordinate and return the element in the biome vector
	unsigned char result = 0;
	int closest = 10 * WINDOW_WIDTH;
	int temp = 0;

	for (unsigned char i = 0; i < biomes.size(); ++i)
	{
		temp = Dist(sf::Vector2i(x, y), biomes.at(i).centre);

		if (temp < closest)
		{
			closest = temp;
			result = i;
		}
	}

	return result;
}

void CityGen::RandomiseRoadAngle(RoadAttributes& ra)
{
	//get angle
	sf::Vector2f direction = ra.end - ra.start;

	float angle = atan2f(direction.y, direction.x); //note: in radians between +-pi. 0 is right.

	//pick random offset
	float offset = -10.f + 5.f * static_cast<float>(rand() % 5);
	//convert to radians
	offset = offset * 3.14159f / 180.f;

	//calculate new direction
	angle = angle + offset;
	direction.y = sin(angle);
	direction.x = cos(angle);

	//generate new coords
	ra.end = ra.start + (BASE_ROAD_LENGTH * direction);
}

void CityGen::FindIdealAngle(RoadAttributes& ra)
{
	//get angle
	sf::Vector2f direction = ra.end - ra.start;

	float angle = atan2f(direction.y, direction.x); //note: in radians between +-pi. 0 is right.

	//pick random offset
	float offset = -20.f;
	vector<float> costs;

	for (unsigned int i = 0; i < 5; ++i)
	{
		offset = -20.f + (10.f * i);

		//convert to radians
		offset = offset * 3.14159f / 180.f;

		//calculate new direction
		angle = angle + offset;
		direction.y = sin(angle);
		direction.x = cos(angle);

		//generate new coords
		ra.end = ra.start + (BASE_ROAD_LENGTH * direction);

		//calculate cost
		costs.push_back(GetCost(ra.start, ra.end, offset * 180.f / 3.14159f));
	}
	
	float mincost = 10000.f;

	//get the smallest cost
	for (unsigned int i = 0; i < costs.size(); ++i)
	{
		if (costs.at(i) < mincost)
		{
			mincost = costs.at(i);
			offset = -20.f + 10.f * i;

			//convert to radians
			offset = offset * 3.14159f / 180.f;
		}
	}

	//calculate new direction
	angle = angle + offset;
	direction.y = sin(angle);
	direction.x = cos(angle);

	//generate new coords
	ra.end = ra.start + (BASE_ROAD_LENGTH * direction);
}

float CityGen::GetCost(sf::Vector2f start, sf::Vector2f end, float offset)
{
	const float slope = perlin->noise(start.x, start.y, 0.0f) - perlin->noise(end.x, end.y, 0.0f);
	
	return abs(slope) + (abs(offset) / 100.0f);
}

bool CityGen::CheckDivisible(Biome b)
{
	//if the biome is very large, return true
	if (b.pixels.size() > 700000)
	{
		if (b.height > 600)
		{
			if (b.width > 800)
			{
				return true;
			}
		}
	}

	return false;
}

void CityGen::DivideRegion(unsigned int index, float threshold)
{
	//take a copy of the region
	Biome region = biomes.at(index);
	vector<Biome> newBiomes;

	//get region dimensions (these are stored in the biome object)


	//try to split the region
	bool check = false;
	char count = 0;

	while (check == false)
	{
		count++;

		Biome temp;
		temp.centre = sf::Vector2i((rand() % region.width - 100) + region.edgesHori.x + 50, (rand() % region.height - 100) + region.edgesVert.x + 50);

		//check centre is in original region
		bool valid = false;
		for (int x = region.edgesHori.x; x <= region.edgesHori.y; ++x)
		{
			for (int y = region.edgesVert.x; y <= region.edgesVert.y; ++y)
			{
				if (x == temp.centre.x && y == temp.centre.y)
				{
					valid = true;
				}
			}
		}

		//check if centre is far enough away from other centres and the edges
		if (valid == true)
		{
			for (unsigned char k = 0; k < newBiomes.size(); ++k)
			{
				if (Dist(temp.centre, newBiomes.at(k).centre) < threshold)
				{
					valid = false;
				}
			}

			for (unsigned char l = 0; l < biomes.size(); ++l)
			{
				if (l != index)
				{
					if (Dist(temp.centre, biomes.at(l).centre) < threshold)
					{
						valid = false;
					}
				}
			}

			if (temp.centre.x < 150 || temp.centre.x > WINDOW_WIDTH - 150)
			{
				valid = false;
			}
			if (temp.centre.y < 150 || temp.centre.y > WINDOW_HEIGHT - 150)
			{
				valid = false;
			}
		}

		if (valid == true)
		{
			newBiomes.push_back(temp);
		}

		if (count == 100 || newBiomes.size() == 4)
		{
			check = true;
		}
	}

	//fill out pixel arrays for new biomes
	for (unsigned int k = 0; k < region.pixels.size(); ++k)
	{
		float minDist = 1000.f;
		char minIndex = 0;

		for (unsigned char i = 0; i < newBiomes.size(); ++i)
		{
			if (Dist(sf::Vector2f(newBiomes.at(i).centre), sf::Vector2f(region.pixels.at(k))) < minDist)
			{
				minDist = Dist(sf::Vector2f(newBiomes.at(i).centre), sf::Vector2f(region.pixels.at(k)));
				minIndex = i;
			}
		}

		newBiomes.at(minIndex).pixels.push_back(region.pixels.at(k));
	}
	
	//fill out other data
	for (unsigned char i = 0; i < newBiomes.size(); ++i)
	{
		newBiomes.at(i).type = region.type;

		sf::Vector2i minvals = sf::Vector2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		sf::Vector2i maxvals = sf::Vector2i(0, 0);

		for (unsigned int j = 0; j < newBiomes.at(i).pixels.size(); ++j)
		{
			if (newBiomes.at(i).pixels.at(j).x < minvals.x) { minvals.x = newBiomes.at(i).pixels.at(j).x; }
			if (newBiomes.at(i).pixels.at(j).x > maxvals.x) { maxvals.x = newBiomes.at(i).pixels.at(j).x; }
							
			if (newBiomes.at(i).pixels.at(j).y < minvals.y) { minvals.y = newBiomes.at(i).pixels.at(j).y; }
			if (newBiomes.at(i).pixels.at(j).y > maxvals.y) { maxvals.y = newBiomes.at(i).pixels.at(j).y; }
		}

		newBiomes.at(i).width = maxvals.x - minvals.x;
		newBiomes.at(i).height = maxvals.y - minvals.y;
		
		newBiomes.at(i).edgesVert = sf::Vector2i(minvals.y, maxvals.y);
		newBiomes.at(i).edgesHori = sf::Vector2i(minvals.x, maxvals.x);
	}

	//remake biome vector
	for (unsigned int i = 0; i < biomes.size(); ++i)
	{
		if (i != index)
		{
			newBiomes.push_back(biomes.at(i));
		}
	}
	biomes = newBiomes;
}