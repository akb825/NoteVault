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

#include "Note.h"
#include "IdFactory.h"
#include <unordered_map>
#include <vector>
#include <iterator>
#include <algorithm>

namespace NoteVault
{

class NoteSet
{
public:
	class iterator;
	class const_iterator;

	iterator insert(const iterator& pos);
	iterator insert(const iterator& pos, const Note& note);

	int erase(uint64_t id);
	int erase(const Note& note)	{return erase(note.getId());}
	iterator erase(const iterator& iter);

	iterator find(uint64_t id);
	const_iterator find(uint64_t id) const;

	Note* find_note(uint64_t id);
	const Note* find_note(uint64_t id) const;

	size_t size() const	{return m_notes.size();}
	void clear();

	Note& operator[](size_t index);
	const Note& operator[](size_t index) const;

	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;

	template <typename Pred>
	void sort(const Pred& pred);

private:
	using NoteMap = std::unordered_map<uint64_t, Note>;
	using OrderList = std::vector<uint64_t>;

	NoteMap m_notes;
	OrderList m_order;
	IdFactory m_ids;
};

class NoteSet::iterator : public std::iterator<std::random_access_iterator_tag, Note>
{
public:
	iterator();

	reference operator*() const;
	pointer operator->() const;
	reference operator[](difference_type offset) const;

	bool operator==(const iterator& other) const;
	bool operator!=(const iterator& other) const;
	bool operator<(const iterator& other) const;
	bool operator<=(const iterator& other) const;
	bool operator>(const iterator& other) const;
	bool operator>=(const iterator& other) const;

	iterator& operator++();
	iterator operator++(int post);
	iterator& operator--();
	iterator operator--(int post);

	difference_type operator-(const iterator& other) const;

	iterator operator+(difference_type offset) const;
	iterator operator-(difference_type offset) const;

	iterator& operator+=(difference_type offset);
	iterator& operator-=(difference_type offset);

private:
	friend class NoteSet;
	friend class const_iterator;

	iterator(NoteSet& notes, OrderList::iterator iter);
	void UpdateCurNote();

	NoteSet* m_notes;
	OrderList::iterator m_iter;
	Note* m_curNote;
};

class NoteSet::const_iterator : public std::iterator<std::random_access_iterator_tag, const Note>
{
public:
	const_iterator();
	const_iterator(const NoteSet::iterator& iter);

	reference operator*() const;
	pointer operator->() const;
	reference operator[](difference_type offset) const;

	bool operator==(const const_iterator& other) const;
	bool operator!=(const const_iterator& other) const;
	bool operator<(const const_iterator& other) const;
	bool operator<=(const const_iterator& other) const;
	bool operator>(const const_iterator& other) const;
	bool operator>=(const const_iterator& other) const;

	const_iterator& operator++();
	const_iterator operator++(int post);
	const_iterator& operator--();
	const_iterator operator--(int post);

	difference_type operator-(const const_iterator& other) const;

	const_iterator operator+(difference_type offset) const;
	const_iterator operator-(difference_type offset) const;

	const_iterator& operator+=(difference_type offset);
	const_iterator& operator-=(difference_type offset);

private:
	friend class NoteSet;

	const_iterator(const NoteSet& notes, OrderList::const_iterator iter);
	void UpdateCurNote();

