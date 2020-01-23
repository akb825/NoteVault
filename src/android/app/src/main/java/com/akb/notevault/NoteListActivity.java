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

package com.akb.notevault;

import android.content.Context;
import android.content.Intent;
import androidx.fragment.app.DialogFragment;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.akb.notevault.dialogs.AddNoteDialog;
import com.akb.notevault.dialogs.ErrorDialog;
import com.akb.notevault.dialogs.OnDialogAcceptedListener;
import com.akb.notevault.dialogs.RemoveNoteDialog;
import com.akb.notevault.dialogs.RenameNoteDialog;
import com.akb.notevault.io.NoteFile;
import com.akb.notevault.notes.Note;
import com.akb.notevault.notes.NoteSet;

import java.io.File;
import java.util.Comparator;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

public class NoteListActivity extends AppCompatActivity
{

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_note_list);

		Intent intent = getIntent();
		m_file = (File)intent.getSerializableExtra("FilePath");
		m_name = m_file.getName();
		m_name = m_name.substring(0, m_name.length() - NoteFile.cExtension.length());

		byte[] encodedKey = (byte[])intent.getSerializableExtra("Key");
		m_key = new SecretKeySpec(encodedKey, 0, encodedKey.length, "AES");

		m_noteListAdapter = new CustomAdapter(getApplicationContext(), R.layout.list_item,
			R.id.itemText);
		ListView noteView = (ListView)getWindow().getDecorView().findViewById(R.id.notes);
		noteView.setAdapter(m_noteListAdapter);
		reloadNotes();

		setTitle(m_name);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.menu_note_list, menu);
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
		if (id == R.id.action_help)
		{
			Intent intent = new Intent(this, HelpActivity.class);
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

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if (resultCode == RESULT_CANCELED)
			return;

		long id = data.getLongExtra("Id", -1);
		String message = data.getStringExtra("Message");

		m_notes.find(id).setMessage(message);
		saveNotes();
	}

	public void addNote(View view)
	{
		AddNoteDialog addDialog = new AddNoteDialog();
		addDialog.setOnDialogAcceptedListener(new AddNoteListener());
		addDialog.show(getSupportFragmentManager(), "add note");
	}

	public void close(View view)
	{
		finish();
	}

	private void reloadNotes()
	{
		NoteFile.LoadResult loadResult = NoteFile.loadNotes(m_file, m_key);
		if (loadResult.result != NoteFile.Result.Success)
		{
			ErrorDialog.show(this, getString(R.string.error_load).replace("%s", m_name));
			finish();
			return;
		}
		m_notes = loadResult.notes;
		m_salt = loadResult.salt;

		repopulateList();
	}

	private void repopulateList()
	{
		sortNotes();
		m_noteListAdapter.clear();
		NoteSet.Iterator iterator = m_notes.iterator();
		while (iterator.hasNext())
			m_noteListAdapter.add(new ListItem(iterator.next()));
	}

	void sortNotes()
	{
		m_notes.sort(new NoteCompare());
	}

	private void saveNotes()
	{
		if (NoteFile.saveNotes(m_file, m_notes, m_salt, m_key) != NoteFile.Result.Success)
		{
			ErrorDialog.show(this, getString(R.string.error_save).replace("%s", m_name));
			finish();
		}
	}

	private void openNote(Note note)
	{
		Intent intent = new Intent(NoteListActivity.this, NoteActivity.class);
		intent.putExtra("Id", note.getId());
		intent.putExtra("Title", note.getTitle());
		intent.putExtra("Message", note.getMessage());
		startActivityForResult(intent, 0);
	}

	private class NoteCompare implements Comparator<Note>
	{
		public int compare(Note left, Note right)
		{
			return left.getTitle().compareToIgnoreCase(right.getTitle());
		}
	}

	private class ListItem
	{
		public ListItem(Note note)
		{
			m_name = note.getTitle();
			m_id = note.getId();
		}

		public String getName() { return m_name; }
		public void setName(String name) { m_name = name; }

		public long getId() { return m_id; }
		public void setId(long id) { m_id = id; }

		@Override
		public String toString()
		{
			return m_name;
		}

		private String m_name;
		private long m_id;
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
			openNote(m_notes.find(((ListItem) m_noteListView.getTag()).getId()));
		}

		private TextView m_noteListView;
	}

	private class AddNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			AddNoteDialog addDialog = (AddNoteDialog)dialog;
			String name = addDialog.getName();

			Note note = m_notes.add();
			note.setTitle(name);

			repopulateList();
			saveNotes();

			openNote(note);
			return true;
		}
	}

	private class RenameListener implements MenuItem.OnMenuItemClickListener
	{
		public RenameListener(TextView noteView)
		{
			m_noteView = noteView;
		}

		@Override
		public boolean onMenuItemClick(MenuItem item)
		{
			RenameNoteDialog renameDialog = new RenameNoteDialog();
			renameDialog.setInitialName(m_noteView.getText().toString());
			renameDialog.setOnDialogAcceptedListener(new RenameNoteListener(m_noteView));
			renameDialog.show(getSupportFragmentManager(), "rename note");
			return true;
		}

		private TextView m_noteView;
	}

	private class RenameNoteListener implements OnDialogAcceptedListener
	{
		public RenameNoteListener(TextView noteView)
		{
			m_noteView = noteView;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			RenameNoteDialog renameDialog = (RenameNoteDialog)dialog;
			String newName = renameDialog.getNewName();

			ListItem item = (ListItem)m_noteView.getTag();
			Note note = m_notes.find(item.getId());
			note.setTitle(newName);

			repopulateList();
			saveNotes();
			return true;
		}

		private TextView m_noteView;
	}

	private class RemoveListener implements MenuItem.OnMenuItemClickListener
	{
		public RemoveListener(TextView noteView)
		{
			m_noteView = noteView;
		}

		@Override
		public boolean onMenuItemClick(MenuItem item)
		{
			RemoveNoteDialog removeDialog = new RemoveNoteDialog();
			removeDialog.setName(m_noteView.getText().toString());
			removeDialog.setOnDialogAcceptedListener(new RemoveNoteListener(m_noteView));
			removeDialog.show(getSupportFragmentManager(), "remove note");
			return true;
		}

		private TextView m_noteView;
	}

	private class RemoveNoteListener implements OnDialogAcceptedListener
	{
		public RemoveNoteListener(TextView noteView)
		{
			m_noteView = noteView;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			ListItem item = (ListItem)m_noteView.getTag();
			m_notes.remove(item.getId());

			repopulateList();
			saveNotes();
			return true;
		}

		private TextView m_noteView;
	}

	String m_name;
	private File m_file;
	private SecretKey m_key;
	private byte[] m_salt;
	private NoteSet m_notes;
	private CustomAdapter m_noteListAdapter;
}
