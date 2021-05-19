#pragma once

struct EnclaveResult {
  static int validate(sgx_status_t status, sgx_status_t ecall_status) {
    if (status != SGX_SUCCESS) {
      std::cout << "SGX status: " << status << std::endl;
      return status;
    }

    if (ecall_status != SGX_SUCCESS) {
      std::cout << "ECall return value: " << ecall_status << std::endl;
      return ecall_status;
    }

    return SGX_SUCCESS;
  }
};
