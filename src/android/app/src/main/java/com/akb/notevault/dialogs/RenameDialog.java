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
import android.widget.EditText;

import com.akb.notevault.R;

public class RenameDialog extends DialogFragment
{
	public String getIntiialName()
	{
		return m_name;
	}

	public void setInitialName(String name)
	{
		m_name = name;
	}

	public String getNewName()
	{
		if (m_newName == null)
			return m_name;
		return m_newName.getText().toString();
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

		View rootView = inflater.inflate(R.layout.rename_dialog, null);
		m_newName = (EditText)rootView.findViewById(R.id.newName);
		m_newName.setText(m_name);

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_rename, new DialogInterface.OnClickListener()
		{
			@Override
			public void onClick(DialogInterface dialog, int id)
			{
				if (m_acceptedListener != null)
					m_acceptedListener.onDialogAccepted(RenameDialog.this);
			}
		});
		builder.setNegativeButton(R.string.button_cancel, new DialogInterface.OnClickListener()
		{
			@Override
			public void onClick(DialogInterface dialog, int id)
			{
				RenameDialog.this.getDialog().cancel();
			}
		});

		return builder.create();
	}

	private OnDialogAcceptedListener m_acceptedListener;
	private String m_name;
	private EditText m_newName;
}
