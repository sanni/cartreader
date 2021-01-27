#include "snes_clk.h"
#include <SdFat.h>

int32_t readClockOffset() {
	File clock_file;
	unsigned char* clock_buf;
	int16_t i;
	int32_t clock_offset;
	if(!clock_file.open("/snes_clk.txt", FILE_READ)) {
		return INT32_MIN;
	}

	clock_buf = malloc(16 * sizeof(char));
	i = clock_file.read(clock_buf, 16);
	clock_file.close();
	if(i == -1) {
		free(clock_buf);
		return 0;
	} else if(i < 16) {
		clock_buf[i] = 0;
	}

	clock_offset = (int32_t)atoi(clock_buf);
	free(clock_buf);

	return clock_offset;
}
