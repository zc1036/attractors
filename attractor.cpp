
//#include <omp.h>
#include <array>
#include <cmath>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

#include "png++/png.hpp"

constexpr int img_height = 1000,
              img_width = img_height,
              margin = 20 * 2; // 20 for each edge

constexpr std::uint32_t iter_count = 30000000;

using float_type = float;

constexpr float_type a = 2.01, b = -2.53, c = 1.61, d = -0.33;

constexpr int iter_cap = 1024;

int omp_get_thread_num() { return 1; }
int omp_get_num_threads() { return 1; }

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
	std::vector<std::array<std::uint16_t, img_width>> data(img_height);

#pragma omp parallel num_threads(4) default(none) shared(data)
	{
		auto num_threads = omp_get_num_threads();
		float_type x = 2 * (static_cast<float_type>(omp_get_thread_num()) / static_cast<float_type>(num_threads)), y = x;

		for (std::uint32_t i = 0; i < iter_count / num_threads; ++i) {
			float_type x1 = std::sin(a * y) - std::cos(b * x);
			float_type y1 = std::sin(c * x) - std::cos(d * y);

			int x_coord = (x1 + 2) / 4 * (img_width - margin) + margin / 2;
			int y_coord = (y1 + 2) / 4 * (img_height - margin) + margin / 2;

			//std::cout << "Doing img[" << x_coord << "][" << y_coord << "] (originally " << ((y1 + 2) / 4) << " and " << ((x1 + 2) / 4) << ")" << std::endl;

			if (data[y_coord][x_coord] < iter_cap) ++data[y_coord][x_coord];

			x = x1;
			y = y1;
		}
	}

	png::image<png::rgb_pixel> image(img_width, img_height);

	for (std::uint32_t i = 0; i < img_height; ++i)
		for (std::uint32_t j = 0; j < img_width; ++j)
			image.set_pixel(j, img_height - i - 1, hits_to_pix(data[i][j])); // img_height - i - 1 because png++ uses a different coordinate system than us

	image.write("attractor.png");
}
