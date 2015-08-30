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
		: m_parentStream(nullptr), m_bufferSize(0), m_bufferPos(0) {}

	~Impl()
	{
		EVP_CIPHER_CTX_cleanup(&m_cipherCtx);
	}

	bool open(IStream& parentStream, const std::vector<uint8_t>& key,
		const std::vector<uint8_t>& iv)
	{
		EVP_CIPHER_CTX_init(&m_cipherCtx);

		const EVP_CIPHER* cipher = EVP_aes_256_cbc();
		if (EVP_CIPHER_key_length(cipher) != Crypto::cKeyLenBytes)
			return false;
		if (EVP_CIPHER_block_size(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (EVP_CIPHER_iv_length(cipher) != Crypto::cBlockLenBytes)
			return false;
		if (key.size() != Crypto::cKeyLenBytes || iv.size() != Crypto::cBlockLenBytes)
			return false;

		if (!EVP_DecryptInit_ex(&m_cipherCtx, cipher, nullptr, key.data(), iv.data()))
			return false;

		m_buffer.resize(Crypto::cBlockLenBytes*2);
		m_bufferSize = 0;
		m_bufferPos = 0;
		m_parentStream = &parentStream;
		return true;
	}

	size_t read(void* data, size_t size)
	{
		assert(m_parentStream);
		uint8_t* dataBytes = reinterpret_cast<uint8_t*>(data);

		size_t readSize = 0;
		while (readSize < size)
		{
			if (m_bufferPos < m_bufferSize)
			{
				//Copy over from the buffer
				size_t copySize = std::min(m_bufferSize - m_bufferPos, size - readSize);
				memcpy(dataBytes + readSize, m_buffer.data() + m_bufferPos, copySize);
				readSize += copySize;
				m_bufferPos += copySize;
				assert(readSize <= size);
				assert(m_bufferPos <= m_bufferSize);
			}
			else
			{
				//Read into the buffer
				uint8_t readBuffer[Crypto::cBlockLenBytes];
				//Input must always be a multiple of the block size.
				size_t streamReadSize = m_parentStream->read(readBuffer, Crypto::cBlockLenBytes);

				int decryptSize;
				if (streamReadSize == 0)
				{
					//If we've reached the end of the stream, then it's the last block.
					if (!EVP_DecryptFinal_ex(&m_cipherCtx, m_buffer.data(), &decryptSize))
						return readSize;
				}
				else
				{
					if (!EVP_DecryptUpdate(&m_cipherCtx, m_buffer.data(), &decryptSize, readBuffer,
						Crypto::cBlockLenBytes))
					{
						return readSize;
					}
				}

				if (decryptSize > static_cast<int>(m_buffer.size()))
					throw std::overflow_error("Buffer overflow!");
				m_bufferSize = decryptSize;
				m_bufferPos = 0;
			}
		}

		return readSize;
	}

private:
	IStream* m_parentStream;
	EVP_CIPHER_CTX m_cipherCtx;
	std::vector<uint8_t> m_buffer;
	size_t m_bufferSize;
	size_t m_bufferPos;
};

CryptoIStream::CryptoIStream()
	: m_impl(nullptr)
{
}

CryptoIStream::~CryptoIStream()
{
	close();
}

bool CryptoIStream::open(IStream& parentStream, const std::vector<uint8_t>& key,
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

size_t CryptoIStream::read(void* data, size_t size)
{
	if (!m_impl)
		return 0;
	return m_impl->read(data, size);
}

void CryptoIStream::close()
{
	delete m_impl;
	m_impl = nullptr;
}

} // namespace NoteVault
