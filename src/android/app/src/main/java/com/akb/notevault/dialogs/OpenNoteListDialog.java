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

package com.akb.notevault.dialogs;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.akb.notevault.R;

public class OpenNoteListDialog extends DialogFragment
{
	public String getName()
	{
		return m_name;
	}

	public void setName(String name)
	{
		m_name = name;
	}

	public String getPassword()
	{
		if (m_password == null)
			return "";
		return m_password.getText().toString();
	}

	public OnDialogAcceptedListener getOnDialogAcceptedListener()
	{
		return m_acceptedListener;
	}

	public void setOnDialogAcceptedListener(OnDialogAcceptedListener listener)
	{
		m_acceptedListener = listener;
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		LayoutInflater inflater = getActivity().getLayoutInflater();

		View rootView = inflater.inflate(R.layout.open_note_list_dialog, null);
		TextView openLabel = (TextView)rootView.findViewById(R.id.openLabel);
		openLabel.setText(getString(R.string.label_open).replace("%s", m_name));
		m_password = (EditText)rootView.findViewById(R.id.password);

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_open, null);
		builder.setNegativeButton(R.string.button_cancel, null);

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
						if (getPassword().isEmpty())
						{
							ErrorDialog.show(getActivity(), R.string.error_empty_password);
							return;
						}

						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(OpenNoteListDialog.this))
						{
							dismiss();
						}
					}
				});
			}
		});

		return alertDialog;
	}

	private OnDialogAcceptedListener m_acceptedListener;
	private String m_name;
	private EditText m_password;
}
