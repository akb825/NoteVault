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

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;

import com.akb.notevault.dialogs.GeneratePasswordDialog;

public class NoteActivity extends AppCompatActivity
{

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_note);

		m_messageText = (EditText)getWindow().getDecorView().findViewById(R.id.message);

		Intent intent = getIntent();
		m_id = intent.getLongExtra("Id", -1);
		setTitle(intent.getStringExtra("Title"));
		m_messageText.setText(intent.getStringExtra("Message"));
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.menu_note, menu);
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
		if (id == R.id.action_password_generator)
		{
			GeneratePasswordDialog dialog = new GeneratePasswordDialog();
			dialog.show(getSupportFragmentManager(), "generate password");
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

	public void save(View view)
	{
		Intent data = new Intent();
		data.putExtra("Id", m_id);
		data.putExtra("Message", m_messageText.getText().toString());
		setResult(RESULT_OK, data);
		finish();
	}

	public void cancel(View view)
	{
		setResult(RESULT_CANCELED);
		finish();
	}

	private long m_id;
	private EditText m_messageText;
}