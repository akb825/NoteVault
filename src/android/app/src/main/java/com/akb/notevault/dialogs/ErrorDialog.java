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

import android.app.Activity;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;

import com.akb.notevault.R;

public class ErrorDialog
{
	public static void show(Activity activity, String errorMessage)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(errorMessage);
		builder.setPositiveButton(R.string.button_ok, null);
		builder.create().show();
	}

	public static void show(Activity activity, int errorMessageResource)
	{
		show(activity, activity.getString(errorMessageResource));
	}
}
