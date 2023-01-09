#pragma once
#include <glm/vec3.hpp>
#include <glm/common.hpp>

namespace TL
{
	inline std::vector<int> range(int begin_incl, int end_excl)
	{
		std::vector<int> arr(end_excl - begin_incl);
		for (int& i : arr) i = begin_incl++;
		return arr;
	}

	inline std::vector<int> range(int n)
	{
		return range(0, n);
	}
	inline glm::vec3 rgb_to_hsv(glm::vec3 col)
	{
		float hue, saturation;
		float value = glm::max(glm::max(col.r, col.g), col.b);
		float lowest = glm::min(glm::min(col.r, col.g), col.b);
		if (value == 0)
			saturation = 0;
		else
			saturation = (value - lowest) / value;

		if (value == col.r)
			hue = 60 * ((col.g - col.b) / (value - lowest));
		else if (value == col.g)
			hue = 60 * (2 + (col.b - col.r) / (value - lowest));
		else
			hue = 60 * (4 + (col.r - col.g) / (value - lowest));

		if (hue < 60)
			hue += 360;

		return glm::vec3(hue, saturation, value);
	}

	inline glm::vec3 hsv_to_rgb(glm::vec3 col)
	{
		float chroma = col.z * col.y;

		//              = hue / 360d in [0,1] space
		float hue_prime = col.x / 0.16667;
		float x = chroma * (1.0 - abs(fmodf(hue_prime, 2.0) - 1.0));
		glm::vec3 ret;
		if (hue_prime < 3.0)
		{
			if (hue_prime < 2.0)
			{
				if (hue_prime < 1.0)
					ret = glm::vec3(chroma, x, 0.0);
				else
					ret = glm::vec3(x, chroma, 0.0);
			}
			else
				ret = glm::vec3(0.0, chroma, x);
		}
		else
		{
			if (hue_prime < 5.0)
			{
				if (hue_prime < 4.0)
					ret = glm::vec3(0.0, x, chroma);
				else
					ret = glm::vec3(x, 0.0, chroma);
			}

			else
				ret = glm::vec3(chroma, 0.0, x);
		}

		return ret + (col.z - chroma);
	}

	inline glm::vec3 gamma(glm::vec3 color, float g)
	{
		return glm::pow(color, glm::vec3(1.0 / g)); 
	}
}