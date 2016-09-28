#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

static const int MAX_TRAINING_EPOCHS = 1000;			// Need to allow for non-linearly seperable data
//typedef struct Input { vector<array<double, 4>> TrB; vector<const char*> TrS; vector<array<double, 4>> TeB; vector<const char*> TeS; } Input;
//typedef struct Input Input;
using namespace std;

class InputData
{
public:
	InputData() {};
	InputData(vector<array<double, 4>> TrB, vector<string> TrS, vector<array<double, 4>> TeB, vector<string> TeS)
	{
		this->TrainingBiometrics = TrB;
		this->TrainingSpeciesNames = TrS;
		this->TestBiometrics = TeB;
		this->TestSpeciesNames = TeS;
	};
	vector<array<double, 4>> TrainingBiometrics;
	vector<string> TrainingSpeciesNames;
	vector<array<double, 4>> TestBiometrics;
	vector<string> TestSpeciesNames;
};

InputData ReadData(const char * dir);	// Both vectors will be populated by function and should be passed empty

class Perceptron
{
	// Default constructors should work
public:
	double Weights[5];									// Can be arbitrarily set to 5 for this problem, with w0 = threshold
	double Inputs[5];
	double ActivationPotential = 0;
	double LearningRate = 1;							// Try 1, doesn't have to be 1

	void InitializeWeights();
	void TrainForSetosa_Otherwise(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForSetosa_Otherwise(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);
};


int main()
{
	const char* dir = "C:\\Users\\Ben\\Documents\\Masters16_17\\Fall2016\\CISC874\\Assignments\\Assignment1\\";		// data file location
	
	InputData ID;
	ID = (ReadData(dir));
	//ID = ReadData(dir);

	vector<array<double, 4>> TrainingBiometrics = ID.TrainingBiometrics;				// Petal width, Petal length, Sepal width, Sepal length
	vector<string> TrainingSpeciesNames = ID.TrainingSpeciesNames;					// Either Setosa, Versicolour, or Virgina
	vector<array<double, 4>> TestBiometrics = ID.TestBiometrics;
	vector<string> TestSpeciesNames = ID.TestSpeciesNames;

	Perceptron SetosaOrNot;
	SetosaOrNot.InitializeWeights();
	SetosaOrNot.TrainForSetosa_Otherwise(TrainingBiometrics, TrainingSpeciesNames);
	SetosaOrNot.TestForSetosa_Otherwise(TestBiometrics, TestSpeciesNames);

	ofstream Output("output.txt");				// This should probably be in an output function

	if (Output.is_open()) Output.close();
	cout << "Pres enter to end program." << endl;
	cin.ignore();
	return 0;
}

InputData ReadData(const char * dir)
{
	// Declare data containers
	vector<array<double, 4>> TrainingBiometrics;
	vector<string> TrainingSpeciesNames;
	vector<array<double, 4>> TestBiometrics;
	vector<string> TestSpeciesNames;

	SetCurrentDirectoryA(dir);
	string line;
	std::string::size_type DatumEnd;
	string RawDatum;
	string RemainingData;
	array<double, 4> Biometrics;
	//char * SpeciesName;

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
				//SpeciesName = RawDatum.c_str();

				// Debug output
				/*
				cout << "Training data:" << endl;
				for (int i = 0; i < 4; i++)
					cout << Biometrics[i] << "	";
				cout << RawDatum.c_str() << endl;
				*/

				TrainingBiometrics.push_back(Biometrics);
				TrainingSpeciesNames.push_back(RawDatum);
			}
		}
		TrainingData.close();
	}
	else
	{
		cout << "Warning - could not open training data file." << endl;
		cout << "	Returning empty data structure." << endl;
		InputData ID(TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);
		return ID;
	}

	ifstream TestData("test.txt");
	if (TestData.is_open())
	{
		while (getline(TestData, line))
		{
			DatumEnd = line.find(",");
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
				//SpeciesName = RawDatum.c_str();

				// Debug output
				/*
				cout << "Test data:" << endl;
				for (int i = 0; i < 4; i++)
				cout << Biometrics[i] << "	";
				cout << RawDatum.c_str() << endl;
				*/

				TestBiometrics.push_back(Biometrics);
				TestSpeciesNames.push_back(RawDatum);
			}
		}
		TestData.close();
	}
	else
	{
		cout << "Warning - could not open test data file." << endl;
		cout << "	Returning empty data structure." << endl;
		InputData ID(TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);
		return ID;
	}
	InputData ID(TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);
	return ID;
}

