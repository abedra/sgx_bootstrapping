#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"
#include "EnclaveInitializer.h"
#include "EnclaveResult.h"
#include "Persistence.h"
#include <iostream>
#include <fstream>

sgx_enclave_id_t global_eid = 0;

static int read_random_number() {
  try {
    std::string number_string;
    std::getline(std::cin, number_string);
    return std::stoi(number_string);
  } catch (std::invalid_argument e) {
    return -1;
  }
}

static int provision(const Persistence &persistence, int number) {
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(number);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  sgx_status_t ecall_status;
  sgx_status_t status = seal(global_eid, &ecall_status,
                             (uint8_t*)&number, sizeof(number),
                             (sgx_sealed_data_t*)sealed_data, sealed_size);

  int validation_result = EnclaveResult::validate(status, ecall_status);
  if (validation_result != SGX_SUCCESS) {
    std::cout << "Failed to unseal " << persistence.path() << std::endl;
    return validation_result;
  }

  persistence.save(sealed_data, sealed_size);

  std::cout << persistence.path() << " saved with value: " << number << std::endl;

  return SGX_SUCCESS;
}

int main(int argc, char** argv) {
  Persistence persistence{std::filesystem::path{"persistence.seal"}};

  if (!persistence.exists()) {
    std::cout << persistence.path() << " does not exist, creating" << std::endl;
  }

  if (EnclaveInitializer::init(&global_eid, EnclaveToken{"enclave.token"}, "enclave.signed.so") != SGX_SUCCESS) {
    std::cout << "Failed to initialize enclave." << std::endl;
    return 1;
  }

  int number = read_random_number();
  if (number == -1) {
    std::cout << "Failed to read random number from stdin" << std::endl;
    return number;
  } else {
    return provision(persistence, number);
  }
}
