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

package com.akb.notevault.dialogs;

import android.app.Activity;
import android.app.Dialog;
import android.text.ClipboardManager;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.akb.notevault.R;
import com.akb.notevault.io.Crypto;

public class GeneratePasswordDialog extends DialogFragment
{
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		LayoutInflater inflater = getActivity().getLayoutInflater();

		View rootView = inflater.inflate(R.layout.password_generator_dialog, null);
		m_numCharacters = (EditText)rootView.findViewById(R.id.numCharacters);
		m_password = (EditText)rootView.findViewById(R.id.password);
		generatePassword();

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_generate, null);
		builder.setNegativeButton(R.string.button_close, null);
		builder.setNeutralButton(R.string.button_copy, null);

		final AlertDialog alertDialog = builder.create();
		alertDialog.setOnShowListener(new DialogInterface.OnShowListener()
		{
			@Override
			public void onShow(DialogInterface dialog)
			{
				Button button = alertDialog.getButton(AlertDialog.BUTTON_POSITIVE);
				button.setOnClickListener(new View.OnClickListener()
				{
					@Override
					public void onClick(View view)
					{
						generatePassword();
					}
				});

				button = alertDialog.getButton(AlertDialog.BUTTON_NEUTRAL);
				button.setOnClickListener(new View.OnClickListener()
				{
					@Override
					public void onClick(View view)
					{
						Activity activity = getActivity();
						ClipboardManager clipboardManager =
							(ClipboardManager)activity.getSystemService(Activity.CLIPBOARD_SERVICE);
						String password = m_password.getText().toString();
						clipboardManager.setText(password);
					}
				});
			}
		});

		return alertDialog;
	}

	private void generatePassword()
	{
		int numChars = Integer.parseInt(m_numCharacters.getText().toString());

		int charOffset = (int)cFirstChar;
		int numCharCodes = cLastChar - cFirstChar;

		byte[] randomChars = Crypto.random(numChars);
		char[] password = new char[numChars];
		for (int i = 0; i < numChars; ++i)
		{
			int randomChar = Math.abs(randomChars[i]);
			password[i] = (char)((randomChar % numCharCodes) + charOffset);
		}
		m_password.setText(password, 0, password.length);
	}

	private static final char cFirstChar = '!';
	private static final char cLastChar = '~';

	private EditText m_numCharacters;
	private EditText m_password;
}
