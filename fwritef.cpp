#include "bi_work_cpp.h"

namespace bi_work{
//using format string to write data to file

	int fwritef(FILE* _Stream, size_t write_length, const char* format, ...) {
		va_list args;
		va_start(args, format);
		int count = vfwritef(_Stream,write_length,format,args);
		va_end(args);
		return count;
	}

}