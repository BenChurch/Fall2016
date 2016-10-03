#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

static const int MAX_TRAINING_EPOCHS = 100;			// Need to allow for non-linearly seperable data
static const char * dir = "C:\\Users\\Ben\\Documents\\Masters16_17\\Fall2016\\CISC874\\Assignments\\Assignment1";

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
	//double SetosaOrNotWeights[5];									// Can be arbitrarily set to 5 for this problem, with w0 = threshold
	//double VersicolorOrVirginicaWeights[2];							// [0] for threshold, [1] for one of 4 input dimensions
	//vector<double> Weights;
	double SInputs[5];
	double SWeights[5];										// To be learned by perceptron for setosa classification and passed to perceptrons for versicolor/virginica classification
	double SActivationPotential = 0;

	double VInputs[4];
	double VWeights[5][2];								// A threshold + weight for each input dimension, then one more to take them all into consideration
	double VActivationPotential[5];

	//double Inputs[5];
	//double ActivationPotential = 0;
	double LearningRate = 0.5;							// Try 1, doesn't have to be 1
	double ClassificationPolarization = 1;				// During error correction learning, perceptron tries to map inputs of different classes to (-/+)ClassificationPolarization 

	ofstream Output;
	void WriteData(string OutputData);
	void WriteData(vector<string> OutputData);

	void InitializeWeights();
	void NormalizeWeights();							
	void TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForSetosaOrNot(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);
	void TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForVersicolorOrVirginica(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);

	void TrainForVersicolorOrVirginica(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames, int Dimension);

	bool ClassifySetosaOrNot(array<double, 4> Inputs, string CorrectName);
	bool ClassifyVersicolorOrVirginica(array<double, 4> Inputs, string CorrectName);
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

	IrisIdentifier.TrainForVersicolorOrVirginica(TrainingBiometrics, TrainingSpeciesNames);
	IrisIdentifier.TestForVersicolorOrVirginica(TrainingBiometrics, TrainingSpeciesNames);
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

	// Used to normalize input dimensions
	double Col0Sum = 0;
	double Col0Avg;
	double Col1Sum = 0;
	double Col1Avg;
	double Col2Sum = 0;
	double Col2Avg;
	double Col3Sum = 0;
	double Col3Avg;
	
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
				Col0Sum += Biometrics[0];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[1] = atof(RawDatum.c_str());
				Col1Sum += Biometrics[1];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[2] = atof(RawDatum.c_str());
				Col2Sum += Biometrics[2];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[3] = atof(RawDatum.c_str());
				Col3Sum += Biometrics[3];

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
		Col0Avg = Col0Sum / TrainingBiometrics.size();
		Col1Avg = Col1Sum / TrainingBiometrics.size();
		Col2Avg = Col2Sum / TrainingBiometrics.size();
		Col3Avg = Col3Sum / TrainingBiometrics.size();
		for (int i = 0; i < TrainingBiometrics.size(); i++)
		{
	//		TrainingBiometrics[i][0] = TrainingBiometrics[i][0] / Col0Avg;
	//		TrainingBiometrics[i][1] = TrainingBiometrics[i][1] / Col1Avg;
	//		TrainingBiometrics[i][2] = TrainingBiometrics[i][2] / Col2Avg;
	//		TrainingBiometrics[i][3] = TrainingBiometrics[i][3] / Col3Avg;
		}
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
		Col0Sum = 0;
		Col1Sum = 0;
		Col2Sum = 0;
		Col3Sum = 0;
		while (getline(TestData, line))
		{
			DatumEnd = line.find(",");
			if (DatumEnd > 0)
			{
				RawDatum = (line.substr(0, DatumEnd));
				RemainingData = line.substr(DatumEnd + 1, line.size());
				Biometrics[0] = atof(RawDatum.c_str());
				Col0Sum += Biometrics[0];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[1] = atof(RawDatum.c_str());
				Col1Sum += Biometrics[1];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[2] = atof(RawDatum.c_str());
				Col2Sum += Biometrics[2];

				DatumEnd = RemainingData.find(",");
				RawDatum = (RemainingData.substr(0, DatumEnd));
				RemainingData = RemainingData.substr(DatumEnd + 1, RemainingData.size());
				Biometrics[3] = atof(RawDatum.c_str());
				Col3Sum += Biometrics[3];

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
		Col0Avg = Col0Sum / TestBiometrics.size();
		Col1Avg = Col1Sum / TestBiometrics.size();
		Col2Avg = Col2Sum / TestBiometrics.size();
		Col3Avg = Col3Sum / TestBiometrics.size();
		for (int i = 0; i < TestBiometrics.size(); i++)
		{
		//	TestBiometrics[i][0] = TestBiometrics[i][0] / Col0Avg;
		//	TestBiometrics[i][1] = TestBiometrics[i][1] / Col1Avg;
		//	TestBiometrics[i][2] = TestBiometrics[i][2] / Col2Avg;
		//	TestBiometrics[i][3] = TestBiometrics[i][3] / Col3Avg;
		}
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
}

void Perceptron::WriteData(vector<string> OutputData)
{
	SetCurrentDirectoryA(dir);
	if (!this->Output.is_open())
		Output.open("output.txt");

	for (unsigned int i = 0; i < OutputData.size(); i++)
		Output << OutputData[i] << endl;
}

void Perceptron::InitializeWeights()
{
	string SWeightsString;
	//string VersicolorOrVirginiaWeightStrings;
	srand(time(NULL));
	int RandomInt;										
	double RandomDouble;

	//std::cout << "Initial random setosa - NON setosa weights (including w0 = threshold): " << endl;
	//SetosaOrNotWeightStrings = "Initial random weights for setosa versus non-setosa classification: ";
	//VersicolorOrVirginiaWeightStrings = "Initial random weights for versicolor versus virginica classification: ";
	for (int i = 0; i < 5; i++)
	{
		RandomInt = (rand() % 200) - 100;									// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		//this->SetosaOrNotWeights[i] = RandomDouble;
		this->SWeights[i] = RandomDouble;
		std::cout << this->SWeights[i] << "	";
		SWeightsString = SWeightsString + to_string(this->SWeights[i]) + "	";
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			RandomInt = (rand() % 200) - 100;
			RandomDouble = RandomInt / 100.0;
			this->VWeights[i][j] = RandomDouble;
		}
	}

	/*
	std::cout << endl << "Initial random versicolor - virginica weights (including w0 = threshold): " << endl;
	for (int i = 0; i < 5; i++)
	{
		RandomInt = (rand() % 200) - 100;									
		RandomDouble = RandomInt / 100.0;										
    this->VersicolorOrVirginicaWeights[i] = RandomDouble;
    std::cout << this->VersicolorOrVirginicaWeights[i] << "	";
    VersicolorOrVirginiaWeightStrings = VersicolorOrVirginiaWeightStrings + to_string(this->VersicolorOrVirginicaWeights[i]) + "	";
	}

	WriteData(SetosaOrNotWeightStrings + " (where w0 is the threshold)");
	WriteData(VersicolorOrVirginiaWeightStrings + " (where w0 is the threshold)");
	std::cout << endl;
	*/
}

