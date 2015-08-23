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

import android.content.Context;
import android.content.Intent;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.akb.notevault.dialogs.AddDialog;
import com.akb.notevault.dialogs.ErrorDialog;
import com.akb.notevault.dialogs.OnDialogAcceptedListener;
import com.akb.notevault.dialogs.OpenDialog;
import com.akb.notevault.dialogs.RemoveDialog;
import com.akb.notevault.dialogs.RenameDialog;

import java.util.Comparator;


public class NoteListSelection extends AppCompatActivity
{

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_note_list_selection);
		setTitle(R.string.title_note_lists);

		m_noteListsAdapter = new CustomAdapter(getApplicationContext(), R.layout.list_item,
			R.id.itemText);
		m_noteListsView = (ListView)getWindow().getDecorView().findViewById(R.id.noteLists);
		m_noteListsView.setAdapter(m_noteListsAdapter);
		populateNoteLists();
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

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo)
	{
		TextView textView = (TextView)v;
		menu.add(R.string.menu_item_rename).setOnMenuItemClickListener(new RenameListener(textView));
		menu.add(R.string.menu_item_remove).setOnMenuItemClickListener(new RemoveListener(textView));
	}

	public void addNoteList(View view)
	{
		AddDialog addDialog = new AddDialog();
		addDialog.setOnDialogAcceptedListener(new AddNoteListener());
		addDialog.show(getSupportFragmentManager(), "add note list");
	}

	private void populateNoteLists()
	{
	}

	private void sortNoteLists()
	{
		m_noteListsAdapter.sort(new ItemCompare());
	}

	private boolean canHaveNoteList(String name)
	{
		for (int i = 0; i < m_noteListsAdapter.getCount(); ++i)
		{
			if (m_noteListsAdapter.getItem(i).getName().equals(name))
				return false;
		}

		return true;
	}

	private class ListItem
	{
		public ListItem(String name) { m_name = name; }
		public String getName() { return m_name; }
		public void setName(String name) { m_name = name; }

		@Override
		public String toString()
		{
			return m_name;
		}

		private String m_name;
	}

	private class ItemCompare implements Comparator<ListItem>
	{
		public int compare(ListItem left, ListItem right)
		{
			return left.getName().compareToIgnoreCase(right.getName());
		}
	}

	private class CustomAdapter extends ArrayAdapter<ListItem>
	{
		public CustomAdapter(Context context, int resource, int textViewResourceId)
		{
			super(context, resource, textViewResourceId);
			m_textViewResourceId = textViewResourceId;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent)
		{
			View view = super.getView(position, convertView, parent);
			TextView textView = (TextView)view.findViewById(m_textViewResourceId);
			textView.setTag(getItem(position));
			textView.setOnClickListener(new OpenListener(textView));
			registerForContextMenu(textView);
			return view;
		}

		private int m_textViewResourceId;
	}

	private class OpenListener implements View.OnClickListener
	{
		public OpenListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public void onClick(View view)
		{
			OpenDialog openDialog = new OpenDialog();
			openDialog.setName(m_noteListView.getText().toString());
			openDialog.setOnDialogAcceptedListener(new OpenNoteListener());
			openDialog.show(getSupportFragmentManager(), "open note list");
		}

		private TextView m_noteListView;
	}

	private class OpenNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			return true;
		}
	}

	private class AddNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			String name = ((AddDialog)dialog).getName();
			if (!canHaveNoteList(name))
			{
				String message = getString(R.string.error_same_name).replace("%s", name);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			m_noteListsAdapter.add(new ListItem(name));
			sortNoteLists();
			return true;
		}
	}

	private class RenameListener implements MenuItem.OnMenuItemClickListener
	{
		public RenameListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public boolean onMenuItemClick(MenuItem item)
		{
			RenameDialog renameDialog = new RenameDialog();
			renameDialog.setInitialName(m_noteListView.getText().toString());
			renameDialog.setOnDialogAcceptedListener(new RenameNoteListener(m_noteListView));
			renameDialog.show(getSupportFragmentManager(), "rename note list");
			return true;
		}

		private TextView m_noteListView;
	}

	private class RemoveListener implements MenuItem.OnMenuItemClickListener
	{
		public RemoveListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public boolean onMenuItemClick(MenuItem item)
		{
			RemoveDialog removeDialog = new RemoveDialog();
			removeDialog.setName(m_noteListView.getText().toString());
			removeDialog.setOnDialogAcceptedListener(new RemoveNoteListener(m_noteListView));
			removeDialog.show(getSupportFragmentManager(), "remove note list");
			return true;
		}

		private TextView m_noteListView;
	}

	private class RenameNoteListener implements OnDialogAcceptedListener
	{
		public RenameNoteListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			String newName = ((RenameDialog)dialog).getNewName();
			if (!canHaveNoteList(newName))
			{
				String message = getString(R.string.error_same_name).replace("%s", newName);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			ListItem item = (ListItem)m_noteListView.getTag();
			item.setName(newName);
			m_noteListView.setText(newName);
			sortNoteLists();
			return true;
		}

		private TextView m_noteListView;
	}

	private class RemoveNoteListener implements OnDialogAcceptedListener
	{
		public RemoveNoteListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			m_noteListsAdapter.remove((ListItem) m_noteListView.getTag());
			return true;
		}

		private TextView m_noteListView;
	}


	private ListView m_noteListsView;
	private CustomAdapter m_noteListsAdapter;
}
