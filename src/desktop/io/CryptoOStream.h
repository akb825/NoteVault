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

#include "OStream.h"
#include <vector>
#include <cstdint>

namespace NoteVault
{

class CryptoOStream : public OStream
{
public:
	CryptoOStream();
	~CryptoOStream();

	bool open(OStream& parentStream, const std::vector<uint8_t>& key,
		const std::vector<uint8_t>& iv);

	size_t write(const void* data, size_t size) override;
	void close() override;

private:
	class Impl;
	Impl* m_impl;
};

} // namespace NoteVault
