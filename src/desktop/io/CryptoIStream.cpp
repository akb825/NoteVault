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

#include "CryptoIStream.h"
#include "Crypto.h"
#include <openssl/evp.h>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstring>

namespace NoteVault
{

class CryptoIStream::Impl
{
public:
	Impl()
		: m_ParentStream(nullptr), m_BufferSize(0), m_BufferPos(0) {}

	~Impl()
	{
		EVP_CIPHER_CTX_cleanup(&m_CipherCtx);
	}

	bool Open(IStream& parentStream, const std::vector<uint8_t>& key,
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

		if (!EVP_DecryptInit_ex(&m_CipherCtx, cipher, nullptr, key.data(), iv.data()))
			return false;

		m_Buffer.resize(Crypto::cBlockLenBytes*2);
		m_BufferSize = 0;
		m_BufferPos = 0;
		m_ParentStream = &parentStream;
		return true;
	}

	size_t Read(void* data, size_t size)
	{
		assert(m_ParentStream);
		uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

		size_t readSize = 0;
		while (readSize < size)
		{
			if (m_BufferPos < m_BufferSize)
			{
				//Copy over from the buffer
				size_t copySize = std::min(m_BufferSize - m_BufferPos, size - readSize);
				memcpy(dataBytes + readSize, m_Buffer.data() + m_BufferPos, copySize);
				readSize += copySize;
				m_BufferPos += copySize;
				assert(readSize <= size);
				assert(m_BufferPos <= m_BufferSize);
			}
			else
			{
				//Read into the buffer
				uint8_t readBuffer[Crypto::cBlockLenBytes];
				//Input must always be a multiple of the block size.
				size_t streamReadSize = m_ParentStream->Read(readBuffer, Crypto::cBlockLenBytes);

				int decryptSize;
				if (streamReadSize == 0)
				{
					//If we've reached the end of the stream, then it's the last block.
					if (!EVP_DecryptFinal_ex(&m_CipherCtx, m_Buffer.data(), &decryptSize))
						return readSize;
				}
				else
				{
					if (!EVP_DecryptUpdate(&m_CipherCtx, m_Buffer.data(), &decryptSize, readBuffer,
						Crypto::cBlockLenBytes))
					{
						return readSize;
					}
				}

				if (decryptSize > static_cast<int>(m_Buffer.size()))
					throw std::overflow_error("Buffer overflow!");
				m_BufferSize = decryptSize;
				m_BufferPos = 0;
			}
		}

		return readSize;
	}

private:
	IStream* m_ParentStream;
	EVP_CIPHER_CTX m_CipherCtx;
	std::vector<uint8_t> m_Buffer;
	size_t m_BufferSize;
	size_t m_BufferPos;
};

CryptoIStream::CryptoIStream()
	: m_Impl(nullptr)
{
}

CryptoIStream::~CryptoIStream()
{
	Close();
}

bool CryptoIStream::Open(IStream& parentStream, const std::vector<uint8_t>& key,
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

size_t CryptoIStream::Read(void* data, size_t size)
{
	if (!m_Impl)
		return 0;
	return m_Impl->Read(data, size);
}

void CryptoIStream::Close()
{
	delete m_Impl;
	m_Impl = nullptr;
}

} // namespace NoteVault