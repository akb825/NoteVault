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

#include "OStream.h"
#include <string>

namespace NoteVault
{

class FileOStream : public OStream
{
public:
	FileOStream();
	~FileOStream();

	bool open(const std::string& fileName);
	size_t write(const void* data, size_t size) override;
	void close() override;
private:
	void* m_file;
};

} // namespace NoteVault
