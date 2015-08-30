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

package com.akb.notevault.notes;

public class Note
{
	public Note(long id)
	{
		m_id = id;
		m_title = "";
		m_message = "";
	}

	public Note(Note other)
	{
		m_id = other.m_id;
		m_title = other.m_title;
		m_message = other.m_message;
	}

	public long getId()	{return m_id;}
	public void setId(long id)	{m_id = id;}

	public String getTitle()	{return m_title;}
	public void setTitle(String title)	{m_title = title;}

	public String getMessage()	{return m_message;}
	public void setMessage(String message)	{m_message = message;}

	private long m_id;
	private String m_title;
	private String m_message;
}
