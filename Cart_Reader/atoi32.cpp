#include "atoi32.h"
#include <stddef.h>
#include <inttypes.h>

int32_t atoi32_signed(const char* input_string) {
  if (input_string == NULL) {
    return 0;
  }

  int int_sign = 1;
  int i = 0;

  if (input_string[0] == '-') {
    int_sign = -1;
    i = 1;
  }

  int64_t return_val = 0;

  while (input_string[i] != '\0') {
    if (input_string[i] >= '0' && input_string[i] <= '9') {
      return_val = (return_val * 10) + (input_string[i] - '0');
    } else if (input_string[i] != '\0') {
      return 0;
    }

    i++;
  }

  return_val = return_val * int_sign;

  return return_val;
}

uint32_t atoi32_unsigned(const char* input_string) {
  if (input_string == NULL) {
    return 0;
  }

  int i = 0;

  uint64_t return_val = 0;

  while (input_string[i] != '\0') {
    if (input_string[i] >= '0' && input_string[i] <= '9') {
      return_val = (return_val * 10) + (input_string[i] - '0');
    } else if (input_string[i] != '\0') {
      return 0;
    }

    i++;
  }

  return return_val;
}
