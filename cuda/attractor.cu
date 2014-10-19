
#include <iostream>

#include <cuda.h>

#include "attractor.cu.h"

__global__ void zero_array(int* array, int array_size) {
	int abs_index = threadIdx.x + blockIdx.x * blockDim.x;
	if (abs_index < array_size)
		array[abs_index] = 0;
}

__global__ void cuda_attractor(int* data, int data_size, int total_thread_count) {
	int threadID = threadIdx.x + blockIdx.x * blockDim.x;

	if (threadID < total_thread_count) {
		std::size_t startPos = (threadID + 1) * 139;
		float_type x = static_cast<float_type>((startPos % img_width * 3 % img_width + margin) % img_width * 4) / img_width - 2,
			y = (static_cast<int>((static_cast<float_type>(startPos) / img_width + 1) * 7) % img_height * 4) / static_cast<float_type>(img_height) - 2; // wrap y value, spread apart, wrap around
	
		int my_iter_count = max(iter_count / total_thread_count, 1u);
	
		for (int i = 0; i < my_iter_count; ++i) {
			float_type x1 = sin(a * y) - cos(b * x),
				y1 = sin(c * x) - cos(d * y);
	
			int x_coord = (x1 + 2) / 4 * (img_width - margin) + margin / 2,
				y_coord = (y1 + 2) / 4 * (img_height - margin) + margin / 2,
				arr_idx = y_coord * img_width + x_coord;

#ifdef ATTRACTOR_ATOMIC_OPS
			++data[arr_idx];
#else
			atomicAdd(data + arr_idx, 1);
#endif
	
			x = x1;
			y = y1;
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif
	void cuda_zero_array(int array_size, int* array) {
		const int threads_per_block = 512;

		zero_array<<<(array_size + threads_per_block - 1) / threads_per_block, threads_per_block>>>(array, array_size);

		cudaDeviceSynchronize();
		cudaError_t err = cudaGetLastError();

		if (err) {
			std::cerr << "cuda_zero_array failed: " << cudaGetErrorString(err) << std::endl;
			std::exit(-1);
		}
	}

	void attractor(int num_threads, int threads_per_block, int* data, int data_size) {
		int num_blocks;
		if (num_threads == -1)
			num_blocks = (data_size + threads_per_block - 1) / threads_per_block;
		else
			num_blocks = (num_threads + threads_per_block - 1) / threads_per_block;

		cuda_attractor<<<num_blocks, threads_per_block>>>(data, data_size, num_threads);

		cudaDeviceSynchronize();
		cudaError_t err = cudaGetLastError();

		if (err) {
			std::cerr << "attractor failed: " << cudaGetErrorString(err) << std::endl;
			std::exit(-1);
		}
	}

	int cuda_malloc(void** dst, size_t count) {
		cudaError_t err = cudaMalloc(dst, count);
		
		if (err) {
			std::cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << std::endl;
			std::exit(-1);
		}

		return err;
	}

	void cuda_free(void* ptr) {
		cudaFree(ptr);
	}

	int cuda_memcpy(void* to, void* from, size_t size, dir_t direction) {
		cudaError_t err = cudaMemcpy(to, from, size, direction == deviceToHost ? cudaMemcpyDeviceToHost : cudaMemcpyHostToDevice);

		if (err) {
			std::cerr << "cudaMemcpy failed: " << cudaGetErrorString(err) << std::endl;
			std::exit(-1);
		}

		return err;
	}
#ifdef __cplusplus
}
#endif
