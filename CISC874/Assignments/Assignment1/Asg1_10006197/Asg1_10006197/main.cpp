#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

static const int MAX_TRAINING_EPOCHS = 100;			// Need to allow for non-linearly seperable data

//static const char * dir = "C:\\Users\\church\\Documents\\Fall2016\\CISC874\\Assignments\\Assignment1";
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
	Perceptron();
	// Used to decide if input is Iris-setosa or not
	double SInputs[4];
	double SWeights[4];
	double SThreshold = 0;
	double SClassificationActivation = 0;

	// If not Iris-setosa, these are used to decide if each input dimension indicates Iris-veriscolor or Iris-virginica
	double VDataInputs[4];
	double VDataWeights[4];
	double VDataThresholds[4];
	double VDataClassificationActivation[4];

	// Given the correspondences of the input dimensions to classes, these make the final classification
	double VActivationInputs[4];
	double VActivationWeights[4];
	double VThreshold = 0;
	double VClassificationActivation = 0;

	double LearningRate = 0.5;							// Try 1, doesn't have to be 1

	ofstream Output;
	void WriteData(string OutputData);
	void WriteData(vector<string> OutputData);

	void InitializeWeights();
	void NormalizeWeights();							
	void TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForSetosaOrNot(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);
	void TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
	void TestForVersicolorOrVirginica(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames);

	bool ClassifySetosaOrNot(array<double, 4> Inputs, string CorrectName);
	bool ClassifyVersicolorOrVirginica(array<double, 4> Inputs, string CorrectName);

	void SimpleFeedbackForSetosa(string CorrectName);
	void ErrorCorrectionForVersicolorOrVirginica(string CorrectName);
};

Perceptron::Perceptron()
{
	for (int i = 0; i < 4; i++)
	{
		this->VDataThresholds[i] = 0;
	}
}

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

	std::cout << "Press enter to end program." << endl;
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
	double ColMax[4];
	
	string line;
	std::string::size_type DatumEnd;
	string RawDatum;
	string RemainingData;
	array<double, 4> Biometrics;
	//char * SpeciesName;

	ifstream TrainingData("train.txt");
	if (TrainingData.is_open())
	{
		for (int i = 0; i < 4; i++)
			ColMax[i] = 0;

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
				RawDatum = (RemainingData.substr(0, DatumEnd));			// String of species name

				// Debug output
				/*
				cout << "Training data:" << endl;
				for (int i = 0; i < 4; i++)
					cout << Biometrics[i] << "	";
				cout << RawDatum.c_str() << endl;
				*/

				TrainingBiometrics.push_back(Biometrics);
				TrainingSpeciesNames.push_back(RawDatum);

				for (int i = 0; i < 4; i++)
				{
					if (ColMax[i] < Biometrics[i])
						ColMax[i] = Biometrics[i];
				}
			}
		}
		TrainingData.close();

		// Normalize input data over range [0, 1]
		for (int i = 0; i < TrainingBiometrics.size(); i++)
			for (int j = 0; j < 4; j++)
				TrainingBiometrics[i][j] = TrainingBiometrics[i][j] / ColMax[j];
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
		for (int i = 0; i < 4; i++)
			ColMax[i] = 0;

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
				RawDatum = (RemainingData.substr(0, DatumEnd));	// String of species name

				// Debug output
				/*
				cout << "Test data:" << endl;
				for (int i = 0; i < 4; i++)
				cout << Biometrics[i] << "	";
				cout << RawDatum.c_str() << endl;
				*/

				TestBiometrics.push_back(Biometrics);
				TestSpeciesNames.push_back(RawDatum);

				for (int i = 0; i < 4; i++)
				{
					if (ColMax[i] < Biometrics[i])
						ColMax[i] = Biometrics[i];
				}
			}
		}
		TestData.close();

		// Normalize input data over range [0, 1]
		for (int i = 0; i < TestBiometrics.size(); i++)
			for (int j = 0; j < 4; j++)
				TestBiometrics[i][j] = TestBiometrics[i][j] / ColMax[j];
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
	srand(time(NULL));
	int RandomInt;										
	double RandomDouble;

	// Initialize weights for Iris-setosa classification
	for (int i = 0; i < 4; i++)									
	{
		RandomInt = (rand() % 200) - 100;							// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		this->SWeights[i] = RandomDouble;
	}

	// Initialize weights for Iris-verisicolor/Iris-virginica classification
	for (int i = 0; i < 4; i++)
	{
		RandomInt = (rand() % 200) - 100;							// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		this->VDataWeights[i] = RandomDouble;
	}

	for (int i = 0; i < 4; i++)
	{
		RandomInt = (rand() % 200) - 100;							// Random number in range [-100, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [-1,1]
		this->VActivationWeights[i] = RandomDouble;
	}
}

