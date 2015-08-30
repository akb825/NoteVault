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

package com.akb.notevault.io;

import com.akb.notevault.notes.Note;
import com.akb.notevault.notes.NoteSet;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;

public class NoteFile
{
	public static final int cFileVersion = 1;
	public static final String cExtension = ".secnote";

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
	}

	public static LoadResult loadNotes(File file, String password)
	{
		FileInputStream stream;
		try
		{
			stream = new FileInputStream(file);
		}
		catch (FileNotFoundException e)
		{
			LoadResult result = new LoadResult();
			result.result = Result.IoError;
			return result;
		}

		LoadResult result = loadNotes(stream, password);
		try
		{
			stream.close();
		}
		catch (IOException e) {}

		return result;
	}

	public static LoadResult loadNotes(InputStream stream, String password)
	{
		LoadResult results = new LoadResult();;

		try
		{
			//Read the header: magic string, version, salt, and initialization vector.
			byte[] magicStringCheck = new byte[m_magicString.length()];
			readBytes(stream, magicStringCheck);
			if (!new String(magicStringCheck, m_charset).equals(m_magicString))
			{
				results.result = Result.InvalidFile;
				return results;
			}

			// Read null terminator.
			stream.read();

			int version = readInt(stream);
			if (version > cFileVersion)
			{
				results.result = Result.InvalidVersion;
				return results;
			}

			int saltLen = readInt(stream);
			byte[] salt = new byte[saltLen];
			readBytes(stream, salt);

			int numIterations = Crypto.cDefaultKeyIterations;
			//If an older file version, set the number of iterations based on that version.
			if (version == 0)
				numIterations = Crypto.cVer0KeyIterations;
			SecretKey key = Crypto.generateKey(password, salt, numIterations);

			int ivLen = readInt(stream);
			byte[] iv = new byte[ivLen];
			readBytes(stream, iv);

			//Main file. (encrypted)
			Cipher cipher = Cipher.getInstance(m_encryptionAlgorithm);
			cipher.init(Cipher.DECRYPT_MODE, key, new IvParameterSpec(iv));
			CipherInputStream cryptoStream = new CipherInputStream(stream, cipher);

			NoteSet notes = new NoteSet();
			results.result = loadNotesImpl(cryptoStream, notes);
			cryptoStream.close();
			if (results.result != Result.Success)
				return results;

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

	public static LoadResult loadNotes(File file, SecretKey key)
	{
		FileInputStream stream;
		try
		{
			stream = new FileInputStream(file);
		}
		catch (FileNotFoundException e)
		{
			LoadResult result = new LoadResult();
			result.result = Result.IoError;
			return result;
		}

		LoadResult result = loadNotes(stream, key);
		try
		{
			stream.close();
		}
		catch (IOException e) {}

		return result;
	}

	public static LoadResult loadNotes(InputStream stream, SecretKey key)
	{
		LoadResult results = new LoadResult();

		try
		{
			//Read the header: magic string, version, salt, and initialization vector.
			byte[] magicStringCheck = new byte[m_magicString.length()];
			readBytes(stream, magicStringCheck);
			if (!new String(magicStringCheck, m_charset).equals(m_magicString))
			{
				results.result = Result.InvalidFile;
				return results;
			}

			// Read null terminator.
			stream.read();

			int version = readInt(stream);
			if (version > cFileVersion)
			{
				results.result = Result.InvalidVersion;
				return results;
			}

			int saltLen = readInt(stream);
			byte[] salt = new byte[saltLen];
			readBytes(stream, salt);

			int ivLen = readInt(stream);
			byte[] iv = new byte[ivLen];
			readBytes(stream, iv);

			//Main file. (encrypted)
			Cipher cipher = Cipher.getInstance(m_encryptionAlgorithm);
			cipher.init(Cipher.DECRYPT_MODE, key, new IvParameterSpec(iv));
			CipherInputStream cryptoStream = new CipherInputStream(stream, cipher);

			NoteSet notes = new NoteSet();
			results.result = loadNotesImpl(cryptoStream, notes);
			cryptoStream.close();
			if (results.result != Result.Success)
				return results;

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

	private static Result loadNotesImpl(InputStream cryptoStream, NoteSet notes) throws IOException
	{
		//Read the magic string again to verify the correct key
		byte[] magicStringCheck = new byte[m_magicString.length()];
		try
		{
			readBytes(cryptoStream, magicStringCheck);
		}
		catch (IOException e)
		{
			return Result.EncryptionError;
		}

		if (!new String(magicStringCheck, m_charset).equals(m_magicString))
			return Result.EncryptionError;

		// Read null terminator.
		cryptoStream.read();

		//Read the notes
		int numNotes = readInt(cryptoStream);
		for (int i = 0; i < numNotes; ++i)
		{
			long id = readLong(cryptoStream);

			Note note = new Note(id);
			note.setTitle(readString(cryptoStream));
			note.setMessage(readString(cryptoStream));

			if (!notes.add(note))
				return Result.IoError;
		}

		return Result.Success;
	}

	public static Result saveNotes(File file, NoteSet notes, byte[] salt, SecretKey key)
	{
		FileOutputStream stream;
		try
		{
			stream = new FileOutputStream(file);
		}
		catch (FileNotFoundException e)
		{
			return Result.IoError;
		}

		Result result = saveNotes(stream, notes, salt, key);
		try
		{
			stream.close();
		}
		catch (IOException e) {}

		return result;
	}

	public static Result saveNotes(OutputStream stream, NoteSet notes, byte[] salt, SecretKey key)
	{
		DataOutputStream dataStream = new DataOutputStream(stream);

		try
		{
			//Write the header: magic string, version, salt, and initialization vector.
			dataStream.write(m_magicString.getBytes(m_charset));
			dataStream.write(0);
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
			cryptoStream.write(0);

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

			cryptoStream.close();
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

	private static String readString(InputStream stream) throws IOException
	{
		int strLen = readInt(stream);
		byte[] strBytes = new byte[strLen];
		readBytes(stream, strBytes);
		return new String(strBytes, m_charset);
	}

	private static int readBytes(InputStream stream, byte[] bytes) throws IOException
	{
		int readBytes = 0;
		do
		{
			int thisRead = stream.read(bytes, readBytes, bytes.length - readBytes);
			if (thisRead <= 0)
				break;
			readBytes += thisRead;
		} while (readBytes < bytes.length);
		return readBytes;
	}

	private static int readInt(InputStream stream) throws IOException
	{
		byte[] intBytes = new byte[4];
		readBytes(stream, intBytes);
		return ByteBuffer.wrap(intBytes).getInt();
	}

	private static long readLong(InputStream stream) throws IOException
	{
		byte[] intBytes = new byte[8];
		readBytes(stream, intBytes);
		return ByteBuffer.wrap(intBytes).getLong();
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
