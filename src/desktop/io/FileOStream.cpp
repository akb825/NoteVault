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

#include "FileOStream.h"
#include <cstdio>

namespace NoteVault
{

FileOStream::FileOStream()
	: m_file(nullptr)
{
}

FileOStream::~FileOStream()
{
	close();
}

bool FileOStream::open(const std::string& fileName)
{
	close();

	FILE* file = fopen(fileName.c_str(), "wb");
	if (!file)
		return false;
	m_file = file;
	return true;
}

size_t FileOStream::write(const void* data, size_t size)
{
	if (!m_file)
		return 0;

	FILE* file = reinterpret_cast<FILE*>(m_file);
	return fwrite(data, 1, size, file);
}

void FileOStream::close()
{
	if (!m_file)
		return;

	fclose(reinterpret_cast<FILE*>(m_file));
	m_file = nullptr;
}

} // namespace NoteVault