void Perceptron::NormalizeWeights()
{
	double SWeightVectorLength = 0;
	double VDataWeightVectorLength = 0;
	double VActivationWeightVectorLength = 0;

	for (int i = 0; i < 4; i++)
	{
		SWeightVectorLength += this->SWeights[i] * this->SWeights[i];
		VDataWeightVectorLength += this->VDataWeights[i] * this->VDataWeights[i];
		VActivationWeightVectorLength += this->VActivationWeights[i] * this->VActivationWeights[i];
	}

	SWeightVectorLength = sqrt(SWeightVectorLength);
	VDataWeightVectorLength = sqrt(VDataWeightVectorLength);
	VActivationWeightVectorLength = sqrt(VActivationWeightVectorLength);

	for (int i = 0; i < 4; i++)
	{
		this->SWeights[i] = this->SWeights[i] / SWeightVectorLength;
		this->VDataWeights[i] = this->VDataWeights[i] / VDataWeightVectorLength;
		this->VActivationWeights[i] = this->VActivationWeights[i] / VActivationWeightVectorLength;
	}
}

void Perceptron::TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
	// Assume Iris-setosa is +ve class, otherwise is -ve class
	int CurrentEpoch = 0;
	string EpochsRequired = "Training epochs required to correctly classify all setosas versus non-setosas: ";	// For output.txt
	string FinalNormalizedWeights = "Normalized weight vector which classifies setosas versus non-setosas: ";
	while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)			
	{
		IncorrectlyClassified = TrainingBiometrics.size();							// New epoch, reinitialize IncorrectlyClassified count
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)		// For all the training patterns
		{
			if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))					// Input classified as Iris-setosa
			{
				if (TrainingSpeciesNames[i] == "Iris-setosa")
					IncorrectlyClassified--;
				else
					this->SimpleFeedbackForSetosa(TrainingSpeciesNames[i]);
				continue;
			}																								// Input classified as NON-Iris-setosa
			else
			{
				if (TrainingSpeciesNames[i] != "Iris-setosa")
					IncorrectlyClassified--;
				else
					this->SimpleFeedbackForSetosa(TrainingSpeciesNames[i]);
			}
		}
		//std::cout << "Incorrectly classified inputs after epoch " << CurrentEpoch << ": " << IncorrectlyClassified << endl;
		CurrentEpoch++;																// Guarantee training termination
	}

	EpochsRequired = EpochsRequired + to_string(CurrentEpoch);
	this->WriteData(EpochsRequired);
	
	for (int i = 0; i < 4; i++)
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
			//std::cout << "Input pattern " << i << " classified as: Iris-setosa." << endl;
			//std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
			continue;
		}
		else																// Input classified as NOT Iris-setosa
		{
			if (TestSpeciesNames[i] != "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as NON Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
			//std::cout << "Input pattern " << i << " classified as: NOT Iris-setosa." << endl;
			//std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
		}
	}
	this->WriteData("Therefore the perceptron correctly distinguished " + to_string(CorrectlyClassified) + " samples out of " + to_string(TestSpeciesNames.size()) + " as being Iris-setosa or not.");
}

