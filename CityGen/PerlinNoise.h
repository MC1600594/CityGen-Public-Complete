#include <vector>

class PerlinNoise {
	//permutation vector
	std::vector<int> perm;
public:

	//Init with set seed
	PerlinNoise() noexcept(false);
	//Init with given seed
	PerlinNoise(unsigned int seed);

	//The Noise Function
	float noise(float x, float y, float z);
private:
	float fade(float t) noexcept;
	float linearInterpolation(float t, float a, float b) noexcept;
	float gradient(int hash, float x, float y, float z) noexcept;
};
