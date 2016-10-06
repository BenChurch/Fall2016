/*			CISC 874 Assignment #1, by Ben Church - 10006197

	This program first learns to seperate Iris-setosa input data from NON-Iris-setosa input data using simpe feedback learning on the provided training data.

	It then tests the weights it learned for that seperation on the test data, writing to 'output.txt' the input value, the classification, and the correct class.

	The program then learns to classify Iris-versicolor data from Iris-virginica data, using error correction learning to adjust weights for four nodes which
	each deal with one of the input data's dimensions. These four nodes send a bipolar (-1 or 1) signal to a final output node, which makes the final classification
	depending on the "majority opinion" of the previous layer of nodes. The weights used to distinguish setosa from NON-setosa are retained for this process, so that
	the versicolor-virginica weights are only trained on NON-setosa input data.

	Finally, the program tests the final versicolor-virginica classification weights on the test data, writing to 'output.txt' the value of the input, followed by the
	perceptron network's classification, with the correct classification for reference.

	An instantiation of the perceptron class cannot be considered to be a perceptron per se; it contains values which allow a perceptron to be implemented and trained 
	to classify Iris-setosa from NON-Iris-setosa data, and values to implement one perceptron per input data dimension plus a final perceptron to take all their activations
	into consideration for the purpose of distinguishing Iris-versicolor from Iris-virginica. This allows the setosa weights to be retained so they can be used to prevent
	the versicolor-virginica nodes from being trained on setosa input data.

*/

#include <array>
#include <fstream>
#include <iostream>
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

static const int MAX_TRAINING_EPOCHS = 100;			// Need to allow for non-linearly seperable data

static const char * dir = ".";

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
	void ShuffleTrainingData();
};

InputData ReadData();	// Both vectors will be populated by function and should be passed empty

class Perceptron
{
public:
	Perceptron();
	// Used to decide if input is Iris-setosa or not
	double SInputs[4];
	double SWeights[4];
	double SThreshold = 0;
	double SClassificationActivation;

	// If not Iris-setosa, these are used to decide if each input dimension indicates Iris-veriscolor or Iris-virginica
	double VDataInputs[4];
	double VDataWeights[4];
	double VDataThresholds[4];
	double VDataClassificationActivation[4];

	// Given the correspondences of the input dimensions to classes, these make the final classification between Versicolor and Virginica
	double VActivationInputs[4];
	double VActivationWeights[4];
	double VThreshold = 0;
	double VClassificationActivation;

	double LearningRate = 0.5;
		
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
	void BatchErrorCorrectionForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames);
};

Perceptron::Perceptron()
{
	SetCurrentDirectoryA(dir);
	for (int i = 0; i < 4; i++)
	{
		this->VDataThresholds[i] = 1;			
	}
}

int main()
{
	InputData ID;
	ID = (ReadData());

	vector<array<double, 4>> TrainingBiometrics = ID.TrainingBiometrics;				// Petal width, Petal length, Sepal width, Sepal length
	vector<string> TrainingSpeciesNames = ID.TrainingSpeciesNames;					// Either Setosa, Versicolour, or Virgina
	vector<array<double, 4>> TestBiometrics = ID.TestBiometrics;
	vector<string> TestSpeciesNames = ID.TestSpeciesNames;

	Perceptron IrisIdentifier;
	IrisIdentifier.InitializeWeights();

	IrisIdentifier.TrainForSetosaOrNot(TrainingBiometrics, TrainingSpeciesNames);
	IrisIdentifier.TestForSetosaOrNot(TestBiometrics, TestSpeciesNames);

	IrisIdentifier.TrainForVersicolorOrVirginica(TrainingBiometrics, TrainingSpeciesNames);
	IrisIdentifier.TestForVersicolorOrVirginica(TestBiometrics, TestSpeciesNames);

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
	if (!this->Output.is_open())
		Output.open("output.txt");

	Output << OutputData << endl;
}

