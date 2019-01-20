#ifndef FLAP_ERROR_H
#define FLAP_ERROR_H

void fail_with_error(const char *error);

#define VK_CHECK(fn, err_msg)                                                  \
  if (fn != VK_SUCCESS) {                                                      \
    fail_with_error(err_msg);                                                  \
    exit(EXIT_FAILURE);                                                        \
  }

#endif // FLAP_ERROR_H
