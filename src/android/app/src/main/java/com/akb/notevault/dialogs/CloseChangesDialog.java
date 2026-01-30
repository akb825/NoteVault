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

import android.view.View;
import android.widget.Button;

import com.akb.notevault.R;

public class CloseChangesDialog extends DialogFragment
{
	public CloseChangesDialog(OnDialogAcceptedListener listener)
	{
		m_acceptedListener = listener;
	}

	public boolean getSave()
	{
		return m_save;
	}

	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());

		builder.setMessage(R.string.message_close_changed);
		builder.setPositiveButton(R.string.button_save, null);
		builder.setNegativeButton(R.string.button_cancel, null);
		builder.setNeutralButton(R.string.button_dont_save, null);

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
						m_save = true;
						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(CloseChangesDialog.this))
						{
							dismiss();
						}
					}
				});
				button = alertDialog.getButton(AlertDialog.BUTTON_NEUTRAL);
				button.setOnClickListener(new View.OnClickListener()
				{
					@Override
					public void onClick(View view)
					{
						m_save = false;
						if (m_acceptedListener == null ||
							m_acceptedListener.onDialogAccepted(CloseChangesDialog.this))
						{
							dismiss();
						}
					}
				});
			}
		});

		return alertDialog;
	}

	private boolean m_save = false;
	private OnDialogAcceptedListener m_acceptedListener;
}
