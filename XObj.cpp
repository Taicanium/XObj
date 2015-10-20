// XObj.cpp : main project file.

#include "stdafx.h"

using namespace System;
using namespace System::IO;

enum FileType
{
	SkelAnim,
	MatAnim,
	VisAnim,
	Tex,
	Model,
	Other,
};

void ConvertFile(array<Byte>^ data, String^ path, String^ ext, FileType type)
{
	unsigned short numBCHs = 0;

	array<unsigned int>^ begins;
	array<unsigned int>^ ends;

	unsigned int firstBCHOffset = 0;

	array<Byte>^ outData;

	array<Byte>^ containerHead;

	MemoryStream^ mems;
	BinaryWriter^ bw;

	MemoryStream^ outStr;
	BinaryWriter^ outBW;

	// PB files are weird; it seems the first BCH block acts as some kind of a special header, which we won't be decoding.
	if (type == FileType::SkelAnim)
	{
		bool headBCH = false;

		for (int k = 0; k < data->Length; k++)
		{
			if (data[k] == 0x42)
			{
				if (data[k + 1] == 0x43)
				{
					if (data[k + 2] == 0x48)
					{
						if (headBCH == true)
						{
							numBCHs++;

							if (numBCHs == 1)
							{
								firstBCHOffset = k;
							}
						}
						else
						{
							headBCH = true;
						}
					}
				}
			}
		}
	}
	else
	{
		for (int k = 0; k < data->Length; k++)
		{
			if (data[k] == 0x42)
			{
				if (data[k + 1] == 0x43)
				{
					if (data[k + 2] == 0x48)
					{
						numBCHs++;

						if (numBCHs == 1)
						{
							firstBCHOffset = k;
						}
					}
				}
			}
		}
	}

	if (numBCHs > 0)
	{
		containerHead = gcnew array<Byte>(firstBCHOffset + 1);

		mems = gcnew MemoryStream(containerHead);
		bw = gcnew BinaryWriter(mems);

		begins = gcnew array<unsigned int>(numBCHs);
		ends = gcnew array<unsigned int>(numBCHs);

		int currentBCH = 0;

		if (type == FileType::SkelAnim)
		{
			if (type == SkelAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x42);
			}
			else if (type == MatAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x46);
			}
			else if (type == VisAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x4B);
			}

			bw->Write(numBCHs);

			bool headBCH = false;

			for (int k = 0; k < data->Length; k++)
			{
				if (data[k] == 0x42)
				{
					if (data[k + 1] == 0x43)
					{
						if (data[k + 2] == 0x48)
						{
							if (headBCH == true)
							{
								bw->Write((unsigned int)k);
								begins[currentBCH] = k;

								if (currentBCH == numBCHs - 1)
								{
									bw->Write((unsigned int)data->Length);
									ends[currentBCH] = data->Length;
								}
								else
								{
									for (int l = k + 1; l < data->Length; l++)
									{
										if (data[l] == 0x42)
										{
											if (data[l + 1] == 0x43)
											{
												if (data[l + 2] == 0x48)
												{
													bw->Write(l);
													ends[currentBCH] = l;

													l = data->Length;
												}
											}
										}
									}
								}

								currentBCH++;
							}
							else
							{
								headBCH = true;
							}
						}
					}
				}
			}
		}
		else if (type == FileType::Other)
		{
			for (int k = 0; k < data->Length; k++)
			{
				if (data[k] == 0x42)
				{
					if (data[k + 1] == 0x43)
					{
						if (data[k + 2] == 0x48)
						{
							begins[currentBCH] = k;

							if (currentBCH == numBCHs - 1)
							{
								ends[currentBCH] = data->Length;
							}
							else
							{
								for (int l = k + 1; l < data->Length; l++)
								{
									if (data[l] == 0x42)
									{
										if (data[l + 1] == 0x43)
										{
											if (data[l + 2] == 0x48)
											{
												ends[currentBCH] = l;

												l = data->Length;
											}
										}
									}
								}
							}

							currentBCH++;
						}
					}
				}
			}
		}
		else
		{
			if (type == SkelAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x42);
			}
			else if (type == MatAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x46);
			}
			else if (type == VisAnim)
			{
				bw->Write((unsigned char)0x50);
				bw->Write((unsigned char)0x4B);
			}

			bw->Write(numBCHs);

			for (int k = 0; k < data->Length; k++)
			{
				if (data[k] == 0x42)
				{
					if (data[k + 1] == 0x43)
					{
						if (data[k + 2] == 0x48)
						{
							bw->Write((unsigned int)k);
							begins[currentBCH] = k;

							if (currentBCH == numBCHs - 1)
							{
								bw->Write((unsigned int)data->Length);
								ends[currentBCH] = data->Length;
							}
							else
							{
								for (int l = k + 1; l < data->Length; l++)
								{
									if (data[l] == 0x42)
									{
										if (data[l + 1] == 0x43)
										{
											if (data[l + 2] == 0x48)
											{
												bw->Write(l);
												ends[currentBCH] = l;

												l = data->Length;
											}
										}
									}
								}
							}

							currentBCH++;
						}
					}
				}
			}
		}

		if (type == FileType::Other)
		{
			bw->Close();

			for (int k = 0; k < numBCHs; k++)
			{
				outData = gcnew array<Byte>(ends[k] - begins[k]);
				outStr = gcnew MemoryStream(outData);
				outBW = gcnew BinaryWriter(outStr);

				for (unsigned int l = begins[k]; l < ends[k]; l++)
				{
					outBW->Write((unsigned char)(data[l]));
				}

				outBW->Close();

				File::WriteAllBytes(path + "_" + k.ToString() + ext, outData);
			}

			return;
		}

		currentBCH = 0;

		outData = gcnew array<Byte>(data->Length);

		if (type == FileType::Other)
		{
			outStr = gcnew MemoryStream(outData);
			outBW = gcnew BinaryWriter(outStr);

			for (int k = 0; k < numBCHs; k++)
			{
				for (unsigned int l = begins[k]; l < ends[k]; l++)
				{
					outBW->Write((unsigned char)(data[l]));
				}

				File::WriteAllBytes(path + "_" + k.ToString() + ext, outData);
			}
		}
		else
		{
			bw->Write(outData->Length);

			for (long long k = bw->BaseStream->Position; k < bw->BaseStream->Length; k++)
			{
				bw->Write((unsigned char)0x00);
			}

			outStr = gcnew MemoryStream(outData);
			outBW = gcnew BinaryWriter(outStr);

			for (int k = 0; k < bw->BaseStream->Length; k++)
			{
				outBW->Write((unsigned char)(containerHead[k]));
			}

			for (int k = 0; k < numBCHs; k++)
			{
				outBW->Seek(begins[k], SeekOrigin::Begin);

				for (unsigned int l = begins[k]; l < ends[k]; l++)
				{
					outBW->Write((unsigned char)(data[l]));
				}
			}

			currentBCH++;

			File::WriteAllBytes(path + ext, outData);
		}
	}

	return;
}

