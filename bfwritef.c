#include "bi_work.h"

//using format string to write data to buffer

int bfwritef(void* dest, size_t buffer_size, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int count = vbfwritef(dest, buffer_size, format, args);
	va_end(args);
	return count;
}