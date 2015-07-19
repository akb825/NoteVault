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

import java.util.HashSet;

public class IdFactory
{
	public IdFactory()
	{
		m_maxId = 0;
		m_ids = new HashSet<>();
	}

	public boolean addId(long id)
	{
		if (!m_ids.add(id))
			return false;
		m_maxId = Math.max(m_maxId, id + 1);
		return true;
	}

	public long addId()
	{
		long newId = m_maxId++;
		m_ids.add(newId);
		return newId;
	}

	public boolean removeId(long id)
	{
		return m_ids.remove(id);
	}

	public void clear()
	{
		m_ids.clear();
	}

	private long m_maxId;
	private HashSet<Long> m_ids;
}