void Perceptron::NormalizeWeights()
{
	double SWeightVectorLength = 0;
	double VWeightVectorLength = 0;
	//double VersicolourOrVirginiaWeightLength = 0;

	for (int i = 1; i < 5; i++)
	{
		SWeightVectorLength += (this->SWeights[i]) * (this->SWeights[i]);
		VWeightVectorLength += (this->VWeights[i][1]) * (this->VWeights[i][1]);						// This amounts to normalizing across all input dimensions
		//VersicolourOrVirginiaWeightLength += (this->VersicolorOrVirginicaWeights[i]) * (this->VersicolorOrVirginicaWeights[i]);
	}

	SWeightVectorLength = sqrt(SWeightVectorLength);
	VWeightVectorLength = sqrt(VWeightVectorLength);
	//VersicolourOrVirginiaWeightLength = sqrt(VersicolourOrVirginiaWeightLength);

	for (int i = 1; i < 5; i++)
  {
	  this->SWeights[i] = this->SWeights[i] / SWeightVectorLength;
	  this->VWeights[i][1] = this->VWeights[i][1] / VWeightVectorLength;
		//this->VersicolorOrVirginicaWeights[i] = this->VersicolorOrVirginicaWeights[i] / VersicolourOrVirginiaWeightLength;
  }
}

