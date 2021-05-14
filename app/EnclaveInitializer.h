#include <iostream>

struct EnclaveInitializer {
  static int init(sgx_enclave_id_t* eid, const std::string& launch_token_path, const std::string& enclave_name) {
    const char* token_path = launch_token_path.c_str();
    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    FILE* fp = fopen(token_path, "rb");

    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
      std::cout << "Warning: Failed to create/open the launch token file " << token_path << std::endl;
    }

    if (fp != NULL) {
      size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);

      if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
        memset(&token, 0x0, sizeof(sgx_launch_token_t));
        std::cout << "Warning: Invalid launch token read from " << token_path << std::endl;
      }
    }

    ret = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, eid, NULL);

    if (ret != SGX_SUCCESS) {
      std::cout << "SGX error code: " << ret << std::endl;

      if (fp != NULL) {
        fclose(fp);
      }

      return -1;
    }

    if (updated == false || fp == NULL) {
      if (fp != NULL) {
        fclose(fp);
      }

      return 0;
    }

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
};