void Perceptron::InitializeWeights()
{
	srand(time(NULL));
	int RandomInt;										
	double RandomDouble;

	// Initialize weights for Iris-setosa classification
	for (int i = 0; i < 4; i++)									
	{
		RandomInt = (rand() % 100);							// Random number in range [0, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [0,1]
		this->SWeights[i] = RandomDouble;
	}

	// Initialize weights for Iris-verisicolor/Iris-virginica classification
	for (int i = 0; i < 4; i++)
	{
		RandomInt = (rand() % 100);							// Random number in range [0, 100], just to have a few decimal places
		RandomDouble = RandomInt / 100.0;										// Random double-precision decimal number in range [0,1]
		this->VDataWeights[i] = RandomDouble;
	}

	for (int i = 0; i < 4; i++)
		this->VActivationWeights[i] = 1;		

}

void Perceptron::TrainForSetosaOrNot(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{	
	string EpochsRequired = "Training epochs required to correctly classify all setosas versus non-setosas: ";	// For output.txt
	string InitialWeights = "Weight values used to classify setosa versus non-setosa before learning: ";
	for (int j = 0; j < 4; j++)
		InitialWeights = InitialWeights + "  " + to_string(this->SWeights[j]);
	string FinalNormalizedWeights = "Weight vector which classifies setosas versus non-setosas: ";

	int CurrentEpoch = 0;
	int IncorrectlyClassified = TrainingBiometrics.size();							// Assume all samples are misclassified and count down
	while (CurrentEpoch < MAX_TRAINING_EPOCHS && IncorrectlyClassified > 0)			
	{
		IncorrectlyClassified = TrainingBiometrics.size();			// New epoch, reinitialize IncorrectlyClassified count
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)		
		{	// For all the training patterns
			if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))					
			{	// Input classified as Iris-setosa
				if (TrainingSpeciesNames[i] == "Iris-setosa")
					IncorrectlyClassified--;
				else
					this->SimpleFeedbackForSetosa(TrainingSpeciesNames[i]);
				continue;
			}																								
			else
			{	// Input classified as NON-Iris-setosa
				if (TrainingSpeciesNames[i] != "Iris-setosa")
					IncorrectlyClassified--;
				else
					this->SimpleFeedbackForSetosa(TrainingSpeciesNames[i]);
			}
		}
		CurrentEpoch++;		// Guarantee training termination
	}

	EpochsRequired = EpochsRequired + to_string(CurrentEpoch);
	this->WriteData(EpochsRequired);
	string SetosaTrainingClassificationRate = "After the last training epoch, " + to_string(TrainingBiometrics.size() - IncorrectlyClassified) + " out of " + to_string(TrainingBiometrics.size())
		+ " inputs were correctly classified as setosa vs. NON-setosa for a classification accuracy of "
		+ to_string((((double)TrainingBiometrics.size() - (double)IncorrectlyClassified) / (double)TrainingBiometrics.size()));
	for (int i = 0; i < 4; i++)
		FinalNormalizedWeights = FinalNormalizedWeights + "	" + to_string(this->SWeights[i]);
	this->WriteData(InitialWeights);
	this->WriteData(FinalNormalizedWeights);
	this->WriteData(SetosaTrainingClassificationRate);

	this->WriteData("");	// New line
	this->WriteData("The equation of the line for the setosa - NON-setosa classification node is:");
	this->WriteData("	" + to_string(this->SWeights[0]) + " X1 + " + to_string(this->SWeights[1]) + " X2 + " + to_string(this->SWeights[2]) + " X3 + " + to_string(this->SWeights[3]) + " X4 = " + to_string(this->SThreshold));
	this->WriteData("	Where X1 is sepal length, X2 is sepal width, X3 is petal length, and X4 is petal width, all normalized in terms of the largest value in their respective dimension.");
	this->WriteData("");	// New line
}