void Perceptron::TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
	this->SInputs[0] = 1;
	// Assume Iris-setosa is +ve class, otherwise is -ve class
	int CurrentEpoch = 0;
	string EpochsRequired = "Training epochs required to correctly classify all setosas versus non-setosas: ";	// For output.txt
	string FinalNormalizedWeights = "Normalized weight vector which classifies setosas versus non-setosas: ";
	while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)			
	{
		IncorrectlyClassified = TrainingBiometrics.size();							// New epoch, reinitialize IncorrectlyClassified count
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)		// For all the training patterns
		{
			if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))					// returns true for Iris-setosa
			{
				if (TrainingSpeciesNames[i] == "Iris-setosa")
				{
					IncorrectlyClassified--;
				}
				else
				{
					for (int j = 1; j < 5; j++)
						this->SWeights[j] -= this->LearningRate * this->SInputs[j];
				}
			}
			else
			{
				if (TrainingSpeciesNames[i] != "Iris-setosa")
				{
					IncorrectlyClassified--;
				}
				else
				{
					for (int j = 1; j < 5; j++)
						this->SWeights[j] += this->LearningRate * this->SInputs[j];
				}
			}
			this->NormalizeWeights();
		}
		//std::cout << "Incorrectly classified inputs after epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
		CurrentEpoch++;																// Guarantee training termination
	}

	EpochsRequired = EpochsRequired + to_string(CurrentEpoch);
	this->WriteData(EpochsRequired);
	
	for (int i = 0; i < 5; i++)
		FinalNormalizedWeights = FinalNormalizedWeights + "	" + to_string(this->SWeights[i]);

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
	for (unsigned int i = 0; i < TestBiometrics.size(); i++)							// Run test on all input patterns
	{
		InputPattern = "";													// Reinitialize to empty string
		for (int j = 1; j < 5; j++)											
			InputPattern = InputPattern + "	" + to_string(TestBiometrics[i][j - 1]);

		if (this->ClassifySetosaOrNot(TestBiometrics[i], TestSpeciesNames[i]))									// Input is classified as Iris-setosa
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

void Perceptron::TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames, int Dimension)
{
	int IncorrectlyClassified = TrainingBiometrics.size();
	this->VInputs[Dimension][0] = 1;
	int CurrentEpoch = 0;
	string EpochsRequired = "Training epochs required to correctly classify all versicolor versus virginica: ";	// For output.txt

	while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)
	{
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)											// For all input patterns
		{
			IncorrectlyClassified = TrainingBiometrics.size();													// Must reclassify all input patters
			if (this -> ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))
			{
				if (TrainingSpeciesNames[i] == "Iris-setosa")
					IncorrectlyClassified--;
			}
			else
			{
				if (this->ClassifyVersicolorOrVirginica(TrainingBiometrics[i], TrainingSpeciesNames[i]))
				{

				}
			}
		}
	}
}

