#include<iostream>
#include<string>
#include<fstream>
#include<Shlwapi.h>
#include<map>
#include<math.h>
#include<Windows.h>

using namespace std;

wchar_t* getFolderPath(int argc, char* argv[])
{
	wchar_t* defaultPath = TEXT("C:\\Windows\\System32");

	if (argc == 2)
	{
		char* givenFolderPath = argv[1];
		size_t length = strlen(givenFolderPath) + 1;
		wchar_t* givenFolderPathW = new wchar_t[length];
		size_t widePathSize;
		mbstowcs_s(&widePathSize, givenFolderPathW, length, givenFolderPath, length - 1);
		return givenFolderPathW;
	}

	return defaultPath;
}

BOOL isDirectoryPresent(LPCWSTR wFolderPath)
{
	return PathIsDirectory(wFolderPath);
}

BOOL isDirectoryEmpty(LPCWSTR wFolderPath)
{
	return PathIsDirectoryEmpty(wFolderPath);
}

double calculateFileEntropy(LPCWSTR filePath)
{
	streampos fileSize;
	double size;
	char *fileBuffer;
	double fileEntropy = 0;
	map<char, long> occurence;

	ifstream fileStream(filePath, ios::in | ios::binary | ios::ate);
	if (!fileStream.is_open())
	{
		wcout << "Error Opening File " << filePath << endl;
		return 0;
	}

	fileSize = fileStream.tellg();
	size = static_cast<double>(fileSize);
	fileBuffer = new char[fileSize];
	fileStream.seekg(0, ios::beg);
	fileStream.read(fileBuffer, fileSize);
	fileStream.close();
	
	for (size_t i = 0; i < fileSize; i++)
	{
		occurence[fileBuffer[i]] = occurence[fileBuffer[i]] + 1;
	}

	double probabilityOfByte = 0;
	for (pair<char, long> fileEntry : occurence)
	{
		probabilityOfByte = fileEntry.second / size;
		fileEntropy = fileEntropy + (probabilityOfByte * log2(probabilityOfByte));
	}

	fileEntropy = fileEntropy * -1;

	return fileEntropy;
}

void getEntropy(LPCWSTR wFolderPath)
{
	HANDLE fileHandle;
	WIN32_FIND_DATA fileData;
	double entropy, minEntropy, maxEntropy, entropyCutoff;
	double totalExeEntropy, totalDllEntropy, averageExeEntropy, averageDllEntropy;
	int exeFilesCount, dllFilesCount;
	int counter;
	map<wstring, double> entropyListExe;
	map<wstring, double> entropyListDll;
	
	entropyCutoff = minEntropy = maxEntropy = -1;
	exeFilesCount = dllFilesCount = 0;
	counter = 1;
	averageExeEntropy = averageDllEntropy = totalDllEntropy = totalExeEntropy = 0;

	fileHandle = FindFirstFile((wstring(wFolderPath) + TEXT("\\*.exe")).c_str(), &fileData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			entropy = calculateFileEntropy((wstring(wFolderPath) + TEXT("\\") + fileData.cFileName).c_str());
			if (minEntropy == -1)
			{
				minEntropy = maxEntropy = entropy;
			}
			else 
			{
				if (entropy < minEntropy)
				{
					minEntropy = entropy;
				}

				if (entropy > maxEntropy)
				{
					maxEntropy = entropy;
				}
			}

			entropyListExe[fileData.cFileName] = entropy;
			exeFilesCount++;
		} while (FindNextFile(fileHandle, &fileData));
	}

	fileHandle = FindFirstFile((wstring(wFolderPath) + TEXT("\\*.dll")).c_str(), &fileData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			entropy = calculateFileEntropy((wstring(wFolderPath) + TEXT("\\") + fileData.cFileName).c_str());
			if (minEntropy == -1)
			{
				minEntropy = maxEntropy = entropy;
			}
			else
			{
				if (entropy < minEntropy)
				{
					minEntropy = entropy;
				}

				if (entropy > maxEntropy)
				{
					maxEntropy = entropy;
				}
			}

			entropyListDll[fileData.cFileName] = entropy;
			dllFilesCount++;
		} while (FindNextFile(fileHandle, &fileData));
	}
	
	entropyCutoff = maxEntropy * 0.9;

	cout << endl << "Maximum Entropy Encountered : " << maxEntropy << endl;
	cout << "Minimum Entropy Encountered : " << minEntropy << endl;

	cout << endl << "Entropy Cutoff value : " << entropyCutoff << endl;
	
	cout << endl << "List of EXE files with top 10% entropy" << endl;
	for (map<wstring, double>::iterator iterator = entropyListExe.begin(); 
		iterator != entropyListExe.end(); iterator++)
	{
		double currentEntropy = iterator->second;
		if (currentEntropy >= entropyCutoff)
		{
			wcout << counter << TEXT(". ") << iterator->first << TEXT(" : ") << currentEntropy << endl;
			counter++;
		}
		totalExeEntropy = totalExeEntropy + currentEntropy;
	}
	averageExeEntropy = totalExeEntropy / exeFilesCount;

	counter = 1;

	cout << endl << "List of DLL files with top 10% entropy" << endl;
	for (map<wstring, double>::iterator iterator = entropyListDll.begin();
		iterator != entropyListDll.end(); iterator++)
	{
		double currentEntropy = iterator->second;
		if (currentEntropy >= entropyCutoff)
		{
			wcout << counter << TEXT(". ") << iterator->first << TEXT(" : ") << currentEntropy << endl;
			counter++;
		}
		totalDllEntropy = totalExeEntropy + currentEntropy;
	}
	averageDllEntropy = totalDllEntropy / dllFilesCount;

	cout << endl << "Total EXE Files found : " << exeFilesCount << endl;
	cout << "Total DLL Files found : " << dllFilesCount << endl;
	cout << endl << "Average Entropy of EXE files is : " << averageExeEntropy << endl;
	cout << "Average Entropy of DLL files is : " << averageDllEntropy << endl;
	cout << endl << "Average Entropy over both types : " << (totalDllEntropy + totalExeEntropy) /
															(exeFilesCount + dllFilesCount) << endl;
}

int main(int argc, char* argv[])
{
	wchar_t* wFolderPath = getFolderPath(argc, argv);

	BOOL isDirectory = isDirectoryPresent(wFolderPath);
	if (!isDirectory)
	{
		cout << "Specified Directory is not Found." << endl;
		getchar();
		return -1;
	}

	BOOL isEmpty = isDirectoryEmpty(wFolderPath);
	if (isEmpty)
	{
		cout << "Specified Directory is Empty." << endl;
		getchar();
		return -1;
	}

	cout << "Calculating Entropy." << endl;
	wcout << TEXT("Folder Path: ") << wFolderPath << endl;

	getEntropy(wFolderPath);

	cout << "Press Any Key to Terminate";
	getchar();
	return 0;
}
