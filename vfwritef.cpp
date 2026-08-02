#include "bi_work_cpp.h"

namespace bi_work{

	int vfwritef(FILE* _Stream,size_t write_length,const char* format,va_list args) {
		if (_Stream == NULL) {
			return NULL_FILE;
		}
		unsigned char* buffer = (unsigned char *)calloc(write_length, sizeof(unsigned char));
		if (buffer == NULL) {
			perror("malloc failed");
			return MALLOC_FAILURE;
		}
		int count = vbfwritef(buffer, write_length, format, args);
		size_t bytes = fwrite(buffer, sizeof(unsigned char), write_length, _Stream);
		if (bytes < write_length) {
			fprintf(stderr, "Warning: bytes written: %zu, but expected: %zu\n", bytes, write_length);
		}
		free(buffer);
		return count;
	}

}