void Perceptron::TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	int IncorrectlyClassified = TrainingBiometrics.size();
	int CurrentEpoch = 0;
	string EpochsRequired = "Training epochs required to correctly classify all versicolor versus virginica: ";	// For output.txt

  while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)
  {
    for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)											// For all input patterns
    {
      IncorrectlyClassified = TrainingBiometrics.size();													// Must reclassify all input patters
      if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))
      {
        if (TrainingSpeciesNames[i] == "Iris-setosa")
          IncorrectlyClassified--;
		continue;
        // Should already be trained to classify setosa, no more learning here
      }
      else
      {
        if (TrainingSpeciesNames[i] != "Iris-setosa")												  // Make sure not to learn off incorrectly classified iris-setosa
        {
          if (this->ClassifyVersicolorOrVirginica(TrainingBiometrics[i], TrainingSpeciesNames[i]))	 // Input classified as Iris-versicolor
          {
            if (TrainingSpeciesNames[i] == "Iris-versicolor")										 // If input is correctly classified as Iris-versicolor
            {
              IncorrectlyClassified--;
            }
            else																					 // If input is incorrectly classified as Iris-versicolor
            {
				this->ErrorCorrectionForVersicolorOrVirginica(TrainingSpeciesNames[i]);
            }
			continue;
          }
          else																							// Input classified as Iris-virginica
          {
            if (TrainingSpeciesNames[i] == "Iris-virginica")										  // Non-setosa input correctly classified as Iris-virginica
            {
              IncorrectlyClassified--;      
            }
            else                                                                  // Non-setosa input incorrecly classified as Iris-virginica
            {
				this->ErrorCorrectionForVersicolorOrVirginica(TrainingSpeciesNames[i]);
            }
			
          }
        }
      }
      this->NormalizeWeights();
    }
	cout << "VDataWeights after " << CurrentEpoch << "th epoch:	";
	for (int j = 0; j < 4; j++)
		cout << this->VDataWeights[j] << "	";
	cout << endl;
    CurrentEpoch++;
  }
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
	  continue;
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
          this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-versicolor. Correct classification is: " + TestSpeciesNames[i] + " Activation: " + to_string(this->VClassificationActivation));
		  continue;
		  //std::cout << "Input pattern " << i << " classified as: Iris-versicolor." << endl;
         // std::cout << "	Correct classification: " << TestSpeciesNames[i] << endl << endl;
        }
        else
        {
          if (TestSpeciesNames[i] == "Iris-virginica")
            CorrectlyClassified++;
          this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-virginica. Correct classification is: " + TestSpeciesNames[i] + " Activation: " + to_string(this->VClassificationActivation));
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
	this->SClassificationActivation = 0;				// Reinitialize for new classification							
	for (unsigned int j = 0; j < 4; j++)				// Instantiate inputs
		this->SInputs[j] = Inputs[j];

	for (unsigned int j = 0; j < 4; j++)				// Compute activation potential
		this->SClassificationActivation += this->SInputs[j] * this->SWeights[j];
	
	if (this->SClassificationActivation > this->SThreshold)	// Determine output value
	{
		return true;
	}
	else
    return false;
}

bool Perceptron::ClassifyVersicolorOrVirginica(array<double, 4> Inputs, string CorrectName)			// Returns true for versicolor classification
{
	this->VClassificationActivation = 0;

	for (int i = 0; i < 4; i++)													// Reinitialize for new classification	
		this->VDataClassificationActivation[i] = 0;
	
	for (int i = 0; i < 4; i++)													// Instantiate (4) nodes' inputs
		this->VDataInputs[i] = Inputs[i];

	for (int i = 0; i < 4; i++)
	{
		this->VDataClassificationActivation[i] += this->VDataInputs[i] * this->VDataWeights[i];
		//if (this->VDataClassificationActivation[i] > this->VDataThresholds[i])		// Dimension i's input indicates Iris-versicolor
		{
			this->VActivationInputs[i] = this->VDataClassificationActivation[i];
			this->VClassificationActivation += (this->VActivationInputs[i]) * (this->VActivationWeights[i]);
		}
	}
	if (this->VClassificationActivation > this->VThreshold)
		return true;
	else return false;
}

