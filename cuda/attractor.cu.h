
#include <stdint.h>

const int img_height = 1000,
    img_width = img_height,
    margin = 20 * 2; // 20 for each edge

const uint32_t iter_count = 5000000;//30000000;

typedef float float_type;

const float_type a = 1.4, b = -2.3, c = 2.4, d = -2.1;

const int iter_cap = 128;//1024;

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum {
		deviceToHost,
		hostToDevice
	} dir_t;

	void cuda_zero_array(int array_size, int* array);

	void attractor(int num_threads, int threads_per_block, int* data, int data_size);

	int cuda_malloc(void**, size_t);

	void cuda_free(void*);

	int cuda_memcpy(void* to, void* from, size_t size, dir_t direction);
#ifdef __cplusplus
}
#endif
