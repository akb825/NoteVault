#pragma once
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

#include <vector>
#include <string>
#include <cstdint>

namespace NoteVault
{

class Crypto
{
public:
	static const unsigned int cKeyLen = 256;
	static const unsigned int cKeyLenBytes = cKeyLen/8;
	static const unsigned int cBlockLen = 128;
	static const unsigned int cBlockLenBytes = cBlockLen/8;
	static const unsigned int cSaltLen = 128;
	static const unsigned int cSaltLenBytes = cSaltLen/8;
	static const unsigned int cDefaultKeyIterations = 30000;

	static const unsigned int cVer0KeyIterations = 100000;

	static void initialize();
	static std::vector<uint8_t> generateKey(const std::string& password,
		const std::vector<uint8_t>& salt, unsigned int numIterations);
	static std::vector<uint8_t> random(unsigned int numBytes);
};

} // namespace NoteVault
