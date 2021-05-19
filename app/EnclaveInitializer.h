#pragma once

#include "EnclaveToken.h"

#include <iostream>

struct EnclaveInitializer {
  static int init(sgx_enclave_id_t* eid, EnclaveToken enclave_token, const std::string& enclave_name) {
    sgx_launch_token_t token = {0};
    sgx_status_t status = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    enclave_token.read(&token);

    status = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, eid, NULL);

    if (status != SGX_SUCCESS) {
      std::cout << "SGX error code: " << status << std::endl;
      return status;
    }

    if (updated == true) {
      return enclave_token.save(&token);
    }

    return SGX_SUCCESS;
  }
};
