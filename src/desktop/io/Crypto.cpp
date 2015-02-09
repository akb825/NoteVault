/*
 * Copyright 2015 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/engine.h>
#include <openssl/err.h>

namespace NoteVault
{

void Crypto::Initialize()
{
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	ENGINE_load_builtin_engines();
	RAND_set_rand_engine(nullptr);
}

std::vector<uint8_t> Crypto::GenerateKey(const std::string& password,
	const std::vector<uint8_t>& salt, unsigned int numIterations)
{
	std::vector<uint8_t> key;
	key.resize(cKeyLenBytes);
	PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), static_cast<int>(password.size()), salt.data(),
		static_cast<int>(salt.size()), numIterations, cKeyLenBytes, key.data());
	return key;
}

std::vector<uint8_t> Crypto::Random(unsigned int numBytes)
{
	std::vector<uint8_t> randBytes;
	randBytes.resize(numBytes);
	RAND_bytes(randBytes.data(), numBytes);
	return randBytes;
}

} // namespace NoteVault
