#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "EnclaveInitializer.h"
#include "EnclaveToken.h"
#include "Persistence.h"
#include <iostream>
#include <fstream>
#include <filesystem>

sgx_enclave_id_t global_eid = 0;

int load(const Persistence &persistence) {
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(int);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  int load_status = persistence.load(sealed_data, sealed_size);

  if (load_status != SGX_SUCCESS) {
    std::cout << "Could not load " << persistence.path() << std::endl;
    free(sealed_data);
    return load_status;
  }

  std::cout << persistence.path() << " loaded" << std::endl;

  int unsealed;
  sgx_status_t ecall_status;
  sgx_status_t status = unseal(global_eid, &ecall_status,
                               (sgx_sealed_data_t*)sealed_data, sealed_size,
                               (uint8_t*)&unsealed, sizeof(unsealed));

  if (status != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << persistence.path() << std::endl;
    std::cout << "SGX status: " << status << std::endl;

    return status;
  }

  if (ecall_status != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << persistence.path() << std::endl;
    std::cout << "ECall return value: " << ecall_status << std::endl;

    return ecall_status;
  }

  std::cout << persistence.path() << " unsealed to: " << unsealed << std::endl;

  return SGX_SUCCESS;
}

int main(int argc, char** argv) {
  Persistence persistence{std::filesystem::path{"persistence.seal"}};

  if (!persistence.exists()) {
    std::cout << persistence.path() << " does not exist" << std::endl;
    return -1;
  }

  if (EnclaveInitializer::init(&global_eid, EnclaveToken{"enclave.token"}, "enclave.signed.so") != SGX_SUCCESS) {
    std::cout << "Failed to initialize enclave." << std::endl;
    return -1;
  }

  return load(persistence);
}
