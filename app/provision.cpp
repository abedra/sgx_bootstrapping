#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "EnclaveInitializer.h"
#include "Bootstrap.h"
#include <iostream>
#include <fstream>

sgx_enclave_id_t global_eid = 0;

int provision(const Bootstrap &bootstrap) {
  int number = 123;
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(number);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  sgx_status_t ecall_status;
  sgx_status_t status = seal(global_eid, &ecall_status,
                             (uint8_t*)&number, sizeof(number),
                             (sgx_sealed_data_t*)sealed_data, sealed_size);

  if (status != SGX_SUCCESS || ecall_status != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << bootstrap.file_name() << std::endl;
    std::cout << "SGX status: " << status << std::endl;
    std::cout << "ECall return value: " << ecall_status << std::endl;

    return -1;
  }

  bootstrap.save(sealed_data, sealed_size);

  std::cout << bootstrap.file_name() << " saved with value: " << number << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  if (EnclaveInitializer::init(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
    std::cout << "Failed to initialize enclave." << std::endl;
    return 1;
  }

  Bootstrap bootstrap{"bootstrap.seal"};

  return provision(bootstrap);
}
