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

package com.akb.notevault.io;

import java.nio.charset.Charset;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Crypto
{
	public static final int cKeyLen = 256;
	public static final int cKeyLenBytes = cKeyLen/8;
	public static final int cBlockLen = 128;
	public static final int cBlockLenBytes = cBlockLen/8;
	public static final int cSaltLen = 128;
	public static final int cSaltLenBytes = cSaltLen/8;
	public static final int cDefaultKeyIterations = 100000;

	public static SecretKey generateKey(String password, byte[] salt, int numIterations)
	{
		PBEKeySpec keySpec = new PBEKeySpec(password.toCharArray(), salt, numIterations,
			cKeyLenBytes);
		try
		{
			return m_keyFactory.generateSecret(keySpec);
		}
		catch (InvalidKeySpecException e)
		{
			throw new NullPointerException();
		}
	}

	public static byte[] random(int numBytes)
	{
		byte[] bytes = new byte[numBytes];
		m_random.nextBytes(bytes);
		return bytes;
	}

	static
	{
		try
		{
			m_keyFactory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
		}
		catch (NoSuchAlgorithmException e)
		{
			throw new NullPointerException();
		}
	}

	private static final SecureRandom m_random = new SecureRandom();
	private static final SecretKeyFactory m_keyFactory;
}
