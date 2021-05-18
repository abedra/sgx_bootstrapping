#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "EnclaveInitializer.h"
#include "Persistence.h"
#include <iostream>
#include <fstream>

sgx_enclave_id_t global_eid = 0;

int load(const Persistence &persistence) {
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(int);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  int load_status = persistence.load(sealed_data, sealed_size);

  if (load_status != SGX_SUCCESS) {
    std::cout << "Could not load " << persistence.file_name() << std::endl;
    free(sealed_data);
    return -1;
  }

  std::cout << persistence.file_name() << " loaded" << std::endl;

  int unsealed;
  sgx_status_t ecall_status;
  sgx_status_t status = unseal(global_eid, &ecall_status,
                               (sgx_sealed_data_t*)sealed_data, sealed_size,
                               (uint8_t*)&unsealed, sizeof(unsealed));

  if (status != SGX_SUCCESS || ecall_status != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << persistence.file_name() << std::endl;
    std::cout << "SGX status: " << status << std::endl;
    std::cout << "ECall return value: " << ecall_status << std::endl;

    return -1;
  }

  std::cout << persistence.file_name() << " unsealed to: " << unsealed << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  if (EnclaveInitializer::init(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
    std::cout << "Failed to initialize enclave." << std::endl;
    return 1;
  }

  Persistence persistence{"persistence.seal"};

  return load(persistence);
}
