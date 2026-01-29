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

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.webkit.WebView;
import android.widget.Button;

import com.akb.notevault.io.NoteFile;

import java.io.File;


public class HelpActivity extends AppCompatActivity
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
		{
			String rootPathStr = rootPath.getPath();
			int androidFolderPos = rootPathStr.indexOf("Android");
			if (androidFolderPos >= 0)
				rootPathStr = rootPathStr.substring(androidFolderPos);
			transfer = transfer.replace("%f", rootPathStr);
		}
		transfer = transfer.replace("%ext", NoteFile.cExtension);

		helpHtml = helpHtml +
			"<p>" + transfer + "</p>";

		helpHtml = helpHtml +
			"<h2>" + getString(R.string.help_tips_title) + "</h2>";

		helpHtml = helpHtml +
			"<p>" + getString(R.string.help_tips) + "</p>";

		helpHtml = helpHtml +
			"<p><font size=2>" + getString(R.string.copyright) + "</font></p>";

		Window window = getWindow();
        View decorView = window.getDecorView();
		WebView helpView = decorView.findViewById(R.id.helpText);
		helpView.loadData(helpHtml, "text/html", null);

		Button closeButton = decorView.findViewById(R.id.closeButton);
		closeButton.setOnClickListener(this::closeActivity);

		setTitle(R.string.title_help_screen);

		ViewCompat.setOnApplyWindowInsetsListener(
			decorView.findViewById(R.id.activityHelpScreen),
			new WindowInsetHandler());

		WindowInsetsControllerCompat insetsController =
			WindowCompat.getInsetsController(window, decorView);
		insetsController.setAppearanceLightStatusBars(true);
		insetsController.setAppearanceLightNavigationBars(true);
	}

	public void closeActivity(View view)
	{
		finish();
	}

}
