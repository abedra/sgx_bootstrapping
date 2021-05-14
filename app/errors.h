#include <iostream>

struct errors {
  static void print_error_message(sgx_status_t ret) {
    std::cout << "SGX error code: " << ret << std::endl;
  }
};
