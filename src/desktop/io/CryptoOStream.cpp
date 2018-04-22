/*
 * Copyright 2015 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CryptoOStream.h"
#include "Crypto.h"
#include <openssl/evp.h>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstring>

namespace NoteVault
{

class CryptoOStream::Impl
{
public:
	Impl()
		: m_parentStream(nullptr), m_cipherCtx(nullptr) {}

	~Impl()
	{
		if (m_parentStream)
		{
			//Write the last block to the stream.
			assert(m_parentStream);
			int encryptSize;
			if (EVP_EncryptFinal_ex(m_cipherCtx, m_buffer.data(), &encryptSize))
			{
				if (encryptSize > static_cast<int>(m_buffer.size()))
					throw std::overflow_error("Buffer overflow!");
				size_t writeSize = m_parentStream->write(m_buffer.data(), encryptSize);
				(void)writeSize;
				assert(static_cast<int>(writeSize) == encryptSize);
			}
			else
				assert(false);
		}
		EVP_CIPHER_CTX_cleanup(m_cipherCtx);
		EVP_CIPHER_CTX_free(m_cipherCtx);
	}

	bool open(OStream& parentStream, const std::vector<uint8_t>& key,
		const std::vector<uint8_t>& iv)
	{
		m_cipherCtx = EVP_CIPHER_CTX_new();
		EVP_CIPHER_CTX_init(m_cipherCtx);

		const EVP_CIPHER* cipher = EVP_aes_256_cbc();
		if (EVP_CIPHER_key_length(cipher) != Crypto::cKeyLenBytes)
			return false;
		if (EVP_CIPHER_block_size(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (EVP_CIPHER_iv_length(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (key.size() != Crypto::cKeyLenBytes || iv.size() != Crypto::cBlockLenBytes)
			return false;

		if (!EVP_EncryptInit_ex(m_cipherCtx, cipher, nullptr, key.data(), iv.data()))
			return false;

		m_buffer.resize(Crypto::cBlockLenBytes*2);
		m_bufferSize = 0;
		m_bufferPos = 0;
		m_parentStream = &parentStream;
		return true;
	}

	size_t write(const void* data, size_t size)
	{
		assert(m_parentStream);
		const uint8_t* dataBytes = reinterpret_cast<const uint8_t*>(data);

		size_t writeSize = 0;
		while (writeSize < size)
		{
			size_t nextSize = std::min(size, writeSize + Crypto::cBlockLenBytes);
			int encryptLen;
			if (!EVP_EncryptUpdate(m_cipherCtx, m_buffer.data(), &encryptLen,
				dataBytes + writeSize, nextSize - writeSize))
			{
				return writeSize;
			}

			if (encryptLen > static_cast<int>(m_buffer.size()))
				throw std::overflow_error("Buffer overflow!");
			size_t streamWriteSize = m_parentStream->write(m_buffer.data(), encryptLen);
			if (static_cast<int>(streamWriteSize) != encryptLen)
				return writeSize;

			writeSize = nextSize;
		}

		return writeSize;
	}

private:
	OStream* m_parentStream;
	EVP_CIPHER_CTX* m_cipherCtx;
	std::vector<uint8_t> m_buffer;
	size_t m_bufferSize;
	size_t m_bufferPos;
};

CryptoOStream::CryptoOStream()
	: m_impl(nullptr)
{
}

CryptoOStream::~CryptoOStream()
{
	close();
}

bool CryptoOStream::open(OStream& parentStream, const std::vector<uint8_t>& key,
	const std::vector<uint8_t>& iv)
{
	close();

	m_impl = new Impl;
	if (!m_impl->open(parentStream, key, iv))
	{
		close();
		return false;
	}

	return true;
}

size_t CryptoOStream::write(const void* data, size_t size)
{
	if (!m_impl)
		return 0;
	return m_impl->write(data, size);
}

void CryptoOStream::close()
{
	delete m_impl;
	m_impl = nullptr;
}

} // namespace NoteVault
