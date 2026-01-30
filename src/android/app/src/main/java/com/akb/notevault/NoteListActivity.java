/*
 * Copyright 2015-2026 Aaron Barany
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

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.fragment.app.DialogFragment;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;

import android.os.Bundle;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ArrayAdapter;
import android.widget.Button;
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
		if (savedInstanceState != null)
		{
			m_lastDialogName = savedInstanceState.getString(LAST_DIALOG_NAME);
			m_lastDialogId = savedInstanceState.getLong(LAST_DIALOG_ID);
		}
		getSupportFragmentManager().setFragmentFactory(new DialogFactory());
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

		Window window = getWindow();
		View decorView = window.getDecorView();
		ListView noteView = decorView.findViewById(R.id.notes);
		noteView.setAdapter(m_noteListAdapter);
		reloadNotes();

		Button addNoteButton = decorView.findViewById(R.id.addNoteButton);
		addNoteButton.setOnClickListener(this::addNote);

		Button closeButton = decorView.findViewById(R.id.closeButton);
		closeButton.setOnClickListener(this::close);

		setTitle(m_name);

		m_noteActivityLauncher = registerForActivityResult(
			new ActivityResultContracts.StartActivityForResult(),
			new ActivityResultCallback<>()
			{
				@Override
				public void onActivityResult(ActivityResult result)
				{
					if (result.getResultCode() != RESULT_CANCELED && result.getData() != null)
						NoteListActivity.this.notesClosed(result.getData());
				}
			}
		);

		ViewCompat.setOnApplyWindowInsetsListener(
			decorView.findViewById(R.id.activityNoteList),
			new WindowInsetHandler());

		WindowInsetsControllerCompat insetsController =
			WindowCompat.getInsetsController(window, decorView);
		insetsController.setAppearanceLightStatusBars(true);
		insetsController.setAppearanceLightNavigationBars(true);
	}

	@Override
	public void onSaveInstanceState(Bundle bundle)
	{
		super.onSaveInstanceState(bundle);
		bundle.putString(LAST_DIALOG_NAME, m_lastDialogName);
		bundle.putLong(LAST_DIALOG_ID, m_lastDialogId);
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

		// noinspection SimplifiableIfStatement
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

	public void addNote(View view)
	{
		AddNoteDialog addDialog = createAddNoteDialog();
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

	private AddNoteDialog createAddNoteDialog()
	{
		return new AddNoteDialog(new AddNoteListener());
	}

	private RenameNoteDialog createRenameNoteDialog(String name, long itemId)
	{
		return new RenameNoteDialog(name, new RenameNoteListener(itemId));
	}

	private RemoveNoteDialog createRemoveNoteDialog(String name, long itemId)
	{
		return new RemoveNoteDialog(name, new RemoveNoteListener(itemId));
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
		m_noteActivityLauncher.launch(intent);
	}

	private void notesClosed(Intent data)
	{
		long id = data.getLongExtra("Id", -1);
		String message = data.getStringExtra("Message");

		m_notes.find(id).setMessage(message);
		saveNotes();
	}

	private class DialogFactory extends FragmentFactory
	{
		@Override
		public Fragment instantiate(ClassLoader classLoader, String className)
		{
			Class<? extends Fragment> clazz = loadFragmentClass(classLoader, className);
			if (clazz == AddNoteDialog.class)
				return createAddNoteDialog();
			if (clazz == RenameNoteDialog.class)
				return createRenameNoteDialog(m_lastDialogName, m_lastDialogId);
			if (clazz == RemoveNoteDialog.class)
				return createRemoveNoteDialog(m_lastDialogName, m_lastDialogId);
			return super.instantiate(classLoader, className);
		}
	}

	private static class NoteCompare implements Comparator<Note>
	{
		public int compare(Note left, Note right)
		{
			return left.getTitle().compareToIgnoreCase(right.getTitle());
		}
	}

	private static class ListItem
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
			TextView textView = view.findViewById(m_textViewResourceId);
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
			m_lastDialogName = m_noteView.getText().toString();
			m_lastDialogId = ((ListItem)m_noteView.getTag()).getId();
			RenameNoteDialog renameDialog =
				createRenameNoteDialog(m_lastDialogName, m_lastDialogId);
			renameDialog.show(getSupportFragmentManager(), "rename note");
			return true;
		}

		private TextView m_noteView;
	}

	private class RenameNoteListener implements OnDialogAcceptedListener
	{
		public RenameNoteListener(long itemId)
		{
			m_itemId = itemId;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			RenameNoteDialog renameDialog = (RenameNoteDialog)dialog;
			String newName = renameDialog.getNewName();

			Note note = m_notes.find(m_itemId);
			note.setTitle(newName);

			repopulateList();
			saveNotes();
			return true;
		}

		private long m_itemId;
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
			m_lastDialogName = m_noteView.getText().toString();
			m_lastDialogId = ((ListItem)m_noteView.getTag()).getId();
			RemoveNoteDialog removeDialog =
				createRemoveNoteDialog(m_lastDialogName, m_lastDialogId);
			removeDialog.show(getSupportFragmentManager(), "remove note");
			return true;
		}

		private TextView m_noteView;
	}

	private class RemoveNoteListener implements OnDialogAcceptedListener
	{
		public RemoveNoteListener(long itemId)
		{
			m_itemId = itemId;
		}

		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			m_notes.remove(m_itemId);

			repopulateList();
			saveNotes();
			return true;
		}

		private long m_itemId;
	}

	String m_name;
	private File m_file;
	private SecretKey m_key;
	private byte[] m_salt;
	private NoteSet m_notes;
	private CustomAdapter m_noteListAdapter;
	private ActivityResultLauncher<Intent> m_noteActivityLauncher;
	private String m_lastDialogName;
	private long m_lastDialogId;

	static private final String LAST_DIALOG_NAME = "lastDialogName";
	static private final String LAST_DIALOG_ID = "lastDialogId";
}
