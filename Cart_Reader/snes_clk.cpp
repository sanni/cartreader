#include "snes_clk.h"
#include "SdFat.h"
#include "atoi32.h"

int32_t readClockOffset() {
  FsFile clock_file;
  char* clock_buf;
  int16_t i;
  int32_t clock_offset;

  if (!clock_file.open("/snes_clk.txt", FILE_READ)) {
    return INT32_MIN;
  }

  clock_buf = (char*)malloc(12 * sizeof(char));
  i = clock_file.read(clock_buf, 11);
  clock_file.close();
  if (i == -1) {
    free(clock_buf);
    return INT32_MIN;
  } else if ((i == 11) && (clock_buf[0] != '-')) {
    free(clock_buf);
    return INT32_MIN;
  } else {
    clock_buf[i] = 0;
  }

  for (i = 0; i < 12; i++) {
    if (clock_buf[i] != '-' && clock_buf[i] < '0' && clock_buf[i] > '9') {
      if (i == 0) {
        free(clock_buf);
        return INT32_MIN;
      } else if ((i == 1) && (clock_buf[0] == '-')) {
        free(clock_buf);
        return INT32_MIN;
      } else {
        clock_buf[i] = 0;
      }
    }
  }

  clock_offset = atoi32_signed(clock_buf);
  free(clock_buf);

  return clock_offset;
}
