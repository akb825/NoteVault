package com.akb.notevault;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import com.akb.notevault.dialogs.ErrorDialog;
import com.akb.notevault.io.NoteFile;
import com.akb.notevault.notes.NoteSet;

import java.io.File;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

public class NoteList extends AppCompatActivity
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
		if (id == R.id.action_reload)
		{
			reloadNotes();
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
	public void onResume()
	{
		super.onResume();
		reloadNotes();
	}

	private void reloadNotes()
	{
		NoteFile.LoadResult loadResult = NoteFile.loadNotes(m_file, m_key);
		if (loadResult.result != NoteFile.Result.Success)
		{
			ErrorDialog.show(this, getString(R.string.error_load).replace("%s", m_name));
			finish();
		}
		m_notes = loadResult.notes;
		m_salt = loadResult.salt;
	}

	String m_name;
	private File m_file;
	private SecretKey m_key;
	private byte[] m_salt;
	private NoteSet m_notes;
}