void Perceptron::TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
  int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
  int MinMisclassified = IncorrectlyClassified;
  double RollbackWeights[5];														// Will store weights of greatest classification accuracy, so they aren't replaced
  for (int j = 0; j < 5; j++)
	  RollbackWeights[j] = this->VersicolorOrVirginicaWeights[j];
  this->Inputs[0] = 1;														
  // Assume Iris-Versicolor is class 1, Iris-Virginica is class -1
  int CurrentEpoch = 0;
  string EpochsRequired = "Training epochs required to correctly classify all versicolor versus virginica: ";	// For output.txt
  string FinalNormalizedWeights = "Normalized weight vector which classifies versicolor versus virginica: ";

  while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)
  {
    IncorrectlyClassified = TrainingBiometrics.size();                  // New epoch, must reclassify all input patterns
    for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)         // Run through all patterns again
    {                        
		  if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))                                // Input classified as Iris-setosa - already learned, but now use error correction
		  {
			  if (TrainingSpeciesNames[i] == "Iris-setosa")
				  IncorrectlyClassified--;
		  }
		  else                                                              // Input is classified as either versicolor or virginica, we don't know which yet           
		  {
			if (TrainingSpeciesNames[i] != "Iris-setosa")                   // So we don't accidentally train off a mis-classified Iris-setosa
			{
				if (this->ClassifyVersicolorOrVirginica(TrainingBiometrics[i], TrainingSpeciesNames[i]))            // Input classified as Versicolour
				{
					if (TrainingSpeciesNames[i] == "Iris-versicolor")          // If input is correctly classified
					{
						IncorrectlyClassified--;
						for (int j = 1; j < 5; j++)
							this->VersicolorOrVirginicaWeights[j] += (this->ClassificationPolarization - this->ActivationPotential) * this->LearningRate * this->Inputs[j];
						cout << "Error on input pattern " << i << ":	" << to_string(this->ClassificationPolarization - this->ActivationPotential) << endl;
					}
					else                                                        // Input is too large for Virginica
					{
						for (int j = 1; j < 5; j++)                               // Adjust weight vector
							this->VersicolorOrVirginicaWeights[j] += ((-1 * this->ClassificationPolarization) - this->ActivationPotential) * this->Inputs[j] * this->LearningRate;
						cout << "Error on input pattern " << i << ":	" << to_string((-1 * this->ClassificationPolarization) - this->ActivationPotential) << endl;
					}
				}
				else																			// Input classified as virginica
				{
					if (TrainingSpeciesNames[i] == "Iris-virginica")
					{
						IncorrectlyClassified--;
						for (int j = 1; j < 5; j++)
						{
							this->VersicolorOrVirginicaWeights[j] += ((-1 * this->ClassificationPolarization) - this->ActivationPotential) * this->LearningRate * this->Inputs[j];
						}
						cout << "Error on input pattern " << i << ":	" << to_string((-1*this->ClassificationPolarization) - this->ActivationPotential) << endl;
					}
					else
					{
						for (int j = 1; j < 5; j++)
						{
							this->VersicolorOrVirginicaWeights[j] += (this->ClassificationPolarization - this->ActivationPotential) * this->Inputs[j] * this->LearningRate;
						}
						cout << "Error on input pattern " << i << ":	" << to_string(this->ClassificationPolarization - this->ActivationPotential) << endl;
					}
				}
			}
		}
	  this->NormalizeWeights();
	}
	if (MinMisclassified < IncorrectlyClassified)						// The weights we wound up with after this iteration perform worse
	{
		//for (int j = 0; j < 5; j++)
		//	this->VersicolorOrVirginicaWeights[j] = RollbackWeights[j];
	}
	else
	{
		MinMisclassified = IncorrectlyClassified;
		for (int j = 0; j < 5; j++)
			RollbackWeights[j] = this->VersicolorOrVirginicaWeights[j];
	}
	
    std::cout << "Incorrectly classified inputs after epoch " << CurrentEpoch << ": " << IncorrectlyClassified << " (for versicolor-virginica training)" << endl;
    CurrentEpoch++;
	//this->LearningRate = this->LearningRate*0.5;
  }
  for (int j = 0; j < 5; j++)
	  this->VersicolorOrVirginicaWeights[j] = RollbackWeights[j];
  std::cout << "Versicolour-Virginia training complete!" << endl;
  std::cout << "  Incorrectly classified inputs after " << CurrentEpoch << " training epochs: " << IncorrectlyClassified << endl;
  std::cout << "  Resulting normalized weight vector for versicolor-virginica classification: " << endl;
  for (int j = 0; j < 5; j++)
    std::cout << "  " << this->VersicolorOrVirginicaWeights[j];
  std::cout << endl;

}

