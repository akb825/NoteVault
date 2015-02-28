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

#include <cstddef>

namespace NoteVault
{

class OStream
{
public:
	OStream() = default;
	OStream(const OStream&) = delete;
	OStream& operator=(const OStream&) = delete;
	virtual ~OStream() = default;

	virtual size_t Write(const void* data, size_t size) = 0;
	virtual void Close() = 0;
};

} // namespace NoteVault