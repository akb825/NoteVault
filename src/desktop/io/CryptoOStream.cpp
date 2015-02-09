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
		: m_ParentStream(nullptr) {}

	~Impl()
	{
		if (m_ParentStream)
		{
			//Write the last block to the stream.
			assert(m_ParentStream);
			int encryptSize;
			if (EVP_EncryptFinal_ex(&m_CipherCtx, m_Buffer.data(), &encryptSize))
			{
				if (encryptSize > static_cast<int>(m_Buffer.size()))
					throw std::overflow_error("Buffer overflow!");
				size_t writeSize = m_ParentStream->Write(m_Buffer.data(), encryptSize);
				assert(static_cast<int>(writeSize) == encryptSize);
			}
			else
				assert(false);
		}
		EVP_CIPHER_CTX_cleanup(&m_CipherCtx);
	}

	bool Open(OStream& parentStream, const std::vector<uint8_t>& key,
		const std::vector<uint8_t>& iv)
	{
		EVP_CIPHER_CTX_init(&m_CipherCtx);

		const EVP_CIPHER* cipher = EVP_aes_256_cbc();
		if (EVP_CIPHER_key_length(cipher) != Crypto::cKeyLenBytes)
			return false;
		if (EVP_CIPHER_block_size(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (EVP_CIPHER_iv_length(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (key.size() != Crypto::cKeyLenBytes || iv.size() != Crypto::cBlockLenBytes)
			return false;

		if (!EVP_EncryptInit_ex(&m_CipherCtx, cipher, nullptr, key.data(), iv.data()))
			return false;

		m_Buffer.resize(Crypto::cBlockLenBytes*2);
		m_BufferSize = 0;
		m_BufferPos = 0;
		m_ParentStream = &parentStream;
		return true;
	}

	size_t Write(const void* data, size_t size)
	{
		assert(m_ParentStream);
		const uint8_t* dataBytes = reinterpret_cast<const uint8_t*>(data);

		size_t writeSize = 0;
		while (writeSize < size)
		{
			size_t nextSize = std::min(size, writeSize + Crypto::cBlockLenBytes);
			int encryptLen;
			if (!EVP_EncryptUpdate(&m_CipherCtx, m_Buffer.data(), &encryptLen,
				dataBytes + writeSize, nextSize - writeSize))
			{
				return writeSize;
			}

			if (encryptLen > static_cast<int>(m_Buffer.size()))
				throw std::overflow_error("Buffer overflow!");
			size_t streamWriteSize = m_ParentStream->Write(m_Buffer.data(), encryptLen);
			if (static_cast<int>(streamWriteSize) != encryptLen)
				return writeSize;

			writeSize = nextSize;
		}

		return writeSize;
	}

private:
	OStream* m_ParentStream;
	EVP_CIPHER_CTX m_CipherCtx;
	std::vector<uint8_t> m_Buffer;
	size_t m_BufferSize;
	size_t m_BufferPos;
};

CryptoOStream::CryptoOStream()
	: m_Impl(nullptr)
{
}

CryptoOStream::~CryptoOStream()
{
	Close();
}

bool CryptoOStream::Open(OStream& parentStream, const std::vector<uint8_t>& key,
	const std::vector<uint8_t>& iv)
{
	Close();

	m_Impl = new Impl;
	if (!m_Impl->Open(parentStream, key, iv))
	{
		Close();
		return false;
	}

	return true;
}

size_t CryptoOStream::Write(const void* data, size_t size)
{
	if (!m_Impl)
		return 0;
	return m_Impl->Write(data, size);
}

void CryptoOStream::Close()
{
	delete m_Impl;
	m_Impl = nullptr;
}

} // namespace NoteVault