void Perceptron::InitializeWeights()
{
	srand(time(NULL));
	int RandomInt;										
	double RandomDouble;

	cout << "Initial random wieghts: " << endl;
	for (int i = 0; i < 5; i++)
	{
		RandomInt = (rand() % 200) - 100;									// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		this->Weights[i] = RandomDouble;
		cout << this->Weights[i] << "	";
	}
	cout << endl;
}

void Perceptron::TrainForSetosa_Otherwise(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
	this->Inputs[0] = 1;
	double InputVectorLength = 0;													// Leave Inputs[0] out of normalization
	// Assume Iris-setosa is class 1, otherwise is class -1
	int CurrentEpoch = 0;
	while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)			
	{
		this->ActivationPotential = 0;												// Reinitialize activation potential for next pattern
		IncorrectlyClassified = TrainingBiometrics.size();							// New epoch, reinitialize IncorrectlyClassified count
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)							// For all the training patterns
		{
			// (Re)Initialize input vector
			for (int j = 1; j < 5; j++)												// For each biometric in the pattern
			{
				this->Inputs[j] = TrainingBiometrics[i][j - 1];
				InputVectorLength += (this->Inputs[j])*(this->Inputs[j]);
			}

			// Normalize input vector
			InputVectorLength = sqrt(InputVectorLength);
			for (int j = 1; j < 5; j++)
				this->Inputs[j] = this->Inputs[j] / InputVectorLength;

			for (int j = 0; j < 5; j++)												// For each input-weight pair
				this->ActivationPotential += this->Inputs[j] * this->Weights[j];

			// Do I need an activation function? A threshold maybe?
			if (this->ActivationPotential > 0)										// Output corresponds to Iris-setosa
			{
				if (TrainingSpeciesNames[i] != "Iris-setosa")						// Input is miscorrectly classified, output is too large
				{
					for (int j = 0; j < 5; j++)
					{
						this->Weights[j] -= this->LearningRate * this->Inputs[j];
					}
				}
				else
					IncorrectlyClassified--;
				//cout << "Remaining input patterns to be classified in epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
			}
			else																	// Output corresponds to Versicolour or Virginia
			{
				if (TrainingSpeciesNames[i] == "Iris-setosa")						// Input is miscorrectly classidied, output is too small
				{
					for (int j = 0; j < 5; j++)
					{
						this->Weights[j] += this->LearningRate * this->Inputs[j];
					}
				}
				else
					IncorrectlyClassified--;
				//cout << "Remaining input patterns to be classified in epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
			}
		}
		cout << "Incorrectly classified inputs after epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
		CurrentEpoch++;																// Guarantee training termination
	}

	cout << "Training complete!" << endl << "	Final weight vector: W = [";
	for (int i = 0; i < 5; i++)
	{
		cout << this->Weights[i];
		if (i != 4) cout << ", ";
		else cout << "]" << endl;
	}
}

void Perceptron::TestForSetosa_Otherwise(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{
	for (int i = 0; i < TestBiometrics.size(); i++)							// Run test on all input patterns
	{
		this->ActivationPotential = 0;
		// (Re) Initialize inputs
		for (int j = 1; j < 5; j++)											// Input[0] = 1
			this->Inputs[j] = TestBiometrics[i][j - 1];

		// (Re)Compute activation potential
		for (int j = 0; j < 5; j++)
			this->ActivationPotential += this->Inputs[j] * this->Weights[j];

		if (this->ActivationPotential > 0)									// Input is classified as Iris-setosa
		{
			cout << "Input pattern " << i << " classified as: Iris-setosa." << endl;
			cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
		}
		else																// Input classified as NOT Iris-setosa
		{
			cout << "Input pattern " << i << " classified as: NOT Iris-setosa." << endl;
			cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
		}
	}
}