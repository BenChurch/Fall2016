#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

static const int MAX_TRAINING_EPOCHS = 1000;			// Need to allow for non-linearly seperable data
static const char * dir = "C:\\Users\\Ben\\Documents\\Masters16_17\\Fall2016\\CISC874\\Assignments\\Assignment1\\";

using namespace std;

class InputData			// Used for tidiness in reading data
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

InputData ReadData();	// Both vectors will be populated by function and should be passed empty




class Perceptron
{
	// Default constructors should work
public:
	double SetosaOrNotWeights[5];									// Can be arbitrarily set to 5 for this problem, with w0 = threshold
	double VersicolorOrVirginiaWeights[5];
	double Inputs[5];
	double ActivationPotential = 0;
	double LearningRate = 1;							// Try 1, doesn't have to be 1

	ofstream Output;
	void WriteData(string OutputData);
	void WriteData(vector<string> OutputData);

	void InitializeWeights();
	void NormalizeWeights();							
	void TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForSetosaOrNot(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);
	void TrainForVersicolourOrVirginia(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForVersicolourOrVirginia(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);
};


int main()
{
	InputData ID;
	ID = (ReadData());
	
	vector<string> OutputData;

	vector<array<double, 4>> TrainingBiometrics = ID.TrainingBiometrics;				// Petal width, Petal length, Sepal width, Sepal length
	vector<string> TrainingSpeciesNames = ID.TrainingSpeciesNames;					// Either Setosa, Versicolour, or Virgina
	vector<array<double, 4>> TestBiometrics = ID.TestBiometrics;
	vector<string> TestSpeciesNames = ID.TestSpeciesNames;

	Perceptron IrisIdentifier;
	IrisIdentifier.InitializeWeights();
	IrisIdentifier.TrainForSetosaOrNot(TrainingBiometrics, TrainingSpeciesNames);
	IrisIdentifier.TestForSetosaOrNot(TestBiometrics, TestSpeciesNames);

	IrisIdentifier.Output.close();

	std::cout << "Pres enter to end program." << endl;
	cin.ignore();
	return 0;
}

InputData ReadData()
{
	SetCurrentDirectoryA(dir);
	// Declare data containers
	vector<array<double, 4>> TrainingBiometrics;
	vector<string> TrainingSpeciesNames;
	vector<array<double, 4>> TestBiometrics;
	vector<string> TestSpeciesNames;

	
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
		std::cout << "Warning - could not open training data file." << endl;
		std::cout << "	Returning empty data structure." << endl;
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
		std::cout << "Warning - could not open test data file." << endl;
		std::cout << "	Returning empty data structure." << endl;
		InputData ID(TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);
		return ID;
	}
	InputData ID(TrainingBiometrics, TrainingSpeciesNames, TestBiometrics, TestSpeciesNames);
	return ID;
}

void Perceptron::WriteData(string OutputData)
{
	SetCurrentDirectoryA(dir);
	if (!this->Output.is_open())
		Output.open("output.txt");

	Output << OutputData << endl;
	if (Output.is_open());
}

void Perceptron::WriteData(vector<string> OutputData)
{
	SetCurrentDirectoryA(dir);
	if (!this->Output.is_open())
		Output.open("output.txt");

	for (int i = 0; i < OutputData.size(); i++)
		Output << OutputData[i] << endl;
}

void Perceptron::InitializeWeights()
{
	string SetosaOrNotWeightStrings;
	string VersicolorOrVirginiaWeightStrings;
	srand(time(NULL));
	int RandomInt;										
	double RandomDouble;

	std::cout << "Initial random weights: " << endl;
	SetosaOrNotWeightStrings = "Initial random weights for setosa versus non-setosa classification: ";
	VersicolorOrVirginiaWeightStrings = "Initial random weights for versicolour versus virginia classification: ";
	for (int i = 0; i < 5; i++)
	{
		RandomInt = (rand() % 200) - 100;									// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		this->SetosaOrNotWeights[i] = RandomDouble;
		std::cout << this->SetosaOrNotWeights[i] << "	";
		SetosaOrNotWeightStrings = SetosaOrNotWeightStrings + to_string(this->SetosaOrNotWeights[i]) + "	";
	}

	for (int i = 0; i < 5; i++)
	{
		RandomInt = (rand() % 200) - 100;									
		RandomDouble = RandomInt / 100.0;										
		this->VersicolorOrVirginiaWeights[i] = RandomDouble;
		std::cout << this->VersicolorOrVirginiaWeights[i] << "	";
		VersicolorOrVirginiaWeightStrings = VersicolorOrVirginiaWeightStrings + to_string(this->VersicolorOrVirginiaWeights[i]) + "	";
	}

	WriteData(SetosaOrNotWeightStrings + " (where w0 is the threshold)");
	WriteData(VersicolorOrVirginiaWeightStrings + " (where w0 is the threshold)");
	std::cout << endl;
}

