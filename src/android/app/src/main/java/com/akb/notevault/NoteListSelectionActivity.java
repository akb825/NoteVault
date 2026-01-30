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
import android.os.Environment;

import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.fragment.app.DialogFragment;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentFactory;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
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

import com.akb.notevault.dialogs.AddNoteListDialog;
import com.akb.notevault.dialogs.ChangePasswordDialog;
import com.akb.notevault.dialogs.ErrorDialog;
import com.akb.notevault.dialogs.LoadingDialog;
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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.crypto.SecretKey;


public class NoteListSelectionActivity extends AppCompatActivity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		if (savedInstanceState != null)
			m_lastDialogName = savedInstanceState.getString(LAST_DIALOG_NAME);
		getSupportFragmentManager().setFragmentFactory(new DialogFactory());

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_note_list_selection);
		setTitle(R.string.title_note_lists);

		m_noteListsAdapter = new CustomAdapter(getApplicationContext(), R.layout.list_item,
			R.id.itemText);

		Window window = getWindow();
		View decorView = window.getDecorView();
		ListView noteListsView = decorView.findViewById(R.id.noteLists);
		noteListsView.setAdapter(m_noteListsAdapter);
		populateNoteLists();

		Button addNoteListButton = decorView.findViewById(R.id.addNoteListButton);
		addNoteListButton.setOnClickListener(this::addNoteList);

		m_executor = Executors.newSingleThreadExecutor();
		m_postExecuteHandler = new Handler(Looper.getMainLooper());

		ViewCompat.setOnApplyWindowInsetsListener(
			decorView.findViewById(R.id.activityNoteListSelection),
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
		menu.add(R.string.menu_item_change_password).setOnMenuItemClickListener(
			new ChangePasswordListener(textView));
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
		AddNoteListDialog addDialog = createAddNoteListDialog();
		addDialog.show(getSupportFragmentManager(), "add note list");
	}

	private AddNoteListDialog createAddNoteListDialog()
	{
		return new AddNoteListDialog(new AddNoteListener());
	}

	private OpenNoteListDialog createOpenNoteListDialog(String name)
	{
		return new OpenNoteListDialog(name, new OpenNoteListener());
	}

	private RenameNoteListDialog createRenameNoteListDialog(String name)
	{
		return new RenameNoteListDialog(name, new RenameNoteListener());
	}

	private RemoveNoteListDialog createRemoveNoteListDialog(String name)
	{
		return new RemoveNoteListDialog(name, new RemoveNoteListener());
	}

	private ChangePasswordDialog createChangePasswordDialog(String name)
	{
		return new ChangePasswordDialog(name, new ChangeNotePasswordListener());
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
			return null;
		return getBaseContext().getExternalFilesDir(null);
	}

	private File[] getNoteListFiles()
	{
		final File root = getNoteListsRoot();
		if (root == null)
		{
			ErrorDialog.show(this, R.string.error_inaccessible);
			return null;
		}

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

	private void exit()
	{
		finish();
		System.exit(1);
	}

	private class DialogFactory extends FragmentFactory
	{
		@Override
		public Fragment instantiate(ClassLoader classLoader, String className)
		{
			Class<? extends Fragment> clazz = loadFragmentClass(classLoader, className);
			if (clazz == AddNoteListDialog.class)
				return createAddNoteListDialog();
			if (clazz == OpenNoteListDialog.class)
				return createOpenNoteListDialog(m_lastDialogName);
			if (clazz == RenameNoteListDialog.class)
				return createRenameNoteListDialog(m_lastDialogName);
			if (clazz == RemoveNoteListDialog.class)
				return createRemoveNoteListDialog(m_lastDialogName);
			if (clazz == ChangePasswordDialog.class)
				return createChangePasswordDialog(m_lastDialogName);
			return super.instantiate(classLoader, className);
		}
	}

	private static class ListItem
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

	private static class ItemCompare implements Comparator<ListItem>
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
			m_lastDialogName = m_noteListView.getText().toString();
			OpenNoteListDialog openDialog = createOpenNoteListDialog(m_lastDialogName);
			openDialog.show(getSupportFragmentManager(), "open note list");
		}

		private TextView m_noteListView;
	}

	private abstract class AsyncTask<DataType, ResultType>
	{
		public void execute(DataType data)
		{
			onPreExecute();
			m_executor.execute(new Runnable()
			{
				@Override
				public void run()
				{
					ResultType result = doInBackground(data);
					m_postExecuteHandler.post(new Runnable()
					{
						@Override
						public void run()
						{
							onPostExecute(result);
						}
					});
				}
			});
		}

		protected abstract void onPreExecute();
		protected abstract ResultType doInBackground(DataType data);
		protected abstract void onPostExecute(ResultType result);
	}

	private class OpenNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			if (m_task != null)
				return false;

			m_task = new Task();
			m_task.execute((OpenNoteListDialog)dialog);
			return false;
		}

		private class Task extends AsyncTask<OpenNoteListDialog, String>
		{
			@Override
			protected void onPreExecute()
			{
				m_loadingDialog.show(getSupportFragmentManager(), "loading dialog");
			}

			@Override
			protected String doInBackground(OpenNoteListDialog dialog)
			{
				m_dialog = dialog;
				String name = m_dialog.getName();
				String password = m_dialog.getPassword();

				File rootDir = getNoteListsRoot();
				if (rootDir == null)
					return getString(R.string.error_inaccessible);

				// Get the file for the new note list
				m_file = new File(rootDir, name + NoteFile.cExtension);
				NoteFile.LoadResult loadResult = NoteFile.loadNotes(m_file, password);
				String message = getString(R.string.error_load).replace("%s", name);
				switch (loadResult.result)
				{
					case Success:
						break;
					case EncryptionError:
						return getString(R.string.error_bad_password);
					default:
						return message;
				}

				m_key = loadResult.key;

				// NOTE: The number of key iterations might have changed. Re-save the file just in case.
				NoteFile.Result result = NoteFile.saveNotes(m_file, loadResult.notes,
					loadResult.salt, m_key);
				if (result != NoteFile.Result.Success)
					return message;

				return null;
			}

			@Override
			protected void onPostExecute(String result)
			{
				try
				{
					m_loadingDialog.dismiss();
					if (result == null)
					{
						// Bring up the note list.
						Intent intent = new Intent(NoteListSelectionActivity.this,
							NoteListActivity.class);
						intent.putExtra("FilePath", m_file);
						intent.putExtra("Key", m_key.getEncoded());
						startActivity(intent);

						m_dialog.dismiss();
					}
					else
						ErrorDialog.show(NoteListSelectionActivity.this, result);
					m_task = null;
				}
				catch (IllegalStateException e)
				{
					exit();
				}
			}

			private File m_file;
			private SecretKey m_key;
			private LoadingDialog m_loadingDialog = new LoadingDialog();
			private OpenNoteListDialog m_dialog;
		}

		private Task m_task;
	}

	private class AddNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			if (m_task != null)
				return false;

			m_task = new Task();
			m_task.execute((AddNoteListDialog)dialog);
			return false;
		}

		private class Task extends AsyncTask<AddNoteListDialog, String>
		{
			@Override
			protected void onPreExecute()
			{
				m_loadingDialog.show(getSupportFragmentManager(), "loading dialog");
			}

			@Override
			protected String doInBackground(AddNoteListDialog dialog)
			{
				m_dialog = dialog;
				String name = m_dialog.getName();
				String password = m_dialog.getPassword();

				File rootDir = getNoteListsRoot();
				if (rootDir == null)
					return getString(R.string.error_inaccessible);

				// Get the file for the new note list
				m_file = new File(rootDir, name + NoteFile.cExtension);
				if (m_file.exists())
					return getString(R.string.error_same_name).replace("%s", name);

				// Save an empty note list.
				byte[] salt = Crypto.random(Crypto.cSaltLenBytes);
				m_key = Crypto.generateKey(password, salt, Crypto.cDefaultKeyIterations);
				NoteFile.Result result = NoteFile.saveNotes(m_file, new NoteSet(), salt, m_key);

				if (result != NoteFile.Result.Success)
				{
					m_file.delete();
					return getString(R.string.error_create).replace("%s", name);
				}

				return null;
			}

			@Override
			protected void onPostExecute(String result)
			{
				try
				{
					m_loadingDialog.dismiss();
					if (result == null)
					{
						// Update the notes.
						populateNoteLists();

						// Bring up the note list.
						Intent intent = new Intent(NoteListSelectionActivity.this,
							NoteListActivity.class);
						intent.putExtra("FilePath", m_file);
						intent.putExtra("Key", m_key.getEncoded());
						startActivity(intent);

						m_dialog.dismiss();
					}
					else
						ErrorDialog.show(NoteListSelectionActivity.this, result);
					m_task = null;
				}
				catch (IllegalStateException e)
				{
					exit();
				}
			}

			private File m_file;
			private SecretKey m_key;
			private LoadingDialog m_loadingDialog = new LoadingDialog();
			private AddNoteListDialog m_dialog;
		}

		private Task m_task;
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
			m_lastDialogName = m_noteListView.getText().toString();
			RenameNoteListDialog renameDialog = createRenameNoteListDialog(m_lastDialogName);
			renameDialog.show(getSupportFragmentManager(), "rename note list");
			return true;
		}

		private TextView m_noteListView;
	}

	private class RenameNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			if (m_task != null)
				return false;

			m_task = new Task();
			m_task.execute((RenameNoteListDialog)dialog);
			return false;
		}

		private class Task extends AsyncTask<RenameNoteListDialog, String>
		{
			@Override
			protected void onPreExecute()
			{
				m_loadingDialog.show(getSupportFragmentManager(), "loading dialog");
			}

			@Override
			protected String doInBackground(RenameNoteListDialog dialog)
			{
				m_dialog = dialog;
				String oldName = m_dialog.getInitialName();
				String newName = m_dialog.getNewName();
				String password = m_dialog.getPassword();

				File rootDir = getNoteListsRoot();
				if (rootDir == null)
					return getString(R.string.error_inaccessible);

				// Ensure we can load the note list.
				File oldFile = new File(rootDir, oldName + NoteFile.cExtension);
				File newFile = new File(rootDir, newName + NoteFile.cExtension);

				if (newFile.exists())
					return getString(R.string.error_same_name).replace("%s", newName);

				NoteFile.LoadResult loadResult = NoteFile.loadNotes(oldFile, password);
				switch (loadResult.result)
				{
					case Success:
						break;
					case EncryptionError:
						return getString(R.string.error_bad_password);
					default:
						return getString(R.string.error_create).replace("%s", oldName);
				}

				if (!oldFile.renameTo(newFile))
					return getString(R.string.error_create).replace("%s", newName);

				return null;
			}

			@Override
			protected void onPostExecute(String result)
			{
				try
				{
					m_loadingDialog.dismiss();
					if (result == null)
					{
						populateNoteLists();
						m_dialog.dismiss();
					}
					else
						ErrorDialog.show(NoteListSelectionActivity.this, result);
					m_task = null;
				}
				catch (IllegalStateException e)
				{
					exit();
				}
			}

			private LoadingDialog m_loadingDialog = new LoadingDialog();
			private RenameNoteListDialog m_dialog;
		}

		private Task m_task;
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
			m_lastDialogName = m_noteListView.getText().toString();
			RemoveNoteListDialog removeDialog = createRemoveNoteListDialog(m_lastDialogName);
			removeDialog.show(getSupportFragmentManager(), "remove note list");
			return true;
		}

		private TextView m_noteListView;
	}

	private class RemoveNoteListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			if (m_task != null)
				return false;

			m_task = new Task();
			m_task.execute((RemoveNoteListDialog)dialog);
			return false;
		}

		private class Task extends AsyncTask<RemoveNoteListDialog, String>
		{
			@Override
			protected void onPreExecute()
			{
				m_loadingDialog.show(getSupportFragmentManager(), "loading dialog");
			}

			@Override
			protected String doInBackground(RemoveNoteListDialog dialog)
			{
				m_dialog = dialog;
				String name = m_dialog.getName();
				String password = m_dialog.getPassword();

				File rootDir = getNoteListsRoot();
				if (rootDir == null)
					return getString(R.string.error_inaccessible);

				// Ensure we can load the note list.
				File file = new File(rootDir, name + NoteFile.cExtension);
				NoteFile.LoadResult loadResult = NoteFile.loadNotes(file, password);

				String message = getString(R.string.error_remove).replace("%s", name);
				switch (loadResult.result)
				{
					case Success:
						break;
					case EncryptionError:
						return getString(R.string.error_bad_password);
					default:
						return message;
				}

				if (!file.delete())
					return message;

				return null;
			}

			@Override
			protected void onPostExecute(String result)
			{
				try
				{
					m_loadingDialog.dismiss();
					if (result == null)
					{
						populateNoteLists();
						m_dialog.dismiss();
					}
					else
						ErrorDialog.show(NoteListSelectionActivity.this, result);
					m_task = null;
				}
				catch (IllegalStateException e)
				{
					exit();
				}
			}

			private LoadingDialog m_loadingDialog = new LoadingDialog();
			private RemoveNoteListDialog m_dialog;
		}

		private Task m_task;
	}

	private class ChangePasswordListener implements MenuItem.OnMenuItemClickListener
	{
		public ChangePasswordListener(TextView noteListView)
		{
			m_noteListView = noteListView;
		}

		@Override
		public boolean onMenuItemClick(MenuItem item)
		{
			m_lastDialogName = m_noteListView.getText().toString();
			ChangePasswordDialog changePasswordDialog =
				createChangePasswordDialog(m_lastDialogName);
			changePasswordDialog.show(getSupportFragmentManager(), "change note list password");
			return true;
		}

		private TextView m_noteListView;
	}

	private class ChangeNotePasswordListener implements OnDialogAcceptedListener
	{
		@Override
		public boolean onDialogAccepted(DialogFragment dialog)
		{
			if (m_task != null)
				return false;

			m_task = new Task();
			m_task.execute((ChangePasswordDialog)dialog);
			return false;
		}

		private class Task extends AsyncTask<ChangePasswordDialog, String>
		{
			@Override
			protected void onPreExecute()
			{
				m_loadingDialog.show(getSupportFragmentManager(), "loading dialog");
			}

			@Override
			protected String doInBackground(ChangePasswordDialog dialog)
			{
				m_dialog = dialog;
				String name = m_dialog.getName();
				String currentPassword = m_dialog.getCurrentPassword();
				String newPassword = m_dialog.getNewPassword();

				File rootDir = getNoteListsRoot();
				if (rootDir == null)
					return getString(R.string.error_inaccessible);

				// First load the note list.
				File file = new File(rootDir, name + NoteFile.cExtension);
				NoteFile.LoadResult loadResult = NoteFile.loadNotes(file, currentPassword);
				switch (loadResult.result)
				{
					case Success:
						break;
					case EncryptionError:
						return getString(R.string.error_bad_password);
					default:
						return getString(R.string.error_load).replace("%s", name);
				}

				// Save the note list with the new password.
				byte[] salt = Crypto.random(Crypto.cSaltLenBytes);
				SecretKey key = Crypto.generateKey(newPassword, salt, Crypto.cDefaultKeyIterations);
				NoteFile.Result result = NoteFile.saveNotes(file, loadResult.notes, salt, key);

				if (result != NoteFile.Result.Success)
					return getString(R.string.error_save).replace("%s", name);

				return null;
			}

			@Override
			protected void onPostExecute(String result)
			{
				try
				{
					m_loadingDialog.dismiss();
					if (result == null)
						m_dialog.dismiss();
					else
						ErrorDialog.show(NoteListSelectionActivity.this, result);
					m_task = null;
				}
				catch (IllegalStateException e)
				{
					exit();
				}
			}

			private LoadingDialog m_loadingDialog = new LoadingDialog();
			private ChangePasswordDialog m_dialog;
		}

		private Task m_task;
	}

	private CustomAdapter m_noteListsAdapter;
	private ExecutorService m_executor;
	private Handler m_postExecuteHandler;
	private String m_lastDialogName;

	static private final String LAST_DIALOG_NAME = "lastDialogName";
}