void Perceptron::TestForSetosaOrNot(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{
	string InputPattern;
	int CorrectlyClassified = 0;
	for (unsigned int i = 0; i < TestBiometrics.size(); i++)							
	{	// Run test on all input patterns
		InputPattern = "";		// Reinitialize to empty string
		for (int j = 1; j < 5; j++)											
			InputPattern = InputPattern + "	" + to_string(TestBiometrics[i][j - 1]);

		if (this->ClassifySetosaOrNot(TestBiometrics[i], TestSpeciesNames[i]))			
		{	// Input is classified as Iris-setosa
			if (TestSpeciesNames[i] == "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
			continue;
		}
		else		
		{	// Input classified as NOT Iris-setosa
			if (TestSpeciesNames[i] != "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as NON Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
		}
	}
	this->WriteData("Therefore the perceptron correctly distinguished " + to_string(CorrectlyClassified) + " samples out of " + to_string(TestSpeciesNames.size()) + " as being Iris-setosa or not "
		+ "for a classification rate of " + to_string((double)CorrectlyClassified / (double)TestBiometrics.size()));
  this->WriteData(" "); // Empty line
}

void Perceptron::TrainForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	string EpochsRequired = "Training epochs required to correctly classify all versicolor versus virginica: ";
	string InitialWeights = "Initial value of weight vector used to classify versicolor-virginica:";
	string LearnedWeights = "Final value of weight vector used to classift versicolor-virginica:";
	 for (int j = 0; j < 4; j++)
		InitialWeights = InitialWeights + " " + to_string(this->VDataWeights[j]);
	this->WriteData(InitialWeights);

	int CurrentEpoch = 0;
	int IncorrectlyClassified = TrainingBiometrics.size();
	while (CurrentEpoch < (MAX_TRAINING_EPOCHS ) && IncorrectlyClassified > 0)				// MAX_TRAINING_EPOCHS + 1 to run it once without learning to demonstrate accuracy for 'output.txt'
	{
		IncorrectlyClassified = TrainingBiometrics.size();			// Must reclassify all input patters
		for (unsigned int i = 0; i < TrainingBiometrics.size(); i++)											
		{	// For all input patterns
			

			if (this->ClassifySetosaOrNot(TrainingBiometrics[i], TrainingSpeciesNames[i]))
			{
				if (TrainingSpeciesNames[i] == "Iris-setosa")
					IncorrectlyClassified--;
				continue;
				// Should already be trained to classify setosa, no more learning here
			}
			else
			{
				if (TrainingSpeciesNames[i] != "Iris-setosa")	// Make sure not to learn off incorrectly classified iris-setosa
				{
					if (this->ClassifyVersicolorOrVirginica(TrainingBiometrics[i], TrainingSpeciesNames[i]))	 
					{	// Input classified as Iris-versicolor
						if (TrainingSpeciesNames[i] == "Iris-versicolor")	// If input is correctly classified as Iris-versicolor
							IncorrectlyClassified--;
						continue;
					}
					else																							
					{	// Input classified as Iris-virginica
						if (TrainingSpeciesNames[i] == "Iris-virginica")	 // Non-setosa input correctly classified as Iris-virginica
							IncorrectlyClassified--;      
					}
				}
			}
		}
		//if (CurrentEpoch <= MAX_TRAINING_EPOCHS)					// We skip this on the extra iteration to demonstrate accuracy after learning for 'output.txt'
			this->BatchErrorCorrectionForVersicolorOrVirginica(TrainingBiometrics, TrainingSpeciesNames);

		std::cout << "VDataWeights after " << CurrentEpoch << "th epoch:	";
		for (int j = 0; j < 4; j++)
			std::cout << this->VDataWeights[j] << "	";
		std::cout << endl;
		CurrentEpoch++;
	}
	for (int j = 0; j < 4; j++)
		LearnedWeights = LearnedWeights + " " + to_string(this->VDataWeights[j]);
	this->WriteData(LearnedWeights);

	string VersicolorVirginicaTrainingClassificationRate = "After the last training epoch, " + to_string(TrainingBiometrics.size() - IncorrectlyClassified) + " out of " + to_string(TrainingBiometrics.size())
		+ " inputs were correctly classified as setosa vs. versicolor vs. virginica for a classification accuracy of "
		+ to_string((((double)TrainingBiometrics.size() - (double)IncorrectlyClassified) / (double)TrainingBiometrics.size()));
	this->WriteData(VersicolorVirginicaTrainingClassificationRate);

	this->WriteData("");	// New line
	this->WriteData("The equations of the nodes used to classify versicolor and virginica data are:");
	this->WriteData("	" + to_string(this->VDataWeights[0]) + " X1 = " + to_string(this->VDataThresholds[0]));
	this->WriteData("	" + to_string(this->VDataWeights[1]) + " X2 = " + to_string(this->VDataThresholds[1]));
	this->WriteData("	" + to_string(this->VDataWeights[2]) + " X3 = " + to_string(this->VDataThresholds[2]));
	this->WriteData("	" + to_string(this->VDataWeights[3]) + " X4 = " + to_string(this->VDataThresholds[3]));
	this->WriteData("	Where X1 is sepal length, X2 is sepal width, X3 is petal length, and X4 is petal width, all normalized in terms of the largest value in their respective dimension.");
	this->WriteData("");	// New line

	this->WriteData("The equation of the node used to combine the outputs of the above four nodes is:");
	this->WriteData("	" + to_string(this->VActivationWeights[0]) + " X1 + " + to_string(this->VActivationWeights[1]) + " X2 + " + to_string(this->VActivationWeights[2]) + " X3 + " + to_string(this->VActivationWeights[3]) + " X4 = " + to_string(this->VThreshold));
	this->WriteData("	Where XN is the output of the Nth data classification node, either -1 or 1.");
	this->WriteData("");
} 

void Perceptron::TestForVersicolorOrVirginica(vector<array<double, 4>> TestBiometrics, vector<string> TestSpeciesNames)
{
	string InputPattern;
	int CorrectlyClassified = 0;
	for (unsigned int i = 0; i < TestBiometrics.size(); i++)				
	{	// Run test on all input patterns
		InputPattern = "";													// Reinitialize to empty string
		// (Re) Initialize inputs
		for (int j = 0; j < 4; j++)										
			InputPattern = InputPattern + "	" + to_string(TestBiometrics[i][j]);

		if (this->ClassifySetosaOrNot(TestBiometrics[i], TestSpeciesNames[i]))		
		{	// Input is classified as Iris-setosa
			if (TestSpeciesNames[i] == "Iris-setosa")
				CorrectlyClassified++;
			this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-setosa. Correct classification is: " + TestSpeciesNames[i]);
			continue;
		}
		else																
		{	// Input classified as NOT Iris-setosa
			if (TestSpeciesNames[i] != "Iris-setosa")
			{
				if (this->ClassifyVersicolorOrVirginica(TestBiometrics[i], TestSpeciesNames[i]))
				{
					if (TestSpeciesNames[i] == "Iris-versicolor")
						CorrectlyClassified++;
					this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-versicolor. Correct classification is: " + TestSpeciesNames[i]);
					continue;
				}
				else
				{
					if (TestSpeciesNames[i] == "Iris-virginica")
						CorrectlyClassified++;
					this->WriteData("Perceptron classified input pattern " + to_string(i) + ":" + InputPattern + " as Iris-virginica. Correct classification is: " + TestSpeciesNames[i]);
				}
			}
			else
			{
				this->WriteData("Pereptron classified input pattern " + to_string(i) + ":" + InputPattern + " incorrectly as NOT Iris-setosa");
			}
		}
	}
	this->WriteData("Therefore the perceptron correctly distinguished " + to_string(CorrectlyClassified) + " samples out of " + to_string(TestSpeciesNames.size()) + " as being setosa, versicolor, or Iris-virginica "
		+ "for a classification rate of " + to_string((double)CorrectlyClassified / (double)TestBiometrics.size()));
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

	for (int i = 0; i < 4; i++)									// Reinitialize for new classification	
		this->VDataClassificationActivation[i] = 0;
	
	for (int i = 0; i < 4; i++)									// Instantiate (4) nodes' inputs
		this->VDataInputs[i] = Inputs[i];

	for (int i = 0; i < 4; i++)
	{
		this->VDataClassificationActivation[i] = (this->VDataInputs[i] * this->VDataWeights[i]);
		if (this->VDataClassificationActivation[i] >= this->VDataThresholds[i])		// Dimension i's input indicates Iris-versicolor
		{
			this->VActivationInputs[i] = -1;	// Versicolor-Virginica data classification neurons have bipolar output
			this->VClassificationActivation += (this->VActivationInputs[i] * this->VActivationWeights[i]);
		}
		else if (this->VDataClassificationActivation[i] < this->VDataThresholds[i])
		{
			this->VActivationInputs[i] = 1;
			this->VClassificationActivation += (this->VActivationInputs[i] * this->VActivationWeights[i]);
		}
	}
	if (this->VClassificationActivation > this->VThreshold)
		return true;
	else 
		return false;
}

void Perceptron::SimpleFeedbackForSetosa(string CorrectName)                          
{
	if (this->SClassificationActivation > this->SThreshold)								
	{	// If classifier believes the input to be Iris-setosa
		if (CorrectName != "Iris-setosa")
			for (int i = 0; i < 4; i++)
				this->SWeights[i] -= this->SInputs[i] * this->LearningRate;
	}
	if (this->SClassificationActivation < this->SThreshold)
	{
		if (CorrectName == "Iris-setosa")
			for (int i = 0; i < 4; i++)
				this->SWeights[i] += this->SInputs[i] * this->LearningRate;
	}
}

void Perceptron::BatchErrorCorrectionForVersicolorOrVirginica(vector<array<double, 4>> TrainingBiometrics, vector<string> TrainingSpeciesNames)
{
	double VersicolorErrorSum[4];
	double VirginicaErrorSum[4];
	double VersicolorAverageError[4];
	double VirginicaAverageError[4];
	for (int j = 0; j < 4; j++)
	{
		VersicolorErrorSum[j] = 0;
		VirginicaErrorSum[j] = 0;
		VersicolorAverageError[j] = 0;
		VirginicaAverageError[j] = 0;
		for (int i = 40; i < TrainingBiometrics.size(); i++)
		{
			if (TrainingSpeciesNames[i] == "Iris-virginica" && ((TrainingBiometrics[i][j] * this->VDataWeights[j]) > this->VDataThresholds[j]))			// Somewhere Virginica was misclassified as Versicolor
				VirginicaErrorSum[j] -= (TrainingBiometrics[i][j] * this->VDataWeights[j]) - (this->VDataThresholds[j]);			
			if (TrainingSpeciesNames[i] == "Iris-versicolor" && (TrainingBiometrics[i][j] * this->VDataWeights[j] < this->VDataThresholds[j]))			// Somewhere Versicolor was misclassified as Virginica
				VersicolorErrorSum[j] += (TrainingBiometrics[i][j] * this->VDataWeights[j]) - (this->VDataThresholds[j]);
		}
		VirginicaAverageError[j] = VirginicaErrorSum[j] / 40;		// Not exactly robust, there happen to be 40 Virginica samples.
		VersicolorAverageError[j] = VersicolorErrorSum[j] / 40;

		if (VirginicaAverageError[j] > VersicolorAverageError[j])	// We gain more benefit from correcting for Virginica classification
		{
			this->VDataWeights[j] += (VirginicaAverageError[j] - VersicolorAverageError[j]) * this->LearningRate;				// Does not include input value
		}
		else
			this->VDataWeights[j] -= (VersicolorAverageError[j] - VirginicaAverageError[j]) * this->LearningRate;
	}
}