void Perceptron::NormalizeWeights()
{
	double SetosaOrNotWeightLength = 0;
	double VersicolourOrVirginiaWeightLength = 0;

	for (int i = 0; i < 5; i++)
	{
		SetosaOrNotWeightLength += (this->SetosaOrNotWeights[i]) * (this->SetosaOrNotWeights[i]);
		VersicolourOrVirginiaWeightLength += (this->VersicolorOrVirginiaWeights[i]) * (this->VersicolorOrVirginiaWeights[i]);
	}

	SetosaOrNotWeightLength = sqrt(SetosaOrNotWeightLength);
	VersicolourOrVirginiaWeightLength = sqrt(VersicolourOrVirginiaWeightLength);

	std::cout << "Weight length = " << SetosaOrNotWeightLength << endl;
	for (int i = 0; i < 5; i++)
		this->SetosaOrNotWeights[i] = this->SetosaOrNotWeights[i] / SetosaOrNotWeightLength;

}

void Perceptron::TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
	this->Inputs[0] = 1;
	double InputVectorLength = 0;													// Leave Inputs[0] out of normalization
	// Assume Iris-setosa is class 1, otherwise is class -1
	int CurrentEpoch = 0;
	string EpochsRequired = "Training epochs required to correctly classify all setosas versus non-setosas: ";	// For output.txt
	string FinalNormalizedWeights = "Normalized weight vector which classifies setosas versus non-setosas: ";
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
				this->ActivationPotential += this->Inputs[j] * this->SetosaOrNotWeights[j];

			// Do I need an activation function? A threshold maybe?
			if (this->ActivationPotential > 0)										// Output corresponds to Iris-setosa
			{
				if (TrainingSpeciesNames[i] != "Iris-setosa")						// Input is miscorrectly classified, output is too large
				{
					for (int j = 0; j < 5; j++)
					{
						this->SetosaOrNotWeights[j] -= this->LearningRate * this->Inputs[j];
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
						this->SetosaOrNotWeights[j] += this->LearningRate * this->Inputs[j];
					}
				}
				else
					IncorrectlyClassified--;
				//cout << "Remaining input patterns to be classified in epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
			}
		}
		std::cout << "Incorrectly classified inputs after epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
		CurrentEpoch++;																// Guarantee training termination
	}

	EpochsRequired = EpochsRequired + to_string(CurrentEpoch);
	this->WriteData(EpochsRequired);

	this->NormalizeWeights();
	for (int i = 0; i < 5; i++)
		FinalNormalizedWeights = FinalNormalizedWeights + "	" + to_string(this->SetosaOrNotWeights[i]);
	this->WriteData(FinalNormalizedWeights);

	/*
	std::cout << "Training complete!" << endl << "	Final weight vector: W = [";
	for (int i = 0; i < 5; i++)
	{
		std::cout << this->Weights[i];
		if (i != 4) std::cout << ", ";
		else std::cout << "]" << endl;
	}
	*/
}

void Perceptron::TestForSetosaOrNot(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{
	string InputPattern;
	int CorrectlyClassified = 0;
	for (int i = 0; i < TestBiometrics.size(); i++)							// Run test on all input patterns
	{
		InputPattern = "";													// Reinitialize to empty string
		this->ActivationPotential = 0;
		// (Re) Initialize inputs
		for (int j = 1; j < 5; j++)											// Input[0] = 1
		{
			this->Inputs[j] = TestBiometrics[i][j - 1];
			InputPattern = InputPattern + "	" + to_string(TestBiometrics[i][j - 1]);
		}

		// (Re)Compute activation potential
		for (int j = 0; j < 5; j++)
			this->ActivationPotential += this->Inputs[j] * this->SetosaOrNotWeights[j];

		if (this->ActivationPotential > 0)									// Input is classified as Iris-setosa
		{
			if (TestSpeciesNames[i] == "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
			std::cout << "Input pattern " << i << " classified as: Iris-setosa." << endl;
			std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
		}
		else																// Input classified as NOT Iris-setosa
		{
			if (TestSpeciesNames[i] != "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as NON Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
			std::cout << "Input pattern " << i << " classified as: NOT Iris-setosa." << endl;
			std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
		}
	}
	this->WriteData("Therefore the perceptron correctly distinguished " + to_string(CorrectlyClassified) + " samples out of " + to_string(TestSpeciesNames.size()) + " as being Iris-setosa or not.");
}

void Perceptron::TrainForVersicolourOrVirginia(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{

}

void Perceptron::TestForVersicolourOrVirginia(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{

}