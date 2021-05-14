#include "errors.h"
#include <string>

struct ECallStatus {
  static bool success(sgx_status_t sgx_status, const std::string& err_msg, sgx_status_t ecall_return_value) {
    if (sgx_status != SGX_SUCCESS || ecall_return_value != SGX_SUCCESS) {
      printf("%s\n", err_msg.c_str());
      errors::print_error_message(sgx_status);
      errors::print_error_message(ecall_return_value);

      return false;
    }

    return true;
  }
};
