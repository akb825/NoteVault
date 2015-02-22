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
	int erase(const Note& note)	{return erase(note.GetId());}
	iterator erase(const iterator& iter);

	iterator find(uint64_t id);
	const_iterator find(uint64_t id) const;

	Note* find_note(uint64_t id);
	const Note* find_note(uint64_t id) const;

	size_t size() const	{return m_Notes.size();}
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

	NoteMap m_Notes;
	OrderList m_Order;
	IdFactory m_Ids;
};

class NoteSet::iterator : public std::iterator<std::random_access_iterator_tag, Note>
{
public:
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

	NoteSet* m_Notes;
	OrderList::iterator m_Iter;
	Note* m_CurNote;
};

class NoteSet::const_iterator : public std::iterator<std::random_access_iterator_tag, const Note>
{
public:
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

	const NoteSet* m_Notes;
	OrderList::const_iterator m_Iter;
	const Note* m_CurNote;
};

template <typename Pred>
void NoteSet::sort(const Pred& pred)
{
	std::sort(m_Order.begin(), m_Order.end(), [this, &pred] (size_t left, size_t right) -> bool
		{
			return pred(m_Notes.find(left)->second, m_Notes.find(right)->second);
		});
}

inline NoteSet::iterator::reference NoteSet::iterator::operator*() const
{
	return *m_CurNote;
}

inline NoteSet::iterator::pointer NoteSet::iterator::operator->() const
{
	return m_CurNote;
}

inline NoteSet::iterator::reference NoteSet::iterator::operator[](difference_type offset) const
{
	return *(*this + offset);
}

inline bool NoteSet::iterator::operator==(const iterator& other) const
{
	return m_Iter == other.m_Iter;
}

inline bool NoteSet::iterator::operator!=(const iterator& other) const
{
	return m_Iter != other.m_Iter;
}

inline bool NoteSet::iterator::operator<(const iterator& other) const
{
	return m_Iter < other.m_Iter;
}

inline bool NoteSet::iterator::operator<=(const iterator& other) const
{
	return m_Iter <= other.m_Iter;
}

inline bool NoteSet::iterator::operator>(const iterator& other) const
{
	return m_Iter > other.m_Iter;
}

inline bool NoteSet::iterator::operator>=(const iterator& other) const
{
	return m_Iter >= other.m_Iter;
}

inline NoteSet::iterator& NoteSet::iterator::operator++()
{
	++m_Iter;
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
	--m_Iter;
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
	return m_Iter - other.m_Iter;
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
	m_Iter += offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator& NoteSet::iterator::operator-=(difference_type offset)
{
	m_Iter -= offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::iterator::iterator(NoteSet& notes, OrderList::iterator iter)
	: m_Notes(&notes), m_Iter(iter)
{
	UpdateCurNote();
}

inline void NoteSet::iterator::UpdateCurNote()
{
	if (m_Iter == m_Notes->m_Order.end())
		m_CurNote = nullptr;
	else
		m_CurNote = &m_Notes->m_Notes.find(*m_Iter)->second;
}

inline NoteSet::const_iterator::const_iterator(const NoteSet::iterator& iter)
	: m_Notes(iter.m_Notes), m_Iter(iter.m_Iter), m_CurNote(iter.m_CurNote)
{
}

inline NoteSet::const_iterator::reference NoteSet::const_iterator::operator*() const
{
	return *m_CurNote;
}

inline NoteSet::const_iterator::pointer NoteSet::const_iterator::operator->() const
{
	return m_CurNote;
}

inline NoteSet::const_iterator::reference NoteSet::const_iterator::operator[](
	difference_type offset) const
{
	return *(*this + offset);
}

inline bool NoteSet::const_iterator::operator==(const const_iterator& other) const
{
	return m_Iter == other.m_Iter;
}

inline bool NoteSet::const_iterator::operator!=(const const_iterator& other) const
{
	return m_Iter != other.m_Iter;
}

inline bool NoteSet::const_iterator::operator<(const const_iterator& other) const
{
	return m_Iter < other.m_Iter;
}

inline bool NoteSet::const_iterator::operator<=(const const_iterator& other) const
{
	return m_Iter <= other.m_Iter;
}

inline bool NoteSet::const_iterator::operator>(const const_iterator& other) const
{
	return m_Iter > other.m_Iter;
}

inline bool NoteSet::const_iterator::operator>=(const const_iterator& other) const
{
	return m_Iter >= other.m_Iter;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator++()
{
	++m_Iter;
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
	--m_Iter;
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
	return m_Iter - other.m_Iter;
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
	m_Iter += offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator& NoteSet::const_iterator::operator-=(difference_type offset)
{
	m_Iter -= offset;
	UpdateCurNote();
	return *this;
}

inline NoteSet::const_iterator::const_iterator(const NoteSet& notes, OrderList::const_iterator iter)
	: m_Notes(&notes), m_Iter(iter)
{
	UpdateCurNote();
}

inline void NoteSet::const_iterator::UpdateCurNote()
{
	if (m_Iter == m_Notes->m_Order.end())
		m_CurNote = nullptr;
	else
		m_CurNote = &m_Notes->m_Notes.find(*m_Iter)->second;
}

} // namespace NoteVault
