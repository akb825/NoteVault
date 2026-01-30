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

package com.akb.notevault.dialogs;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import androidx.fragment.app.DialogFragment;
import androidx.appcompat.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;

import com.akb.notevault.R;

public class RenameNoteDialog extends DialogFragment
{
	public RenameNoteDialog(String name, OnDialogAcceptedListener listener)
	{
		m_name = name;
		m_acceptedListener = listener;
	}

	public String getInitialName()
	{
		return m_name;
	}

	public String getNewName()
	{
		if (m_newName == null)
			return m_name;
		return m_newName.getText().toString();
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
		LayoutInflater inflater = getActivity().getLayoutInflater();

		View rootView = inflater.inflate(R.layout.add_rename_note_dialog, null);
		m_newName = rootView.findViewById(R.id.newName);
		m_newName.setText(m_name);
		m_newName.selectAll();

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_rename, null);
		builder.setNegativeButton(R.string.button_cancel, null);

		final AlertDialog alertDialog = builder.create();
		alertDialog.getWindow().setSoftInputMode(
			WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
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
						if (getNewName().isEmpty())
						{
							ErrorDialog.show(getActivity(), R.string.error_empty_name);
							return;
						}
						else if (getInitialName().equals(getNewName()))
						{
							dismiss();
							return;
						}

						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(RenameNoteDialog.this))
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
	private EditText m_newName;
}