	const NoteSet* m_notes;
	OrderList::const_iterator m_iter;
	const Note* m_curNote;
};

template <typename Pred>
void NoteSet::sort(const Pred& pred)
{
	std::sort(m_order.begin(), m_order.end(), [this, &pred] (uint64_t left, uint64_t right) -> bool
		{
			return pred(m_notes.find(left)->second, m_notes.find(right)->second);
		});
}

inline NoteSet::iterator::iterator()
	: m_notes(nullptr), m_curNote(nullptr)
{
}

inline NoteSet::iterator::reference NoteSet::iterator::operator*() const
{
	return *m_curNote;
}

inline NoteSet::iterator::pointer NoteSet::iterator::operator->() const
{
	return m_curNote;
}

inline NoteSet::iterator::reference NoteSet::iterator::operator[](difference_type offset) const
{
	return *(*this + offset);
}

inline bool NoteSet::iterator::operator==(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes == other.m_notes;
	return m_iter == other.m_iter;
}

inline bool NoteSet::iterator::operator!=(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes != other.m_notes;
	return m_iter != other.m_iter;
}

inline bool NoteSet::iterator::operator<(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes < other.m_notes;
	return m_iter < other.m_iter;
}

inline bool NoteSet::iterator::operator<=(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes <= other.m_notes;
	return m_iter <= other.m_iter;
}

inline bool NoteSet::iterator::operator>(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes > other.m_notes;
	return m_iter > other.m_iter;
}

inline bool NoteSet::iterator::operator>=(const iterator& other) const
{
	if (!m_notes || !other.m_notes)
		return m_notes >= other.m_notes;
	return m_iter >= other.m_iter;
}

inline NoteSet::iterator& NoteSet::iterator::operator++()
{
	++m_iter;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator NoteSet::iterator::operator++(int post)
{
	iterator copy(*this);
	++*this;
	return copy;
}

inline NoteSet::iterator& NoteSet::iterator::operator--()
{
	--m_iter;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator NoteSet::iterator::operator--(int post)
{
	iterator copy(*this);
	--*this;
	return *this;
}

inline NoteSet::iterator::difference_type NoteSet::iterator::operator-(const iterator& other) const
{
	return m_iter - other.m_iter;
}

inline NoteSet::iterator NoteSet::iterator::operator+(difference_type offset) const
{
	iterator copy(*this);
	copy += offset;
	return copy;
}

inline NoteSet::iterator NoteSet::iterator::operator-(difference_type offset) const
{
	iterator copy(*this);
	copy -= offset;
	return copy;
}

inline NoteSet::iterator& NoteSet::iterator::operator+=(difference_type offset)
{
	m_iter += offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator& NoteSet::iterator::operator-=(difference_type offset)
{
	m_iter -= offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator::iterator(NoteSet& notes, OrderList::iterator iter)
	: m_notes(&notes), m_iter(iter)
{
	UpdateCurNote();
}

inline void NoteSet::iterator::UpdateCurNote()
{
	if (m_iter == m_notes->m_order.end())
		m_curNote = nullptr;
	else
		m_curNote = &m_notes->m_notes.find(*m_iter)->second;
}

inline NoteSet::const_iterator::const_iterator()
	: m_notes(nullptr), m_curNote(nullptr)
{
}

inline NoteSet::const_iterator::const_iterator(const NoteSet::iterator& iter)
	: m_notes(iter.m_notes), m_iter(iter.m_iter), m_curNote(iter.m_curNote)
{
}

inline NoteSet::const_iterator::reference NoteSet::const_iterator::operator*() const
{
	return *m_curNote;
}

inline NoteSet::const_iterator::pointer NoteSet::const_iterator::operator->() const
{
	return m_curNote;
}

inline NoteSet::const_iterator::reference NoteSet::const_iterator::operator[](
	difference_type offset) const
{
	return *(*this + offset);
}

inline bool NoteSet::const_iterator::operator==(const const_iterator& other) const
{
	return m_iter == other.m_iter;
}

inline bool NoteSet::const_iterator::operator!=(const const_iterator& other) const
{
	return m_iter != other.m_iter;
}

inline bool NoteSet::const_iterator::operator<(const const_iterator& other) const
{
	return m_iter < other.m_iter;
}

inline bool NoteSet::const_iterator::operator<=(const const_iterator& other) const
{
	return m_iter <= other.m_iter;
}

inline bool NoteSet::const_iterator::operator>(const const_iterator& other) const
{
	return m_iter > other.m_iter;
}

inline bool NoteSet::const_iterator::operator>=(const const_iterator& other) const
{
	return m_iter >= other.m_iter;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator++()
{
	++m_iter;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator NoteSet::const_iterator::operator++(int post)
{
	const_iterator copy(*this);
	++*this;
	return copy;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator--()
{
	--m_iter;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator NoteSet::const_iterator::operator--(int post)
{
	const_iterator copy(*this);
	--*this;
	return *this;
}

inline NoteSet::const_iterator::difference_type NoteSet::const_iterator::operator-(
	const const_iterator& other) const
{
	return m_iter - other.m_iter;
}

inline NoteSet::const_iterator NoteSet::const_iterator::operator+(difference_type offset) const
{
	const_iterator copy(*this);
	copy += offset;
	return copy;
}

inline NoteSet::const_iterator NoteSet::const_iterator::operator-(difference_type offset) const
{
	const_iterator copy(*this);
	copy -= offset;
	return copy;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator+=(difference_type offset)
{
	m_iter += offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator-=(difference_type offset)
{
	m_iter -= offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator::const_iterator(const NoteSet& notes, OrderList::const_iterator iter)
	: m_notes(&notes), m_iter(iter)
{
	UpdateCurNote();
}

inline void NoteSet::const_iterator::UpdateCurNote()
{
	if (m_iter == m_notes->m_order.end())
		m_curNote = nullptr;
	else
		m_curNote = &m_notes->m_notes.find(*m_iter)->second;
}

} // namespace NoteVault
