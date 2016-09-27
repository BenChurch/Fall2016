#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// atof
#include <string>
#include <vector>
#include <Windows.h>

using namespace std;

void ReadData(const char * dir, vector<array<double, 4>> TrainingBiometrics, vector<const char *> TrainingSpeciesNames, vector<array<double, 4>> TestBiometrics, vector<const char *> TestSpeciesNames);	// Both vectors will be populated by function and should be passed empty

class Perceptron
{
// Default constructors should work
public:


};

int main()
{
	vector<array<double, 4>> TrainingBiometrics;				// Petal width, Petal length, Sepal width, Sepal length
	vector<const char*> TrainingSpeciesNames;			// Either Setosa, Versicolour, or Virgina
	vector<array<double, 4>> TestBiometrics;	
	vector<const char*> TestSpeciesNames;
	const char* dir = "C:\\Users\\Ben\\Documents\\Masters16_17\\Fall2016\\CISC874\\Assignments\\Assignment1\\";		// data file location

	ReadData(dir, TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);

	ofstream Output("output.txt");				// This should probably be in an output function

	if (Output.is_open()) Output.close();
	cout << "Pres enter to end program." << endl;
	cin.ignore();
	return 0;
}

void ReadData(const char * dir, vector<array<double, 4>> TrainingBiometrics, vector<const char *> TrainingSpeciesNames, vector<array<double, 4>> TestBiometrics, vector<const char *> TestSpeciesNames)
{
	SetCurrentDirectoryA(dir);
	string line;
	std::string::size_type DatumEnd;
	string RawDatum;
	string RemainingData;
	array<double, 4> Biometrics;
	const char * SpeciesName;

	ifstream TrainingData("train.txt");
	if (TrainingData.is_open())
	{
		while (getline(TrainingData, line))		// Writes lines from .txt file into <string>line
		{
			DatumEnd = line.find(",");
			if (DatumEnd > 0)
			{
				RawDatum = (line.substr(0, DatumEnd));
				RemainingData = line.substr(DatumEnd+1, line.size());
				Biometrics[0] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[1] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[2] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[3] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				SpeciesName = RawDatum.c_str();

				// Debug output
				/*
				cout << "Training data:" << endl;
				for (int i = 0; i < 4; i++)
					cout << Biometrics[i] << "	";
				cout << SpeciesName << endl;
				*/

				TrainingBiometrics.push_back(Biometrics);
				TrainingSpeciesNames.push_back(SpeciesName);
			}
		}
		TrainingData.close();
	}
	else
	{
		cout << "Warning - could not open training data file." << endl;
		return;
	}

	ifstream TestData("test.txt");
	if (TestData.is_open())
	{
		while (getline(TestData, line))
		{
			DatumEnd = line.string::find(",");
			if (DatumEnd > 0)
			{
				RawDatum = (line.substr(0, DatumEnd));
				RemainingData = line.substr(DatumEnd + 1, line.size());
				Biometrics[0] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[1] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[2] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[3] = atof(RawDatum.c_str());

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				SpeciesName = RawDatum.c_str();

				// Debug output
				/*
				cout << "Training data:" << endl;
				for (int i = 0; i < 4; i++)
				cout << Biometrics[i] << "	";
				cout << SpeciesName << endl;
				*/

				TestBiometrics.push_back(Biometrics);
				TestSpeciesNames.push_back(SpeciesName);
			}
		}
		TestData.close();
	}
	else
	{
		cout << "Warning - could not open test data file." << endl;
		return;
	}
}