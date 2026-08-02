#ifndef BINARY_WORK_CPP_LOADED
#define BINARY_WORK_CPP_LOADED

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

namespace bi_work{
	enum ErrorCode {
		NULL_BUFFER_OR_FORMAT = -1,
		NULL_FILE = -2,
		MALLOC_FAILURE = -3
	};

	int swap_endianf(const char* format, ...);
	int vswap_endianf(const char* format, va_list args);

	int vbfwritef(void* dest, size_t buffer_size, const char* format, va_list args);
	int vbfreadf(const void* raw_buffer, size_t buffer_size, const char* format, va_list args);

	int bfreadf(const void* raw_buffer, size_t buffer_size, const char* format, ...);
	int bfwritef(void* dest, size_t buffer_size, const char* format, ...);

	int vfreadf(FILE* _Stream, size_t read_length, const char* format, va_list args);
	int vfwritef(FILE* _Stream, size_t write_length, const char* format, va_list args);

	int fwritef(FILE* _Stream, size_t write_length, const char* format, ...);
	int freadf(FILE* _Stream, size_t read_length, const char* format, ...);

}
#endif