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

import com.akb.notevault.notes.Note;
import com.akb.notevault.notes.NoteSet;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.Charset;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;

public class NoteFile
{
	public static final int cFileVersion = 0;

	public enum Result
	{
		Success,
		InvalidFile,
		InvalidVersion,
		IoError,
		EncryptionError
	}

	public static class LoadResult
	{
		public Result result;
		public NoteSet notes;
		public byte[] salt;
		public SecretKey key;
	};

	public static LoadResult loadNotes(InputStream stream, String password)
	{
		LoadResult results = new LoadResult();
		DataInputStream dataStream = new DataInputStream(stream);

		try
		{
			//Read the header: magic string, version, salt, and initialization vector.
			byte[] magicStringCheck = new byte[m_magicString.length()];
			dataStream.read(magicStringCheck);
			if (!new String(magicStringCheck, m_charset).equals(m_magicString))
			{
				results.result = Result.InvalidFile;
				return results;
			}

			int version = dataStream.readInt();
			if (version > cFileVersion)
			{
				results.result = Result.InvalidVersion;
				return results;
			}

			int saltLen = dataStream.readInt();
			byte[] salt = new byte[saltLen];
			dataStream.read(salt);

			int numIterations = dataStream.readInt();
			SecretKey key = Crypto.generateKey(password, salt, numIterations);

			int ivLen = dataStream.readInt();
			byte[] iv = new byte[ivLen];
			dataStream.read(iv);

			//Main file. (encrypted)
			Cipher cipher = Cipher.getInstance(m_encryptionAlgorithm);
			cipher.init(Cipher.DECRYPT_MODE, key, new IvParameterSpec(iv));
			DataInputStream cryptoStream = new DataInputStream(new CipherInputStream(dataStream,
				cipher));

			//Read the magic string again to verify the correct key
			magicStringCheck = new byte[m_magicString.length()];
			cryptoStream.read(magicStringCheck);
			if (!new String(magicStringCheck, m_charset).equals(m_magicString))
			{
				results.result = Result.EncryptionError;
				return results;
			}

			//Read the notes
			NoteSet notes = new NoteSet();
			int numNotes = cryptoStream.readInt();
			for (int i = 0; i < numNotes; ++i)
			{
				long id = cryptoStream.readLong();

				Note note = new Note(id);
				note.setTitle(readString(cryptoStream));
				note.setMessage(readString(cryptoStream));

				if (!notes.add(note))
				{
					results.result = Result.IoError;
					return results;
				}
			}

			//If reading from an old file, re-calculate the key with the updated number of iterations.
			if (numIterations != Crypto.cDefaultKeyIterations)
				key = Crypto.generateKey(password, salt, Crypto.cDefaultKeyIterations);

			results.result = Result.Success;
			results.notes = notes;
			results.salt = salt;
			results.key = key;
		}
		catch (IOException e)
		{
			results.result = Result.IoError;
			return results;
		}
		catch (NoSuchPaddingException | NoSuchAlgorithmException |
			InvalidAlgorithmParameterException | InvalidKeyException e)
		{
			results.result = Result.EncryptionError;
			return results;
		}

		return results;
	}

	private static Result writeNotes(OutputStream stream, NoteSet notes, String password,
		byte[] salt, PrivateKey key)
	{
		DataOutputStream dataStream = new DataOutputStream(stream);

		try
		{
			//Write the header: magic string, version, salt, and initialization vector.
			dataStream.write(m_magicString.getBytes(m_charset));
			dataStream.writeInt(cFileVersion);
			dataStream.writeInt(salt.length);
			dataStream.write(salt);

			//Generate a new initialization vector
			byte[] iv = Crypto.random(Crypto.cBlockLenBytes);
			dataStream.writeInt(iv.length);
			dataStream.write(iv);

			//Main file. (encrypted)
			Cipher cipher = Cipher.getInstance(m_encryptionAlgorithm);
			cipher.init(Cipher.ENCRYPT_MODE, key, new IvParameterSpec(iv));
			DataOutputStream cryptoStream = new DataOutputStream(new CipherOutputStream(dataStream,
				cipher));

			//write the magic string again for verifying the correct key
			cryptoStream.write(m_magicString.getBytes(m_charset));

			//write the notes
			cryptoStream.writeInt(notes.size());
			NoteSet.Iterator iterator = notes.iterator();
			while (iterator.hasNext())
			{
				Note note = iterator.next();
				cryptoStream.writeLong(note.getId());
				writeString(cryptoStream, note.getTitle());
				writeString(cryptoStream, note.getMessage());
			}

			cryptoStream.flush();
		}
		catch (IOException e)
		{
			return Result.IoError;
		}
		catch (NoSuchPaddingException | NoSuchAlgorithmException |
			InvalidAlgorithmParameterException | InvalidKeyException e)
		{
			return Result.EncryptionError;
		}

		return Result.Success;
	}

	private static String readString(DataInputStream stream) throws IOException
	{
		int strLen = stream.readInt();
		byte[] strBytes = new byte[strLen];
		return new String(strBytes, m_charset);
	}

	private static void writeString(DataOutputStream stream, String string) throws IOException
	{
		stream.writeInt(string.length());
		stream.write(string.getBytes(m_charset));
	}

	private static final String m_encryptionAlgorithm = "AES/CBC/PKCS5Padding";
	private static final String m_magicString = "NoteVault";
	private static final Charset m_charset = Charset.forName("UTF-8");
}
