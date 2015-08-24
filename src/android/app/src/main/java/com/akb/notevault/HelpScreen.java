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

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.webkit.WebView;

import com.akb.notevault.io.NoteFile;

import java.io.File;


public class HelpScreen extends AppCompatActivity
{

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_help_screen);

		String helpHtml = "<html><body>";

		helpHtml = helpHtml +
			"<h1>" + getString(R.string.app_name) + "</h1>";

		helpHtml = helpHtml +
			"<p>" + getString(R.string.help_about) + "</p>";

		String transfer = getString(R.string.help_transfer);
		File rootPath = getBaseContext().getExternalFilesDir(null);
		if (rootPath != null)
			transfer = transfer.replace("%f", rootPath.getPath());
		transfer = transfer.replace("%ext", NoteFile.cExtension);

		helpHtml = helpHtml +
			"<p>" + transfer + "</p>";

		helpHtml = helpHtml +
			"<p><font size=2>" + getString(R.string.copyright) + "</font></p>";

		WebView helpView = (WebView)getWindow().getDecorView().findViewById(R.id.helpText);
		helpView.loadData(helpHtml, "text/html", null);

		setTitle(R.string.title_help_screen);
	}

	public void closeActivity(View view)
	{
		finish();
	}

}
