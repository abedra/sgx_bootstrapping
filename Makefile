#
# Copyright (C) 2011-2016 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#   * Neither the name of Intel Corporation nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

######## SGX SDK Settings ########

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_PRERELEASE=1
SGX_ARCH ?= x64
SGX_COMMON_CFLAGS := -m64
SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
		SGX_COMMON_CFLAGS += -O0 -g
else
		SGX_COMMON_CFLAGS += -O2
endif

######## App Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

Unseal_Cpp_Files := app/unseal.cpp
Unseal_Include_Paths := -Iapp -I$(SGX_SDK)/include -Iinclude -Itest
Provision_Cpp_Files := app/provision.cpp
Provision_Include_Paths := -Iapp -I$(SGX_SDK)/include -Iinclude -Itest

Unseal_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(Unseal_Include_Paths)
Provision_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(Provision_Include_Paths)

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
		Unseal_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
		Unseal_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
		Unseal_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

Unseal_Cpp_Flags := $(Unseal_C_Flags) -std=c++17
Unseal_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread
Provision_Cpp_Flags := $(Provision_C_Flags) -std=c++17
Provision_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread

ifneq ($(SGX_MODE), HW)
	Unseal_Link_Flags += -lsgx_uae_service_sim
	Provision_Link_Flags += -lsgx_uae_service_sim
else
	Unseal_Link_Flags += -lsgx_uae_service
	Provision_Link_Flags += -lsgx_uae_service
endif

Unseal_Cpp_Objects := $(Unseal_Cpp_Files:.cpp=.o)
Provision_Cpp_Objects := $(Provision_Cpp_Files:.cpp=.o)

Unseal_Name := unseal
Provision_Name := provision

######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

Enclave_Cpp_Files := enclave/enclave.cpp
Enclave_Include_Paths := -Ienclave -Iinclude -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/stlport

Enclave_C_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Enclave_Include_Paths)
Enclave_Cpp_Flags := $(Enclave_C_Flags) -std=c++03 -nostdinc++
Enclave_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0

Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := enclave/enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: $(Unseal_Name) $(Provision_Name) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(Unseal_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool. See User's Guide for more details."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: $(Unseal_Name) $(Provision_Name) $(Signed_Enclave_Name)
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(Unseal_Name)
	@$(CURDIR)/$(Provision_Name)
	@echo "RUN  =>  $(Unseal_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## App Objects ########

app/enclave_u.c: $(SGX_EDGER8R) enclave/enclave.edl
	@cd app && $(SGX_EDGER8R) --untrusted ../enclave/enclave.edl --search-path ../enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

app/enclave_u.o: app/enclave_u.c
	@$(CC) $(Unseal_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

app/%.o: app/%.cpp
	@$(CXX) $(Unseal_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(Unseal_Name): app/enclave_u.o $(Unseal_Cpp_Objects)
	@$(CXX) $^ -o $@ $(Unseal_Link_Flags)
	@echo "LINK =>  $@"

$(Provision_Name): app/enclave_u.o $(Provision_Cpp_Objects)
	@$(CXX) $^ -o $@ $(Provision_Link_Flags)
	@echo "LINK =>  $@"


######## Enclave Objects ########

enclave/enclave_t.c: $(SGX_EDGER8R) enclave/enclave.edl
	@cd enclave && $(SGX_EDGER8R) --trusted ../enclave/enclave.edl --search-path ../enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

enclave/enclave_t.o: enclave/enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

enclave/%.o: enclave/%.cpp
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(Enclave_Name): enclave/enclave_t.o $(Enclave_Cpp_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key enclave/enclave_private.pem -enclave $(Enclave_Name) -out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"

.PHONY: clean

clean:
	@rm -f $(Unseal_Name) $(Provision_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(Unseal_Cpp_Objects) $(Provision_Cpp_Objects) app/enclave_u.* $(Enclave_Cpp_Objects) enclave/enclave_t.*
