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
	uint64_t id = m_ids.newId();
	Note note(id);
	m_notes.insert(NoteMap::value_type(id, note));
	OrderList::iterator orderPos = m_order.insert(pos.m_iter, id);
	return iterator(*this, orderPos);
}

NoteSet::iterator NoteSet::insert(const iterator& pos, const Note& note)
{
	if (!m_ids.addId(note.getId()))
		return end();
	m_notes.insert(NoteMap::value_type(note.getId(), note));
	OrderList::iterator orderPos = m_order.insert(pos.m_iter, note.getId());
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
	m_ids.removeId(iter->getId());
	m_notes.erase(iter->getId());
	OrderList::iterator newIter = m_order.erase(iter.m_iter);
	return iterator(*this, newIter);
}

NoteSet::iterator NoteSet::find(uint64_t id)
{
	OrderList::iterator foundIter = std::find(m_order.begin(), m_order.end(), id);
	if (foundIter == m_order.end())
		return end();
	return iterator(*this, foundIter);
}

NoteSet::const_iterator NoteSet::find(uint64_t id) const
{
	OrderList::const_iterator foundIter = std::find(m_order.begin(), m_order.end(), id);
	if (foundIter == m_order.end())
		return end();
	return const_iterator(*this, foundIter);
}

Note* NoteSet::find_note(uint64_t id)
{
	NoteMap::iterator foundIter = m_notes.find(id);
	if (foundIter == m_notes.end())
		return nullptr;
	return &foundIter->second;
}

const Note* NoteSet::find_note(uint64_t id) const
{
	NoteMap::const_iterator foundIter = m_notes.find(id);
	if (foundIter == m_notes.end())
		return nullptr;
	return &foundIter->second;
}

void NoteSet::clear()
{
	m_notes.clear();
	m_order.clear();
	m_ids.clear();
}

Note& NoteSet::operator[](size_t index)
{
	return m_notes.find(m_order[index])->second;
}

const Note& NoteSet::operator[](size_t index) const
{
	return m_notes.find(m_order[index])->second;
}

NoteSet::iterator NoteSet::begin()
{
	return iterator(*this, m_order.begin());
}

NoteSet::const_iterator NoteSet::end() const
{
	return const_iterator(*this, m_order.end());
}

NoteSet::const_iterator NoteSet::begin() const
{
	return const_iterator(*this, m_order.begin());
}

NoteSet::iterator NoteSet::end()
{
	return iterator(*this, m_order.end());
}

} // namespace NoteVault
