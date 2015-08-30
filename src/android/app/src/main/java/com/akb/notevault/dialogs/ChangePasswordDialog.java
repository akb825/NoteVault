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

public class ChangePasswordDialog extends DialogFragment
{
	public String getName()
	{
		return m_name;
	}

	public void setName(String name)
	{
		m_name = name;
	}

	public String getCurrentPassword()
	{
		if (m_currentPassword == null)
			return "";
		return m_currentPassword.getText().toString();
	}

	public String getNewPassword()
	{
		if (m_newPassword == null)
			return "";
		return m_newPassword.getText().toString();
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

		View rootView = inflater.inflate(R.layout.change_password_dialog, null);
		TextView title = (TextView)rootView.findViewById(R.id.changePasswordLabel);
		title.setText(getString(R.string.label_change_password).replace("%s", m_name));
		m_currentPassword = (EditText)rootView.findViewById(R.id.currentPassword);
		m_newPassword = (EditText)rootView.findViewById(R.id.newPassword);
		m_passwordConfirm = (EditText)rootView.findViewById(R.id.passwordConfirm);

		builder.setView(rootView);
		builder.setPositiveButton(R.string.button_change, null);
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
						if (getName().isEmpty())
						{
							ErrorDialog.show(getActivity(), R.string.error_empty_name);
							return;
						}

						String currentPassword = getCurrentPassword();
						String newPassword = getNewPassword();
						if (currentPassword.isEmpty() || newPassword.isEmpty())
						{
							ErrorDialog.show(getActivity(), R.string.error_empty_password);
							return;
						}

						if (!newPassword.equals(m_passwordConfirm.getText().toString()))
						{
							ErrorDialog.show(getActivity(), R.string.error_password_mismatch);
							return;
						}

						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(ChangePasswordDialog.this))
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
	private EditText m_currentPassword;
	private EditText m_newPassword;
	private EditText m_passwordConfirm;
}
