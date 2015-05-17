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

package com.akb.notevault;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import java.util.Comparator;


public class NoteListSelection extends AppCompatActivity implements AdapterView.OnItemSelectedListener
{

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_note_list_selection);
		setTitle(R.string.title_note_lists);

		m_noteListsAdapter = new ArrayAdapter(getApplicationContext(), R.layout.list_item);
		m_noteListsView = (ListView)getWindow().getDecorView().findViewById(R.id.noteLists);
		m_noteListsView.setOnItemSelectedListener(this);
		m_noteListsView.setAdapter(m_noteListsAdapter);
		populateNoteLists();

		m_openButton = (Button)getWindow().getDecorView().findViewById(R.id.openNoteListButton);
		m_removeButton = (Button)getWindow().getDecorView().findViewById(R.id.removeNoteListButton);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.menu_note_vault, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();

		//noinspection SimplifiableIfStatement
		if (id == R.id.action_settings)
		{
			Intent intent = new Intent(this, HelpScreen.class);
			startActivity(intent);
			return true;
		}

		return super.onOptionsItemSelected(item);
	}

	public void addNoteList(View view)
	{
		m_noteListsAdapter.add("New Note");
		int selectedIndex = m_noteListsAdapter.getCount() - 1;
		m_noteListsView.setSelection(selectedIndex);
		m_selectedNoteList = (TextView)m_noteListsAdapter.getView(selectedIndex, null,
			m_noteListsView).findViewById(R.id.itemText);
		System.out.printf("%d: %b\n", selectedIndex, m_selectedNoteList != null);
		sortNoteLists();
		m_selectedNoteList.beginBatchEdit();
	}

	public void removeNoteList(View view)
	{
	}

	public void openNoteList(View view)
	{
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
	{
		System.out.printf("here\n");
		m_selectedNoteList = (TextView)view.findViewById(R.id.itemText);
		m_removeButton.setEnabled(true);
		m_openButton.setEnabled(true);
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent)
	{
		m_selectedNoteList = null;
		m_removeButton.setEnabled(false);
		m_openButton.setEnabled(false);
	}

	private void populateNoteLists()
	{
	}

	private void sortNoteLists()
	{
		m_noteListsAdapter.sort(new StringCompare());
	}

	private class StringCompare implements Comparator<String>
	{
		public int compare(String left, String right)
		{
			return left.compareToIgnoreCase(right);
		}
	}

	private ListView m_noteListsView;
	private Button m_openButton;
	private Button m_removeButton;
	private TextView m_selectedNoteList;
	private ArrayAdapter<String> m_noteListsAdapter;
}
