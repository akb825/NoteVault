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
#include <cstdint>

namespace NoteVault
{

class Note
{
public:
	explicit Note(uint64_t id)
		: m_id(id) {}

	Note& operator=(const Note& other);

	uint64_t getId() const	{return m_id;}

	const std::string& getTitle() const	{return m_title;}
	void setTitle(const std::string& title)	{m_title = title;}

	const std::string& getMessage() const	{return m_message;}
	void setMessage(const std::string& message)	{m_message = message;}

private:
	uint64_t m_id;
	std::string m_title;
	std::string m_message;
};

inline Note& Note::operator=(const Note& other)
{
	if (this == &other)
		return *this;

	m_title = other.m_title;
	m_message = other.m_message;
	return *this;
}

} // namespace NoteVault
