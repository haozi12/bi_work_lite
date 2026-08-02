#include "bi_work_cpp.h"
namespace bi_work{

	int vfreadf(FILE* _Stream, size_t read_length, const char* format, va_list args) {
		if (_Stream == NULL) {
			return NULL_FILE;
		}
		unsigned char* buffer = (unsigned char*)calloc(read_length, sizeof(unsigned char));
		if (buffer == NULL) {
			perror("malloc failed");
			return MALLOC_FAILURE;
		}
		size_t bytes = fread(buffer, sizeof(unsigned char), read_length, _Stream);
		if (bytes < read_length) {
			fprintf(stderr, "Warning: File does not have enough bytes, data might be incomplete.\n");
		}
		int count = vbfreadf(buffer, bytes, format, args);
		free(buffer);
		return count;
	}
	
}