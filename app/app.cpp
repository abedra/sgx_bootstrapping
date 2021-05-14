#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "ECallStatus.h"
#include "EnclaveInitializer.h"
#include "Bootstrap.h"
#include <iostream>
#include <fstream>

sgx_enclave_id_t global_eid = 0;

int provision(const Bootstrap &bootstrap) {
  std::cout << "bootstrap.seal not found" << std::endl;

  int number = 823;
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(number);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  sgx_status_t ecall_status;
  sgx_status_t status = seal(global_eid, &ecall_status,
                             (uint8_t*)&number, sizeof(number),
                             (sgx_sealed_data_t*)sealed_data, sealed_size);

  if (!ECallStatus::success(status, "Sealing failed :(", ecall_status)) {
    return 1;
  }

  bootstrap.save(sealed_data, sealed_size);

  std::cout << "boostrap.seal saved with value: " << number << std::endl;

  return 0;
}

int load(const Bootstrap &bootstrap) {
    std::cout << "bootstrap.seal located" << std::endl;

    size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(int);
    uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
    int load_status = bootstrap.load(sealed_data, sealed_size);

    if (load_status != SGX_SUCCESS) {
      free(sealed_data);
      return -1;
    }

    std::cout << "bootstrap.seal loaded" << std::endl;

    int unsealed;
    sgx_status_t ecall_status;
    sgx_status_t status = unseal(global_eid, &ecall_status,
                                 (sgx_sealed_data_t*)sealed_data, sealed_size,
                                 (uint8_t*)&unsealed, sizeof(unsealed));

    if (!ECallStatus::success(status, "Unsealing failed :(", ecall_status)) {
      return 1;
    }

    std::cout << "bootstrap.seal unsealed to: " << unsealed << std::endl;

    return 0;
}

int main(int argc, char** argv) {
  if (EnclaveInitializer::init(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
    std::cout << "Fail to initialize enclave." << std::endl;
    return 1;
  }

  Bootstrap bootstrap{"bootstrap.seal"};

  if (bootstrap.is_bootstrap_file()) {
    return load(bootstrap);
  } else {
    return provision(bootstrap);
  }
}
