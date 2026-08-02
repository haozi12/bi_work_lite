#include "bi_work.h"

//using format string to read data from buffer

int bfreadf(const void* raw_buffer, size_t buffer_size, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int count = vbfreadf(raw_buffer, buffer_size, format, args);
	va_end(args);
	return count;
}