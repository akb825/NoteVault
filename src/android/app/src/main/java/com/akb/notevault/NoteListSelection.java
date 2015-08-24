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
import android.os.Environment;
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

import com.akb.notevault.dialogs.AddNoteListDialog;
import com.akb.notevault.dialogs.ErrorDialog;
import com.akb.notevault.dialogs.OnDialogAcceptedListener;
import com.akb.notevault.dialogs.OpenNoteListDialog;
import com.akb.notevault.dialogs.RemoveNoteListDialog;
import com.akb.notevault.dialogs.RenameNoteListDialog;
import com.akb.notevault.io.Crypto;
import com.akb.notevault.io.NoteFile;
import com.akb.notevault.notes.NoteSet;

import java.io.File;
import java.io.FilenameFilter;
import java.util.Comparator;

import javax.crypto.SecretKey;


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
		ListView noteListsView = (ListView)getWindow().getDecorView().findViewById(R.id.noteLists);
		noteListsView.setAdapter(m_noteListsAdapter);
		populateNoteLists();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.menu_note_list_selection, menu);
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
		if (id == R.id.action_refresh)
		{
			populateNoteLists();
			return true;
		}
		else if (id == R.id.action_help)
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

	@Override
	public void onResume()
	{
		super.onResume();
		populateNoteLists();
	}

	public void addNoteList(View view)
	{
		AddNoteListDialog addDialog = new AddNoteListDialog();
		addDialog.setOnDialogAcceptedListener(new AddNoteListener());
		addDialog.show(getSupportFragmentManager(), "add note list");
	}

	private void populateNoteLists()
	{
		m_noteListsAdapter.clear();

		File[] files = getNoteListFiles();
		if (files == null)
			return;

		for (File file : files)
		{
			String name = file.getName();
			name = name.substring(0, name.length() - NoteFile.cExtension.length());
			m_noteListsAdapter.add(new ListItem(name));
		}

		sortNoteLists();
	}

	private boolean canAccessNoteLists()
	{
		String state = Environment.getExternalStorageState();
		return state.equals(Environment.MEDIA_MOUNTED);
	}

	private File getNoteListsRoot()
	{
		if (!canAccessNoteLists())
		{
			ErrorDialog.show(this, R.string.error_inaccessible);
			return null;
		}

		File root = getBaseContext().getExternalFilesDir(null);
		if (root == null)
		{
			ErrorDialog.show(this, R.string.error_inaccessible);
			return null;
		}

		return root;
	}

	private File[] getNoteListFiles()
	{
		final File root = getNoteListsRoot();
		if (root == null)
			return null;

		return root.listFiles(new FilenameFilter()
		{
			@Override
			public boolean accept(File dir, String filename)
			{
				return dir.equals(root) && filename.endsWith(NoteFile.cExtension);
			}
		});
	}

	private void sortNoteLists()
	{
		m_noteListsAdapter.sort(new ItemCompare());
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
			OpenNoteListDialog openDialog = new OpenNoteListDialog();
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
			OpenNoteListDialog openDialog = (OpenNoteListDialog)dialog;
			String name = openDialog.getName();
			String password = openDialog.getPassword();

			File rootDir = getNoteListsRoot();
			if (rootDir == null)
				return false;

			// Get the file for the new note list
			File file = new File(rootDir, name + NoteFile.cExtension);
			NoteFile.LoadResult loadResult = NoteFile.loadNotes(file, password);
			switch (loadResult.result)
			{
				case Success:
					break;
				case EncryptionError:
					ErrorDialog.show(NoteListSelection.this, R.string.error_bad_password);
					return false;
				default:
				{
					String message = getString(R.string.error_load).replace("%s", name);
					ErrorDialog.show(NoteListSelection.this, message);
					return true;
				}
			}

			// Bring up the note list.
			Intent intent = new Intent(NoteListSelection.this, NoteList.class);
			intent.putExtra("FilePath", file);
			intent.putExtra("Key", loadResult.key.getEncoded());
			startActivity(intent);

			return true;
		}
	}

	private class AddNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			AddNoteListDialog addDialog = (AddNoteListDialog)dialog;
			String name = addDialog.getName();
			String password = addDialog.getPassword();

			File rootDir = getNoteListsRoot();
			if (rootDir == null)
				return false;

			// Get the file for the new note list
			File newFile = new File(rootDir, name + NoteFile.cExtension);
			if (newFile.exists())
			{
				String message = getString(R.string.error_same_name).replace("%s", name);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			// Save an empty note list.
			byte[] salt = Crypto.random(Crypto.cSaltLenBytes);
			SecretKey key = Crypto.generateKey(password, salt, Crypto.cDefaultKeyIterations);
			NoteFile.Result result = NoteFile.writeNotes(newFile, new NoteSet(), salt, key);

			if (result != NoteFile.Result.Success)
			{
				newFile.delete();
				String message = getString(R.string.error_create).replace("%s", name);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			// Update the notes.
			populateNoteLists();

			// Bring up the note list.
			Intent intent = new Intent(NoteListSelection.this, NoteList.class);
			intent.putExtra("FilePath", newFile);
			intent.putExtra("Key", key.getEncoded());
			startActivity(intent);

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
			RenameNoteListDialog renameDialog = new RenameNoteListDialog();
			renameDialog.setInitialName(m_noteListView.getText().toString());
			renameDialog.setOnDialogAcceptedListener(new RenameNoteListener());
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
			RemoveNoteListDialog removeDialog = new RemoveNoteListDialog();
			removeDialog.setName(m_noteListView.getText().toString());
			removeDialog.setOnDialogAcceptedListener(new RemoveNoteListener());
			removeDialog.show(getSupportFragmentManager(), "remove note list");
			return true;
		}

		private TextView m_noteListView;
	}

	private class RenameNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			RenameNoteListDialog renameDialog = (RenameNoteListDialog)dialog;
			String oldName = renameDialog.getInitialName();
			String newName = renameDialog.getNewName();
			String password = renameDialog.getPassword();

			File rootDir = getNoteListsRoot();
			if (rootDir == null)
				return false;

			// Ensure we can load the note list.
			File oldFile = new File(rootDir, oldName + NoteFile.cExtension);
			File newFile = new File(rootDir, newName + NoteFile.cExtension);

			NoteFile.LoadResult loadResult = NoteFile.loadNotes(oldFile, password);
			switch (loadResult.result)
			{
				case Success:
					break;
				case EncryptionError:
					ErrorDialog.show(NoteListSelection.this, R.string.error_bad_password);
					return false;
				default:
				{
					String message = getString(R.string.error_create).replace("%s", oldName);
					ErrorDialog.show(NoteListSelection.this, message);
					return false;
				}
			}

			if (newFile.exists())
			{
				String message = getString(R.string.error_same_name).replace("%s", newName);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			if (!oldFile.renameTo(newFile))
			{
				String message = getString(R.string.error_create).replace("%s", newName);
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			populateNoteLists();
			return true;
		}
	}

	private class RemoveNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			RemoveNoteListDialog removeDialog = (RemoveNoteListDialog)dialog;
			String name = removeDialog.getName();
			String password = removeDialog.getPassword();

			File rootDir = getNoteListsRoot();
			if (rootDir == null)
				return false;

			// Ensure we can load the note list.
			File file = new File(rootDir, name + NoteFile.cExtension);
			NoteFile.LoadResult loadResult = NoteFile.loadNotes(file, password);
			String message = getString(R.string.error_remove).replace("%s", name);
			switch (loadResult.result)
			{
				case Success:
					break;
				case EncryptionError:
					ErrorDialog.show(NoteListSelection.this, R.string.error_bad_password);
					return false;
				default:
					ErrorDialog.show(NoteListSelection.this, message);
					return false;
			}

			if (!file.delete())
			{
				ErrorDialog.show(NoteListSelection.this, message);
				return false;
			}

			populateNoteLists();
			return true;
		}
	}


	private CustomAdapter m_noteListsAdapter;
}
