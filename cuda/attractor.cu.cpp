
#include <array>
#include <cmath>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <iostream>

#include "attractor.cu.h"

#include "png++/png.hpp"

std::uint8_t quadratic_pct(int num, int denom, std::uint8_t start, std::uint8_t end) {
	float_type percent = static_cast<float_type>(num) / static_cast<float_type>(denom);
	percent *= percent;
	return static_cast<std::uint8_t>(start + static_cast<std::int16_t>((end - start) * percent));
}

std::uint8_t cubic_pct(int num, int denom, std::uint8_t start, std::uint8_t end) {
	float_type percent = static_cast<float_type>(num) / static_cast<float_type>(denom);
	percent *= percent * percent;
	return static_cast<std::uint8_t>(start + static_cast<std::int16_t>((end - start) * percent));
}

std::uint8_t quartic_pct(int num, int denom, std::uint8_t start, std::uint8_t end) {
	float_type percent = static_cast<float_type>(num) / static_cast<float_type>(denom);
	percent *= percent * percent * percent;
	return static_cast<std::uint8_t>(start + static_cast<std::int16_t>((end - start) * percent));
}

std::uint8_t sqrt_pct(int num, int denom, std::uint8_t start, std::uint8_t end) {
	float_type percent = static_cast<float_type>(num) / static_cast<float_type>(denom);
	percent = std::sqrt(percent);
	return static_cast<std::uint8_t>(start + static_cast<std::int16_t>((end - start) * percent));
}

std::uint8_t linear_pct(int num, int denom, std::uint8_t start, std::uint8_t end) {
	float_type percent = static_cast<float_type>(num) / static_cast<float_type>(denom);
	return static_cast<std::uint8_t>(start + static_cast<std::int16_t>((end - start) * percent));
}

png::rgb_pixel hits_to_pix(int hits) {
	if (hits < iter_cap / 2)
		//return { quartic_pct(hits, iter_cap / 2, 0, 0xFF), 0, sqrt_pct(hits, iter_cap / 2, 0, 0xFF) }; // purple from black
		return { linear_pct(hits, iter_cap / 2, 0xFF, 0), linear_pct(hits, iter_cap / 2, 0xFF, 0), linear_pct(hits, iter_cap / 2, 0xFF, 0) }; // to white from black

	//return { 0xFF, quartic_pct(hits - iter_cap / 2, iter_cap / 2, 0, 0xFF), 0xFF }; // white from purple
	//return { linear_pct(hits - iter_cap / 2, iter_cap / 2, 0, 0xFF), linear_pct(hits - iter_cap / 2, iter_cap / 2, 0, 0xFF), linear_pct(hits - iter_cap / 2, iter_cap / 2, 0, 0xFF) }; // back to white from black
	return { };
}

int main() {
	int* gpu_data,
		count = img_height * img_width,
		size = sizeof(*gpu_data) * count;

	cuda_malloc((void**)&gpu_data, size);

	cuda_zero_array(count, gpu_data);

	attractor(32, 32, gpu_data, count);

	std::vector<std::array<int, img_width>> cpu_data(img_height);

	cuda_memcpy(cpu_data.data(), gpu_data, size, deviceToHost);

	png::image<png::rgb_pixel> image(img_width, img_height);

	for (std::uint32_t i = 0; i < img_height; ++i)
		for (std::uint32_t j = 0; j < img_width; ++j)
			image.set_pixel(j, img_height - i - 1, hits_to_pix(cpu_data[i][j])); // img_height - i - 1 because png++ uses a different coordinate system than us

	image.write("attractor.png");
}
