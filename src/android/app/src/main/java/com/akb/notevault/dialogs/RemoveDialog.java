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

import com.akb.notevault.R;

public class RemoveDialog extends DialogFragment
{
	public String getName()
	{
		return m_name;
	}

	public void setName(String name)
	{
		m_name = name;
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

		String message = getString(R.string.message_confirm_remove);
		message = message.replace("%s", m_name);
		builder.setMessage(message);
		builder.setPositiveButton(R.string.button_remove, new DialogInterface.OnClickListener()
		{
			@Override
			public void onClick(DialogInterface dialog, int id)
			{
				if (m_acceptedListener != null)
					m_acceptedListener.onDialogAccepted(RemoveDialog.this);
			}
		});
		builder.setNegativeButton(R.string.button_cancel, new DialogInterface.OnClickListener()
		{
			@Override
			public void onClick(DialogInterface dialog, int id)
			{
				RemoveDialog.this.getDialog().cancel();
			}
		});

		return builder.create();
	}

	private OnDialogAcceptedListener m_acceptedListener;
	private String m_name;
}