void Perceptron::SimpleFeedbackForSetosa(string CorrectName)                          
{
	if (this->SClassificationActivation > this->SThreshold)								// If classifier believes the input to be Iris-setosa
	{
		if (CorrectName != "Iris-setosa")
			for (int i = 0; i < 4; i++)
				this->SWeights[i] -= this->SInputs[i] * this->LearningRate;
	}
	else if (this->SClassificationActivation < (-1)*this->SThreshold)
	{
		if (CorrectName == "Iris-setosa")
			for (int i = 0; i < 4; i++)
				this->SWeights[i] += this->SInputs[i] * this->LearningRate;
	}
	else
	{
		if (CorrectName == "Iris-setosa")
		{
			for (int i = 0; i < 4; i++)
				this->SWeights[i] += this->SInputs[i] * this->LearningRate;
		}
		else
			for (int i = 0; i < 4; i++)
				this->SWeights[i] -= this->SInputs[i] * this->LearningRate;
	}
  this->NormalizeWeights();															
}

void Perceptron::ErrorCorrectionForVersicolorOrVirginica(string CorrectName)			   // Called regardless of (in)correctness of classification
{
	if (this->VClassificationActivation > this->VThreshold)									 
	{	// Input classified as Iris-versicolor
		if (CorrectName == "Iris-versicolor")												
		{	// Input correctly classified as Versicolor - activation is high enough	
			// Leave this->VActivationWeights alone for correct classification
			for (int i = 0; i < 4; i++)
			{	// Find dimensions classified in error
				if (this->VDataClassificationActivation[i] < this->VDataThresholds[i])		
				{
					this->VDataWeights[i] += (this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
		else	// Correct name == "Iris-virginica"										   
		{// Input incorrectly classified as Virginica - activation is too high
			for (int i = 0; i < 4; i++)
			{
				this->VActivationWeights[i] += ((-1)*this->VThreshold - this->VClassificationActivation) * this->VDataClassificationActivation[i] * this->LearningRate;
				if (this->VDataClassificationActivation[i] > (-1)*this->VDataThresholds[i])		
				{	
					this->VDataWeights[i] += ((-1)*this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
	}
	else if (this->VClassificationActivation < (-1)*this->VThreshold)						
	{	// Input classified as Iris-virginica
		if (CorrectName == "Iris-versicolor")												
		{	// Input incorrectly classified as Versicolor - activation too low
			for (int i = 0; i < 4; i++)
			{
				this->VActivationWeights[i] += (this->VThreshold - this->VClassificationActivation) * this->VDataClassificationActivation[i] * this->LearningRate;
				if (this->VDataClassificationActivation[i] < this->VDataThresholds[i])
				{
					this->VDataWeights[i] += (this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
		else																				
		{	// Input correctly classified as Virginica - activation is low enough
			// Leave this->VActivationWeights alone for correct classification
			for (int i = 0; i < 4; i++)
			{
				if (this->VDataClassificationActivation[i] > (-1)*this->VDataThresholds[i])		
				{	// Find dimensions classified in error
					this->VDataWeights[i] += ((-1)*this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
	}
	
	/*
	else																					// Unable to classify input
	{
		if (CorrectName == "Iris-versicolor")												// Activation was too low to recognize Iris-versicolor
		{
			for (int i = 0; i < 4; i++)
			{
				if (this->VDataClassificationActivation[i] < this->VDataThresholds[i])
				{
					this->VDataWeights[i] += (this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
		else //if (CorrectName == "Iris-virginica")
		{
			for (int i = 0; i < 4; i++)
			{
				if (this->VDataClassificationActivation[i] > this->VDataThresholds[i])
				{
					this->VDataWeights[i] += (this->VDataThresholds[i] - this->VDataClassificationActivation[i]) * this->VDataInputs[i] * this->LearningRate;
				}
			}
		}
	}
	*/
	
  this->NormalizeWeights();
}