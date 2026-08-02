#include "bi_work.h"

//using format string to read data from file

int freadf(FILE* _Stream,size_t read_length,const char* format,...) {
	va_list args;
	va_start(args, format);
	int count = vfreadf(_Stream, read_length, format, args);
	va_end(args);
	return count;
}