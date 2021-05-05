#include "enclave_u.h"
#include "sgx_urts.h"
#include "sgx_tseal.h"

#include <iostream>
#include <fstream>

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

static const std::string BOOTSTRAP_FILE = "bootstrap.seal";

sgx_enclave_id_t global_eid = 0;

void print_error_message(sgx_status_t ret) {
  std::cout << "SGX error code: " << ret << std::endl;
}


bool is_ecall_successful(sgx_status_t sgx_status, const std::string& err_msg, sgx_status_t ecall_return_value) {
  if (sgx_status != SGX_SUCCESS || ecall_return_value != SGX_SUCCESS) {
    printf("%s\n", err_msg.c_str());
    print_error_message(sgx_status);
    print_error_message(ecall_return_value);

    return false;
  }

  return true;
}

int initialize_enclave(sgx_enclave_id_t* eid, const std::string& launch_token_path, const std::string& enclave_name) {
  const char* token_path = launch_token_path.c_str();
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = 0;

  /* Step 1: try to retrieve the launch token saved by last transaction
   *         if there is no token, then create a new one.
   */
  /* try to get the token saved in $HOME */
  FILE* fp = fopen(token_path, "rb");

  if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
    std::cout << "Warning: Failed to create/open the launch token file " << token_path << std::endl;
  }

  if (fp != NULL) {
    /* read the token from saved file */
    size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);

    if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
      /* if token is invalid, clear the buffer */
      memset(&token, 0x0, sizeof(sgx_launch_token_t));
      std::cout << "Warning: Invalid launch token read from " << token_path << std::endl;
    }
  }
  /* Step 2: call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, eid, NULL);

  if (ret != SGX_SUCCESS) {
    std::cout << "SGX error code: " << ret << std::endl;

    if (fp != NULL) {
      fclose(fp);
    }

    return -1;
  }

  /* Step 3: save the launch token if it is updated */
  if (updated == FALSE || fp == NULL) {
    /* if the token is not updated, or file handler is invalid, do not perform saving */
    if (fp != NULL) {
      fclose(fp);
    }

    return 0;
  }

  /* reopen the file with write capablity */
  fp = freopen(token_path, "wb", fp);

  if (fp == NULL) {
    return 0;
  }

  size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);

  if (write_num != sizeof(sgx_launch_token_t)) {
    std::cout << "Warning: Failed to save launch token to " << token_path << std::endl;
  }

  fclose(fp);

  return 0;
}

int save_bootstrap_file(const uint8_t* sealed_data, const size_t sealed_size) {
  std::ofstream file(BOOTSTRAP_FILE, std::ios::out | std::ios::binary);

  if (file.fail()) {
    return 1;
  }

  file.write((const char*) sealed_data, sealed_size);
  file.close();

  return 0;
}

int load_bootstrap_file(uint8_t* sealed_data, const size_t sealed_size) {
  std::ifstream file(BOOTSTRAP_FILE, std::ios::in | std::ios::binary);

  if (file.fail()) {
    return 1;
  }

  file.read((char*) sealed_data, sealed_size);
  file.close();

  return 0;
}

int is_bootstrap_file(void) {
  std::ifstream file(BOOTSTRAP_FILE, std::ios::in | std::ios::binary);

  if (file.fail()) {
    return 0;
  }

  file.close();

  return 1;
}

int provision() {
  std::cout << "bootstrap.seal not found" << std::endl;

  int number = 823;
  size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(number);
  uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
  sgx_status_t ecall_status;
  sgx_status_t status = seal(global_eid, &ecall_status,
                             (uint8_t*)&number, sizeof(number),
                             (sgx_sealed_data_t*)sealed_data, sealed_size);

  if (!is_ecall_successful(status, "Sealing failed :(", ecall_status)) {
    return 1;
  }

  save_bootstrap_file(sealed_data, sealed_size);

  std::cout << "boostrap.seal saved with value: " << number << std::endl;

  return 0;
}

int load() {
    std::cout << "bootstrap.seal located" << std::endl;

    size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(int);
    uint8_t* sealed_data = (uint8_t*)malloc(sealed_size);
    int load_status = load_bootstrap_file(sealed_data, sealed_size);

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

    if (!is_ecall_successful(status, "Unsealing failed :(", ecall_status)) {
      return 1;
    }

    std::cout << "bootstrap.seal unsealed to: " << unsealed << std::endl;

    return 0;
}

int main(int argc, char** argv) {
  if (initialize_enclave(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
    std::cout << "Fail to initialize enclave." << std::endl;
    return 1;
  }

  if (is_bootstrap_file()) {
    return load();
  } else {
    return provision();
  }
}
