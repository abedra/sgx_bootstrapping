#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "EnclaveInitializer.h"
#include "Persistence.h"
#include <iostream>
#include <fstream>

sgx_enclave_id_t global_eid = 0;

int provision(const Persistence &persistence) {
  int number = 123;
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(number);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  sgx_status_t ecall_status;
  sgx_status_t status = seal(global_eid, &ecall_status,
                             (uint8_t*)&number, sizeof(number),
                             (sgx_sealed_data_t*)sealed_data, sealed_size);

  if (status != SGX_SUCCESS || ecall_status != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << persistence.path() << std::endl;
    std::cout << "SGX status: " << status << std::endl;
    std::cout << "ECall return value: " << ecall_status << std::endl;

    return -1;
  }

  persistence.save(sealed_data, sealed_size);

  std::cout << persistence.path() << " saved with value: " << number << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  Persistence persistence{std::filesystem::path{"persistence.seal"}};

  if (!persistence.exists()) {
    std::cout << persistence.path() << " does not exist, creating" << std::endl;
  }

  if (EnclaveInitializer::init(&global_eid, EnclaveToken{"enclave.token"}, "enclave.signed.so") < 0) {
    std::cout << "Failed to initialize enclave." << std::endl;
    return 1;
  }

  return provision(persistence);
}
