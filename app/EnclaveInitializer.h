#pragma once

#include "EnclaveToken.h"

#include <iostream>

struct EnclaveInitializer {
  static int init(sgx_enclave_id_t* eid, EnclaveToken enclave_token, const std::string& enclave_name) {
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    enclave_token.read(&token);

    ret = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, eid, NULL);

    if (ret != SGX_SUCCESS) {
      std::cout << "SGX error code: " << ret << std::endl;
      return -1;
    }

    if (updated == true) {
      return enclave_token.save(&token);
    }

    return 0;
  }
};
