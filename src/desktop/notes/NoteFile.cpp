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
#include "NoteSet.h"
#include "../io/IStream.h"
#include "../io/OStream.h"

#if defined(__BIG_ENDIAN__)
#	define DO_SWAP 1
#elif defined(__LITTLE_ENDIAN__)
#	define DO_SWAP 0
#elif defined(_WIN32) || defined(_WIN64)
#	define DO_SWAP 0
#else
#	include <endian.h>
#	if __BYTE_ORDER == __BIG_ENDIAN
#		define DO_SWAP 1
#	else
#		define DO_SWAP 0
#	endif
#endif

namespace NoteVault
{

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

bool NoteFile::LoadNotes(NoteSet& notes, IStream& stream)
{
	uint32_t numNotes;
	if (!Read(numNotes, stream))
		return false;

	notes.clear();
	std::string title, message;
	for (uint32_t i = 0; i < numNotes; ++i)
	{
		uint64_t id;
		if (!Read(id, stream))
			return false;

		if (!Read(title, stream) || !Read(message, stream))
			return false;

		Note note(id);
		note.SetTitle(title);
		note.SetMessage(message);
		if (notes.insert(notes.end(), note) == notes.end())
			return false;
	}

	return true;
}

bool NoteFile::SaveNotes(const NoteSet& notes, OStream& stream)
{
	uint32_t numNotes = static_cast<int32_t>(notes.size());
	if (!Write(numNotes, stream))
		return false;

	for (const Note& note : notes)
	{
		if (!Write(note.GetId(), stream))
			return false;

		if (!Write(note.GetTitle(), stream) || !Write(note.GetMessage(), stream))
			return false;
	}

	return true;
}

} // namespace NoteVault
