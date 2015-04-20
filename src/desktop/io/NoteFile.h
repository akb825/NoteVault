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

#include <string>
#include <vector>
#include <cstdint>

namespace NoteVault
{

class IStream;
class OStream;
class NoteSet;

class NoteFile
{
public:
	static const uint32_t cFileVersion = 0;

	enum class Result
	{
		Success,
		InvalidFile,
		InvalidVersion,
		IoError,
		EncryptionError
	};

	static Result loadNotes(NoteSet& notes, IStream& stream, const std::string& password,
		std::vector<uint8_t>& salt, std::vector<uint8_t>& key);
	static Result saveNotes(const NoteSet& notes, OStream& stream,
		const std::vector<uint8_t>& salt, const std::vector<uint8_t>& key);
};

} // namespace NoteVault
