#include "bi_work_cpp.h"

namespace bi_work{
	int vbfreadf(const void* raw_buffer, size_t buffer_size, const char* format, va_list args) {
		if (raw_buffer == NULL || format == NULL) {
			return NULL_BUFFER_OR_FORMAT;
		}
		const unsigned char* forward_ptr = (const unsigned char*)raw_buffer;
		int count = 0;
		const char* ptr = format;
		while (*ptr != '\0' && forward_ptr < (const unsigned char*)raw_buffer + buffer_size) {
			if (*ptr == '%') {
				ptr++;
				if (*ptr == '\0')
					break;
				switch (*ptr) {
				case 'd':
				{
					if (forward_ptr + sizeof(int) > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(va_arg(args, int*), forward_ptr, sizeof(int));
					count++;
					forward_ptr += sizeof(int);
					break;
				}
				case 's':
				{
					char* dest = va_arg(args, char*);
					size_t size = va_arg(args, size_t);
					if (size == 0) {
						break;
					}
					if (forward_ptr + size > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(dest, forward_ptr, size);
					dest[size - 1] = '\0';//Ensure null-termination
					count++;
					forward_ptr += size;
					break;
				}
				case 'r':
				{
					char* dest = va_arg(args, char*);
					size_t size = va_arg(args, size_t);
					if (size == 0) {
						break;
					}
					if (forward_ptr + size > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(dest, forward_ptr, size);
					//这里不会置零
					count++;
					forward_ptr += size;
					break;
				}
				case 'n':
				{
					size_t* n_ptr = va_arg(args, size_t*);
					if (n_ptr != NULL) {
						*n_ptr = (size_t)(forward_ptr - (const unsigned char*)raw_buffer);
					}//write the number of bytes read so far into the provided pointer
					break;
				}
				case 'c':
				{
					if (forward_ptr + sizeof(char) > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(va_arg(args, char*), forward_ptr, sizeof(char));
					count++;
					forward_ptr += sizeof(char);
					break;
				}
				case 'f':
				{
					if (forward_ptr + sizeof(float) > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(va_arg(args, float*), forward_ptr, sizeof(float));
					count++;
					forward_ptr += sizeof(float);
					break;
				}
				case 'h':
				{
					ptr++;
					if (*ptr == '\0') {
						goto end;
					}
					if (*ptr == 'd') {
						if (forward_ptr + sizeof(short) > (const unsigned char*)raw_buffer + buffer_size) {
							goto end;
						}
						memmove(va_arg(args, short*), forward_ptr, sizeof(short));
						count++;
						forward_ptr += sizeof(short);
						break;
					}
					if (*ptr == 'u') {
						if (forward_ptr + sizeof(unsigned short) > (const unsigned char*)raw_buffer + buffer_size) {
							goto end;
						}
						memmove(va_arg(args, unsigned short*), forward_ptr, sizeof(unsigned short));
						count++;
						forward_ptr += sizeof(unsigned short);
						break;
					}
					if (*ptr == 'h') {
						ptr++;
						if (*ptr == 'd') {
							if (forward_ptr + sizeof(signed char) > (const unsigned char*)raw_buffer + buffer_size) {
								goto end;
							}
							memmove(va_arg(args, signed char*), forward_ptr, sizeof(signed char));
							count++;
							forward_ptr += sizeof(signed char);
							break;
						}
						if (*ptr == 'u') {
							if (forward_ptr + sizeof(unsigned char) > (const unsigned char*)raw_buffer + buffer_size) {
								goto end;
							}
							memmove(va_arg(args, unsigned char*), forward_ptr, sizeof(unsigned char));
							count++;
							forward_ptr += sizeof(unsigned char);
							break;
						}
						if (*ptr == '\0')
						{
							goto end;
						}
						else {
							break;
						}
					}
					break;
				}
				case 'l':
				{
					ptr++;
					if (*ptr == '\0') {
						goto end;
					}
					if (*ptr == 'f') {
						if (forward_ptr + sizeof(double) > (const unsigned char*)raw_buffer + buffer_size) {
							goto end;
						}
						memmove(va_arg(args, double*), forward_ptr, sizeof(double));
						count++;
						forward_ptr += sizeof(double);
						break;
					}
					if (*ptr == 'd') {
						if (forward_ptr + sizeof(long) > (const unsigned char*)raw_buffer + buffer_size) {
							goto end;
						}
						memmove(va_arg(args, long*), forward_ptr, sizeof(long));
						count++;
						forward_ptr += sizeof(long);
						break;
					}
					if (*ptr == 'u') {
						if (forward_ptr + sizeof(unsigned long) > (const unsigned char*)raw_buffer + buffer_size) {
							goto end;
						}
						memmove(va_arg(args, unsigned long*), forward_ptr, sizeof(unsigned long));
						count++;
						forward_ptr += sizeof(unsigned long);
						break;
					}
					if (*ptr == 'l')
					{
						ptr++;
						if (*ptr == 'd') {
							if (forward_ptr + sizeof(long long) > (const unsigned char*)raw_buffer + buffer_size) {
								goto end;
							}
							memmove(va_arg(args, long long*), forward_ptr, sizeof(long long));
							count++;
							forward_ptr += sizeof(long long);
							break;
						}
						if (*ptr == 'u') {
							if (forward_ptr + sizeof(unsigned long long) > (const unsigned char*)raw_buffer + buffer_size) {
								goto end;
							}
							memmove(va_arg(args, unsigned long long*), forward_ptr, sizeof(unsigned long long));
							count++;
							forward_ptr += sizeof(unsigned long long);
							break;
						}
						if (*ptr == '\0')
						{
							goto end;
						}
						else {
							break;
						}
					}
					break;
				}
				case 'u':
				{
					if (forward_ptr + sizeof(unsigned int) > (const unsigned char*)raw_buffer + buffer_size) {
						goto end;
					}
					memmove(va_arg(args, unsigned int*), forward_ptr, sizeof(unsigned int));
					count++;
					forward_ptr += sizeof(unsigned int);
					break;
				}
				default:
					break;
				}
			}
			ptr++;
		}
	end:
		return count;
	}
}