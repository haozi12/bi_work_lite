#include "bi_work.h"

static void swap_bytes(void* data, size_t size) {
	if (data == NULL || size == 0) {
		return;
	}
	unsigned char* start = (unsigned char*)data;
	unsigned char* end = (unsigned char*)data + size - 1;
	while (start < end) {
		*start ^= *end;
		*end ^= *start;
		*start ^= *end;
		start++;
		end--;
	}
}

int vswap_endianf(const char* format, va_list args) {
	if (format == NULL) {
		return NULL_BUFFER_OR_FORMAT;
	}
	int count = 0;
	const char* ptr = format;
	while (*ptr != '\0') {
		if (*ptr == '%') {
			ptr++;
			if (*ptr == '\0')
				break;
			switch (*ptr) {
			case 'd':
				swap_bytes(va_arg(args,int*), sizeof(int));
				count++;
				break;
			case 's':
			{
				char* dest = va_arg(args, char*);
				size_t size = va_arg(args, size_t);
				if (size == 0) {
					break;
				}
				swap_bytes(dest, size);
				count++;
				break;
			}
			case 'r':
			{
				char* dest = va_arg(args, char*);
				size_t size = va_arg(args, size_t);
				if (size == 0) {
					break;
				}
				swap_bytes(dest, size);
				count++;
				break;
			}
			case 'c':
				swap_bytes(va_arg(args, char*), sizeof(char));
				count++;
				break;
			case 'f':
				swap_bytes(va_arg(args, float*), sizeof(float));
				count++;
				break;
			case 'h':
				ptr++;
				if (*ptr == '\0') {
					goto end;
				}
				if (*ptr == 'd') {
					swap_bytes(va_arg(args, short*), sizeof(short));
					count++;
					break;
				}
				if (*ptr == 'u') {
					swap_bytes(va_arg(args, unsigned short*), sizeof(unsigned short));
					count++;
					break;
				}
				if (*ptr == 'h') {
					ptr++;
					if (*ptr == 'd') {
						swap_bytes(va_arg(args, signed char*), sizeof(signed char));
						count++;
						break;
					}
					if (*ptr == 'u') {
						swap_bytes(va_arg(args, unsigned char*),sizeof(unsigned char));
						count++;
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
			case 'l':
				ptr++;
				if (*ptr == '\0') {
					goto end;
				}
				if (*ptr == 'f') {
					swap_bytes(va_arg(args, double*), sizeof(double));
					count++;
					break;
				}
				if (*ptr == 'd') {
					swap_bytes(va_arg(args, long*),sizeof(long));
					count++;
					break;
				}
				if (*ptr == 'u') {
					swap_bytes(va_arg(args, unsigned long*), sizeof(unsigned long));
					count++;
					break;
				}
				if (*ptr == 'l')
				{
					ptr++;
					if (*ptr == 'd') {
						swap_bytes(va_arg(args,long long*), sizeof(long long));
						count++;
						break;
					}
					if (*ptr == 'u') {
						swap_bytes(va_arg(args, unsigned long long*),sizeof(unsigned long long));
						count++;
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
			case 'u':
				swap_bytes(va_arg(args, unsigned int*), sizeof(unsigned int));
				count++;
				break;
			default:
				break;
			}
		}
		ptr++;
	}
end:
	return count;
}
