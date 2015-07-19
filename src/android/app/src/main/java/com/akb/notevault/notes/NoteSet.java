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

package com.akb.notevault.notes;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;

public class NoteSet
{
	public class Iterator implements java.util.Iterator<Note>
	{
		Iterator()
		{
			m_orderIter = m_order.iterator();
			m_lastId = -1;
		}

		public boolean hasNext()
		{
			return m_orderIter.hasNext();
		}

		public Note next()
		{
			m_lastId = m_orderIter.next();
			return m_notes.get(m_lastId);
		}

		public void remove()
		{
			if (m_lastId < 0)
				throw new IllegalStateException();

			m_ids.removeId(m_lastId);
			m_notes.remove(m_lastId);
			m_orderIter.remove();
			m_lastId = -1;
		}

		java.util.Iterator<Long> m_orderIter;
		long m_lastId;
	}

	public NoteSet()
	{
		m_notes = new HashMap<>();
		m_order = new ArrayList<>();
		m_ids = new IdFactory();
	}

	public Note add()
	{
		long id = m_ids.addId();
		Note note = new Note(id);
		m_notes.put(id, note);
		m_order.add(id);
		return note;
	}

	public boolean add(Note note)
	{
		if (!m_ids.addId(note.getId()))
			return false;

		m_notes.put(note.getId(), note);
		m_order.add(note.getId());
		return true;
	}

	public Note add(int index)
	{
		long id = m_ids.addId();
		Note note = new Note(id);
		m_notes.put(id, note);
		m_order.add(index, id);
		return note;
	}

	public boolean add(int index, Note note)
	{
		if (!m_ids.addId(note.getId()))
			return false;

		m_notes.put(note.getId(), note);
		m_order.add(index, note.getId());
		return true;
	}

	public boolean remove(long id)
	{
		if (!m_ids.removeId(id))
			return false;

		m_notes.remove(id);
		m_order.remove(new Long(id));
		return true;
	}

	public boolean remove(Note note)
	{
		return remove(note.getId());
	}

	public Note find(long id)
	{
		return m_notes.get(id);
	}

	public Note get(int index)
	{
		return m_notes.get(m_order.get(index));
	}

	public int indexOf(long id)
	{
		return m_order.indexOf(id);
	}

	public int indexOf(Note note)
	{
		return indexOf(note.getId());
	}

	public int size()
	{
		return m_notes.size();
	}

	public void clear()
	{
		m_notes.clear();
		m_order.clear();
		m_ids.clear();
	}

	public Iterator iterator()
	{
		return new Iterator();
	}

	public void sort(final Comparator<Note> comparator)
	{
		Collections.sort(m_order,
			new Comparator<Long>()
			{
				@Override
				public int compare(Long lhs, Long rhs)
				{
					return comparator.compare(m_notes.get(lhs), m_notes.get(rhs));
				}
			});
	}

	private HashMap<Long, Note> m_notes;
	private ArrayList<Long> m_order;
	private IdFactory m_ids;
}
