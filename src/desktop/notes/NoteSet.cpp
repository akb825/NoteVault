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

#include "NoteSet.h"
#include <algorithm>

namespace NoteVault
{

NoteSet::iterator NoteSet::insert(const iterator& pos)
{
	uint64_t id = m_Ids.NewId();
	Note note(id);
	m_Notes.insert(NoteMap::value_type(id, note));
	OrderList::iterator orderPos = m_Order.insert(pos.m_Iter, id);
	return iterator(*this, orderPos);
}

NoteSet::iterator NoteSet::insert(const iterator& pos, const Note& note)
{
	if (!m_Ids.AddId(note.GetId()))
		return end();
	m_Notes.insert(NoteMap::value_type(note.GetId(), note));
	OrderList::iterator orderPos = m_Order.insert(pos.m_Iter, note.GetId());
	return iterator(*this, orderPos);
}

int NoteSet::erase(uint64_t id)
{
	iterator foundIter = find(id);
	if (foundIter == end())
		return 0;
	erase(foundIter);
	return 1;
}

NoteSet::iterator NoteSet::erase(const iterator& iter)
{
	m_Ids.RemoveId(iter->GetId());
	m_Notes.erase(iter->GetId());
	OrderList::iterator newIter = m_Order.erase(iter.m_Iter);
	return iterator(*this, newIter);
}

NoteSet::iterator NoteSet::find(uint64_t id)
{
	OrderList::iterator foundIter = std::find(m_Order.begin(), m_Order.end(), id);
	if (foundIter == m_Order.end())
		return end();
	return iterator(*this, foundIter);
}

NoteSet::const_iterator NoteSet::find(uint64_t id) const
{
	OrderList::const_iterator foundIter = std::find(m_Order.begin(), m_Order.end(), id);
	if (foundIter == m_Order.end())
		return end();
	return const_iterator(*this, foundIter);
}

Note* NoteSet::find_note(uint64_t id)
{
	NoteMap::iterator foundIter = m_Notes.find(id);
	if (foundIter == m_Notes.end())
		return nullptr;
	return &foundIter->second;
}

const Note* NoteSet::find_note(uint64_t id) const
{
	NoteMap::const_iterator foundIter = m_Notes.find(id);
	if (foundIter == m_Notes.end())
		return nullptr;
	return &foundIter->second;
}

void NoteSet::clear()
{
	m_Notes.clear();
	m_Order.clear();
	m_Ids.Clear();
}

Note& NoteSet::operator[](size_t index)
{
	return m_Notes.find(m_Order[index])->second;
}

const Note& NoteSet::operator[](size_t index) const
{
	return m_Notes.find(m_Order[index])->second;
}

NoteSet::iterator NoteSet::begin()
{
	return iterator(*this, m_Order.begin());
}

NoteSet::const_iterator NoteSet::end() const
{
	return const_iterator(*this, m_Order.end());
}

NoteSet::const_iterator NoteSet::begin() const
{
	return const_iterator(*this, m_Order.begin());
}

NoteSet::iterator NoteSet::end()
{
	return iterator(*this, m_Order.end());
}

} // namespace NoteVault
