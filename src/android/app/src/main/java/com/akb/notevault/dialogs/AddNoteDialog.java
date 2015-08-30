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

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import com.akb.notevault.R;

public class AddNoteDialog extends DialogFragment
{
	public String getName()
	{
		if (m_name == null)
			return "";
		return m_name.getText().toString();
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

		View rootView = inflater.inflate(R.layout.add_rename_note_dialog, null);
		m_name = (EditText)rootView.findViewById(R.id.newName);

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_add, null);
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
						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(AddNoteDialog.this))
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
	private EditText m_name;
}