int main(array<System::String ^> ^args)
{
	array<String^>^ folders = Directory::GetDirectories("XModels/", "*", SearchOption::AllDirectories);

	bool DECOMPRESSING = false;

	for (int i = 0; i < folders->Length; i++)
	{
		if (folders[i]->Contains("0/0/7_"))
		{
			array<String^>^ files = Directory::GetFiles(folders[i]);

			for (int j = 4; j < files->Length; j++)
			{
				if (files[j]->Contains("dec_") == false && files[j]->Contains(".lz") == false)
				{
					DECOMPRESSING = true;
					File::Move(files[j], files[j] + ".lz");
				}
			}
		}
	}

	if (DECOMPRESSING == false)
	{
		for (int i = 0; i < folders->Length; i++)
		{
			array<String^>^ files = Directory::GetFiles(folders[i]);

			for (int j = 0; j < files->Length; j++)
			{
				if (files[j]->Contains(".") == true)
				{
					array<Byte>^ data = File::ReadAllBytes(files[j]);

					if (data->Length == 88)
					{
						File::Delete(files[j]);
					}
					else
					{
						if (data->Length > 1)
						{
							if (data[0] == 0x50)
							{
								if (data[1] == 0x42)
								{
									Console::WriteLine(files[j] + ": ANIMATION");

									ConvertFile(data, files[j], ".pb", SkelAnim);
								}
								else if (data[1] == 0x43)
								{
									Console::WriteLine(files[j] + ": MODEL");

									File::WriteAllBytes(files[j] + ".pc", File::ReadAllBytes(files[j]));
								}
								else if (data[1] == 0x46)
								{
									Console::WriteLine(files[j] + ": ANIMATION");

									ConvertFile(data, files[j], ".pf", MatAnim);
								}
								else if (data[1] == 0x4B)
								{
									Console::WriteLine(files[j] + ": ANIMATION");

									ConvertFile(data, files[j], ".pk", VisAnim);
								}
								else if (data[1] == 0x54)
								{
									Console::WriteLine(files[j] + ": TEXTURE");

									File::WriteAllBytes(files[j] + ".pt", File::ReadAllBytes(files[j]));
								}
								else
								{
									for (int k = 0; k < data->Length - 2; k++)
									{
										if (data[k] == 0x42)
										{
											if (data[k + 1] == 0x43)
											{
												if (data[k + 2] == 0x48)
												{
													if (k != 0)
													{
														Console::WriteLine(files[j] + " is a file of an unknown type, but it seems to contain BCH data.\nAttempting to extract it.");
														ConvertFile(data, files[j], ".bch", FileType::Other);

														k = data->Length - 2;
													}
												}
											}
										}
									}
								}
							}
							else
							{
								for (int k = 0; k < data->Length - 2; k++)
								{
									if (data[k] == 0x42)
									{
										if (data[k + 1] == 0x43)
										{
											if (data[k + 2] == 0x48)
											{
												if (k != 0)
												{
													Console::WriteLine(files[j] + " is a file of an unknown type, but it seems to contain BCH data.\nAttempting to extract it.");
													ConvertFile(data, files[j], ".bch", FileType::Other);

													k = data->Length - 2;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	Console::WriteLine("\nAll files converted. Press any key to exit...");
	Console::ReadKey(true);

	return 0;
}