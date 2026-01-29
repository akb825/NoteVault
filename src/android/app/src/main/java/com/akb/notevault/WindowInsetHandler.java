/*
 * Copyright 2026 Aaron Barany
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

import android.view.View;
import android.view.ViewGroup;

import androidx.core.graphics.Insets;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.WindowInsetsCompat;

public class WindowInsetHandler implements OnApplyWindowInsetsListener
{
	@Override
	public WindowInsetsCompat onApplyWindowInsets(View view, WindowInsetsCompat windowInsets)
	{
		// Apply insets for the system bars to account for edge-to-edge displays.
		Insets insets = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars());
		ViewGroup.MarginLayoutParams layoutParams =
			(ViewGroup.MarginLayoutParams)view.getLayoutParams();
		layoutParams.leftMargin = insets.left;
		layoutParams.topMargin = insets.top;
		layoutParams.rightMargin = insets.right;
		layoutParams.bottomMargin = insets.bottom;
		view.setLayoutParams(layoutParams);
		return WindowInsetsCompat.CONSUMED;
	}
}
