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

#include "NoteFile.h"
#include "Crypto.h"
#include "CryptoIStream.h"
#include "CryptoOStream.h"
#include "../notes/NoteSet.h"
#include <cstring>

#if defined(__BIG_ENDIAN__)
#	define DO_SWAP 0
#elif defined(__LITTLE_ENDIAN__)
#	define DO_SWAP 1
#elif defined(_WIN32) || defined(_WIN64)
#	define DO_SWAP 1
#else
#	include <endian.h>
#	if __BYTE_ORDER == __BIG_ENDIAN
#		define DO_SWAP 0
#	else
#		define DO_SWAP 1
#	endif
#endif

namespace NoteVault
{

static const char cMagicString[] = "NoteVault";

#if DO_SWAP
static uint64_t Swap(uint64_t val)
{
	return
		((val >> 56) & 0x00000000000000FFULL) |
		((val >> 40) & 0x000000000000FF00ULL) |
		((val >> 24) & 0x0000000000FF0000ULL) |
		((val >> 8)  & 0x00000000FF000000ULL) |
		((val << 8)  & 0x000000FF00000000ULL) |
		((val << 24) & 0x0000FF0000000000ULL) |
		((val << 40) & 0x00FF000000000000ULL) |
		((val << 56) & 0xFF00000000000000ULL);
}

static uint32_t Swap(uint32_t val)
{
	return
		((val >> 24) & 0x000000FF) |
		((val >> 8)  & 0x0000FF00) |
		((val << 8)  & 0x00FF0000) |
		((val << 24) & 0xFF000000);
}
#endif

static bool Read(uint64_t& val, IStream& stream)
{
	if (stream.Read(&val, sizeof(val)) != sizeof(val))
		return false;
#if DO_SWAP
	val = Swap(val);
#endif
	return true;
}

static bool Read(uint32_t& val, IStream& stream)
{
	if (stream.Read(&val, sizeof(val)) != sizeof(val))
		return false;
#if DO_SWAP
	val = Swap(val);
#endif
	return true;
}

static bool Read(std::string& val, IStream& stream)
{
	uint32_t length;
	if (!Read(length, stream))
		return false;

	val.resize(length);
	return stream.Read(&val[0], val.size()) == val.size();
}

static bool Write(uint64_t val, OStream& stream)
{
#if DO_SWAP
	val = Swap(val);
#endif
	return stream.Write(&val, sizeof(val)) == sizeof(val);
}

static bool Write(uint32_t val, OStream& stream)
{
#if DO_SWAP
	val = Swap(val);
#endif
	return stream.Write(&val, sizeof(val)) == sizeof(val);
}

static bool Write(const std::string& val, OStream& stream)
{
	if (!Write(static_cast<uint32_t>(val.size()), stream))
		return false;

	return stream.Write(&val[0], val.size()) == val.size();
}

NoteFile::Result NoteFile::LoadNotes(NoteSet& notes, IStream& stream, const std::string& password,
	std::vector<uint8_t>& salt, std::vector<uint8_t>& key)
{
	notes.clear();

	//Read the header: magic string, version, salt, and initialization vector.
	char magicStringCheck[sizeof(cMagicString)];
	if (stream.Read(magicStringCheck, sizeof(magicStringCheck)) != sizeof(magicStringCheck) ||
		strncmp(magicStringCheck, cMagicString, sizeof(cMagicString) != 0))
	{
		return Result::InvalidFile;
	}

	uint32_t version;
	if (!Read(version, stream) || version > cFileVersion)
		return Result::InvalidFile;

	uint32_t saltLen;
	if (!Read(saltLen, stream))
		return Result::IoError;
	salt.resize(saltLen);
	if (stream.Read(salt.data(), saltLen) != saltLen)
		return Result::IoError;

	key = Crypto::GenerateKey(password, salt, Crypto::cDefaultKeyIterations);
	if (key.empty())
		return Result::EncryptionError;

	uint32_t ivLen;
	if (!Read(ivLen, stream))
		return Result::IoError;
	std::vector<uint8_t> iv;
	iv.resize(ivLen);
	if (stream.Read(iv.data(), ivLen) != ivLen)
		return Result::IoError;

	//Main file. (encrypted)
	CryptoIStream cryptoStream;
	if (!cryptoStream.Open(stream, key, iv))
		return Result::EncryptionError;

	//Read the magic string again to verify the correct key
	memset(magicStringCheck, 0, sizeof(magicStringCheck));
	if (cryptoStream.Read(magicStringCheck, sizeof(magicStringCheck)) != sizeof(magicStringCheck))
		return Result::IoError;
	if (strncmp(magicStringCheck, cMagicString, sizeof(cMagicString) != 0))
		return Result::EncryptionError;

	//Read the notes
	uint32_t numNotes;
	if (!Read(numNotes, cryptoStream))
		return Result::IoError;

	std::string title, message;
	for (uint32_t i = 0; i < numNotes; ++i)
	{
		uint64_t id;
		if (!Read(id, cryptoStream))
			return Result::IoError;

		if (!Read(title, cryptoStream) || !Read(message, cryptoStream))
			return Result::IoError;

		Note note(id);
		note.SetTitle(title);
		note.SetMessage(message);
		if (notes.insert(notes.end(), note) == notes.end())
			return Result::IoError;
	}

	return Result::Success;
}

NoteFile::Result NoteFile::SaveNotes(const NoteSet& notes, OStream& stream,
	const std::vector<uint8_t>& salt, const std::vector<uint8_t>& key)
{
	//Read the header: magic string, version, salt, and initialization vector.
	if (stream.Write(cMagicString, sizeof(cMagicString)) != sizeof(cMagicString))
		return Result::IoError;

	if (!Write(cFileVersion, stream))
		return Result::IoError;

	if (!Write(static_cast<uint32_t>(salt.size()), stream))
		return Result::IoError;
	if (stream.Write(salt.data(), salt.size()) != salt.size())
		return Result::IoError;

	//Generate a new initialization vector
	std::vector<uint8_t> iv = Crypto::Random(Crypto::cBlockLenBytes);
	if (!Write(static_cast<uint32_t>(iv.size()), stream))
		return Result::IoError;
	if (stream.Write(iv.data(), iv.size()) != iv.size())
		return Result::IoError;

	//Main file. (encrypted)
	CryptoOStream cryptoStream;
	if (!cryptoStream.Open(stream, key, iv))
		return Result::EncryptionError;

	//Write the magic string again for verifying the correct key
	if (cryptoStream.Write(cMagicString, sizeof(cMagicString)) != sizeof(cMagicString))
		return Result::IoError;

	//Write the notes
	uint32_t numNotes = static_cast<int32_t>(notes.size());
	if (!Write(numNotes, cryptoStream))
		return Result::IoError;

	for (const Note& note : notes)
	{
		if (!Write(note.GetId(), cryptoStream))
			return Result::IoError;

		if (!Write(note.GetTitle(), cryptoStream) || !Write(note.GetMessage(), cryptoStream))
			return Result::IoError;
	}

	return Result::Success;
}

} // namespace NoteVault
