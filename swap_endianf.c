#include "bi_work.h"

int swap_endianf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	int count = vswap_endianf(format, args);
	va_end(args);
	return count;
}