void Perceptron::TestForVersicolorOrVirginica(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{
  string InputPattern;
  int CorrectlyClassified = 0;
  for (unsigned int i = 0; i < TestBiometrics.size(); i++)				// Run test on all input patterns
  {
    InputPattern = "";													// Reinitialize to empty string
    // (Re) Initialize inputs
    for (int j = 1; j < 5; j++)											// Input[0] = 1
      InputPattern = InputPattern + "	" + to_string(TestBiometrics[i][j - 1]);

    if (this->ClassifySetosaOrNot(TestBiometrics[i], TestSpeciesNames[i]))		// Input is classified as Iris-setosa
    {
      if (TestSpeciesNames[i] == "Iris-setosa")
        CorrectlyClassified++;
      this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
      //std::cout << "Input pattern " << i << " classified as: Iris-setosa." << endl;
      //std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
    }
    else																// Input classified as NOT Iris-setosa
    {
      if (TestSpeciesNames[i] != "Iris-setosa")
      {
        if (this->ClassifyVersicolorOrVirginica(TestBiometrics[i], TestSpeciesNames[i]))
        {
          if (TestSpeciesNames[i] == "Iris-versicolor")
            CorrectlyClassified++;
          this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-versicolor. Correct classification is: " + TestSpeciesNames[i]);
          //std::cout << "Input pattern " << i << " classified as: Iris-versicolor." << endl;
         // std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
        }
        else
        {
          if (TestSpeciesNames[i] == "Iris-virginica")
            CorrectlyClassified++;
          this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-virginica. Correct classification is: " + TestSpeciesNames[i]);
          //std::cout << "Input pattern " << i << " classified as: Iris-virginica." << endl;
          //std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
        }
      }
	  else
	  {
		  this->WriteData("Pereptron classified input pattern " + to_string(i) + ":" + InputPattern + " incorrectly as NOT Iris-setosa");
	  }
    }
  }
  this->WriteData("Therefore the perceptron correctly distinguished " + to_string(CorrectlyClassified) + " samples out of " + to_string(TestSpeciesNames.size()) + " as being Iris-setosa or not.");
}

bool Perceptron::ClassifySetosaOrNot(array<double, 4> Inputs, string CorrectClassification)
{
	double InputVectorLength = 0;									// Used for input vector normalization
	this->SActivationPotential = 0;									// Reinitialize for new classification
	for (unsigned int j = 1; j < 5; j++)
	{
		this->SInputs[j] = Inputs[j - 1];
		InputVectorLength += (this->SInputs[j] * this->SInputs[j]);
	}
	InputVectorLength = sqrt(InputVectorLength);
	for (unsigned int j = 1; j <5; j++)
	{
		this->SInputs[j] = this->SInputs[j] / InputVectorLength;
		this->SActivationPotential += this->SInputs[j] * this->SWeights[j];
	}
	
	if (this->SActivationPotential > this->SWeights[0])
		return true;
	else return false;
}

bool Perceptron::ClassifyVersicolorOrVirginica(array<double, 4> Inputs, string CorrectName)			// Returns true for versicolor classification
{
	double InputVectorLength = 0;
	this->VActivationPotential[5] = 0;
	for (int j = 0; j < 4; j++)
	{
		this->VActivationPotential[j] = 0;
		this->VInputs[j][1] = Inputs[j - 1];
		InputVectorLength += (this->VInputs[j][1] * this->VInputs[j][1]);
	}
	InputVectorLength = sqrt(InputVectorLength);
	for (int j = 0; j < 4; j++)
	{
		this->VInputs[j][1] = this->VInputs[j][1] / InputVectorLength;
		this->ActivationPotential += this->Inputs[j] * this->VersicolorOrVirginicaWeights[j];
	}
	if (this->ActivationPotential > this->VersicolorOrVirginicaWeights[0])
		return true;
	else return false;
}