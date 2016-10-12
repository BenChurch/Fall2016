#include <array>
#include <fstream>
#include <iostream>
#include <math.h>		// For computation of nodes' sigmoidal-exponential activation function
#include <stdlib.h>		// For .txt file strings to double representations, and random number generation, and random number generation
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

<<<<<<< HEAD
static const int MAX_TRAINING_EPOCHS = 10000;

static const int NumHiddenNodes = 16;
=======
static const int MAX_TRAINING_EPOCHS = 500;
static const int NumHiddenNodes = 20;
>>>>>>> cab62334720641f8a8ec8048aa4990c60b54b609

static const char * Dir = ".";

using namespace std;

class ImageData		// Can store either 1 bit or 4 bit image data with the images' correct classification
{
public:
	vector<vector<vector<int>>> Images;				// Each <vector<vector<int>> is a vector of rows, each of which is a vector of pixel values; use a 3rd vector "layer" to store muliple images
	vector<int> CorrectClassifications;

	void PrintAllData();							// Used in development to verify ReadInputs() functionality
};

ImageData ReadInputs(const char * FileName);

void WriteDataForMatlab(ImageData TrainingData, ImageData TestData);

class Node
{
public:
	Node(double Threshold, int Inputs);
	double Threshold;
	vector<double> Inputs;				// Should be same length as Weights vector
	vector<double> Weights;

	double ActivationPotential = 0;

	void ComputeIdentityActivation();		// Used for input layer nodes
	void ComputeSigmoidActivation();

	
};

Node::Node(double Threshold, int Inputs)
{
	this->Threshold = Threshold;
	int RandomInt;
	double RandomDouble;
	for (int i = 0; i < Inputs; i++)
	{
		this->Inputs.push_back(0);
		RandomInt = (rand() % 200) - 100;			// Random int in range [-100 to 100] to leave us with some decimal places
		RandomDouble = ((double)RandomInt / 100);
		this->Weights.push_back(RandomDouble);
	}
}

void Node::ComputeIdentityActivation()
{
	this->ActivationPotential = 0;					// Reinitialize for new iteration
	for (int i = 0; i < this->Inputs.size(); i++)
		this->ActivationPotential += this->Inputs[i] * 1;		// Hacking around randomly initialized input weight values -- should fix this initialization
		//this->ActivationPotential += this->Inputs[i] * this->Weights[i];
}

void Node::ComputeSigmoidActivation()
{
	this->ActivationPotential = 0;
	double A = 0;						// Input to activation function, SUM(Input * Weight)
	for (int i = 0; i < this->Inputs.size(); i++)			// Will any normalization be necessary?
	{
		A += this->Inputs[i] * this->Weights[i];
	}
	this->ActivationPotential = 1 / (1 + exp((-1)*A));			// Slope of function curve can be adjusted by multiplying Xi by a constant
}

class Network
{
public:
	int Classification;

	Network();
<<<<<<< HEAD
  int Classification;
  double MeanSquaredError = 0;    // Used to decide when to stop training
  static const int MAX_TRAINGING_EPOCHS = MAX_TRAINING_EPOCHS;
  double ErrorOffset = 0.3;   // Used to aim past the threshold, so we don't sit right on it
	double Momentum = 0.4;			// Used to keep weights from settling on local minima
=======
	static const int MAX_TRAINGING_EPOCHS = MAX_TRAINING_EPOCHS;
	double Momentum = 0.1;			// Used to keep weights from settling on local minima
>>>>>>> cab62334720641f8a8ec8048aa4990c60b54b609
	double LeanringRate = 1;
	double ErrorOffset = 0.5;		//

	vector<vector<Node>> InputLayer;// Activation function is identity, serves to pass weighted inputs to hidden node - representing in matrix form analogous to image pixels		
	vector<Node> HiddenLayer;		// Activation function is sigmoidal
	vector<Node> OutputLayer;		// Activation function is sigmoidal

	void ConstructNodes(vector<vector<int>> TemplateImage);
	void Feedworward(vector<vector<int>> Image);
	void Backpropagate(int CorrectClassification);
	void Train(ImageData TrainingData);
	void Test(ImageData TestData);
  void Classify();
  void EasyClassify();

	void Classify();				// maps state of output neurons to [0-9] integer classification

	void WriteSelf(string FileIdentifier);
};
Network::Network()
{
	srand(time(NULL));			// Provide random number seed for random weight initialization
}

void Network::ConstructNodes(vector<vector<int>> TemplateImage)	// Initialize network architecture
{
	// Initialize input layer
	vector<Node> AnInputRow;
	for (int InputRow = 0; InputRow < TemplateImage.size(); InputRow++)
	{	// Organize in array structure, like image data
		for (int InputNode = 0; InputNode < TemplateImage[InputRow].size(); InputNode++)
		{
			AnInputRow.push_back(Node(0, 1));
		}
		this->InputLayer.push_back(AnInputRow);
		AnInputRow.clear();
	}

	// Construct hidden layer to recognize features
	for (int HiddenNode = 0; HiddenNode < NumHiddenNodes; HiddenNode++)
		this->HiddenLayer.push_back(Node(0, (this->InputLayer.size() * this->InputLayer[0].size())));

	// Construct output layer to classify 10 different digits
	for (int OutputNode = 0; OutputNode < 10; OutputNode++)		// Need to recognize 10 different digits
		this->OutputLayer.push_back(Node(0.5, this->HiddenLayer.size()));
}

void Network::Feedworward(vector<vector<int>> Image)
{
	// Iterators over network just to tidy the code
	Node * CurrentInputNode; 
	Node * CurrentHiddenNode; 
	Node * CurrentOutputNode;

	for (int InputNodeRow = 0; InputNodeRow < this->InputLayer.size(); InputNodeRow++)
	{
		for (int InputNode = 0; InputNode < this->InputLayer[InputNodeRow].size(); InputNode++)
		{
			CurrentInputNode =  &(this->InputLayer[InputNodeRow][InputNode]);			
			(*CurrentInputNode).Inputs[0] = Image[InputNodeRow][InputNode];
			(*CurrentInputNode).ComputeIdentityActivation();		// ? Input nodes not to perform computation, just pass weighted value ?
			for (int HiddenNode = 0; HiddenNode < this->HiddenLayer.size(); HiddenNode++)
			{
				CurrentHiddenNode = &(this->HiddenLayer[HiddenNode]);
				(*CurrentHiddenNode).Inputs[((InputNodeRow + 1) * (InputNode + 1)) - 1] = (*CurrentInputNode).ActivationPotential;		// This 2D to 1D indexing redistributes values across HiddenNode Inputs
				// ComputeSigmoidActivation() iterates through each weight, populated or not, therfore only run this once all inputs updated
			}
		}
	}

	for (int HiddenNode = 0; HiddenNode < this->HiddenLayer.size(); HiddenNode++)
	{
		CurrentHiddenNode = &(this->HiddenLayer[HiddenNode]);
		(*CurrentHiddenNode).ComputeSigmoidActivation();
		for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
		{
			CurrentOutputNode = &(this->OutputLayer[OutputNode]);
			(*CurrentOutputNode).Inputs[HiddenNode] = (*CurrentHiddenNode).ActivationPotential;
		}
	}

	for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
	{
		CurrentOutputNode = &(this->OutputLayer[OutputNode]);
		(*CurrentOutputNode).ComputeSigmoidActivation();
	}
	// Assert that network is in equilibrium with input Image
}

void Network::Backpropagate(int CorrectClassification)
{
	Node * CurrentInputNode;
	Node * CurrentHiddenNode;
	Node * CurrentOutputNode;
	double CurrentActivation;						// Saves me from writing (*CurrentNode).Activation (== y) everywhere

	vector<double> ErrorVector;
	vector<double> deltaHiddenToOutput;

	double LastWeightChange = 0;					// Used with Momentum feature
	for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)		// Check all possible classifications
	{
		CurrentOutputNode = &(this->OutputLayer[OutputNode]);
		CurrentActivation = (*CurrentOutputNode).ActivationPotential;
		
		if (OutputNode != CorrectClassification && (CurrentActivation >(*CurrentOutputNode).Threshold - this->ErrorOffset))
		{	// If an output node is active that shouldn't be
<<<<<<< HEAD
      this->MeanSquaredError += ((CurrentActivation - (*CurrentOutputNode).Threshold)*(CurrentActivation - (*CurrentOutputNode).Threshold));
			ErrorVector.push_back(((*CurrentOutputNode).Threshold - CurrentActivation - this->ErrorOffset));		// Error is negative
=======
			ErrorVector.push_back(((*CurrentOutputNode).Threshold - CurrentActivation) - this->ErrorOffset);		// Error is negative
>>>>>>> cab62334720641f8a8ec8048aa4990c60b54b609
			deltaHiddenToOutput.push_back(ErrorVector[OutputNode] * CurrentActivation * (1 - CurrentActivation));
			for (int Input = 0; Input < (*CurrentOutputNode).Weights.size(); Input++)
			{
        (*CurrentOutputNode).Weights[Input] += ((this->LeanringRate) * ((*CurrentOutputNode).Inputs[Input]) * deltaHiddenToOutput[OutputNode])
					+ this->Momentum*(LastWeightChange);
        LastWeightChange = ((this->LeanringRate) * ((*CurrentOutputNode).Inputs[Input]) * deltaHiddenToOutput[OutputNode])
					+ this->Momentum*(LastWeightChange);
			}
			
			continue;
		}
		else if (OutputNode == CorrectClassification && CurrentActivation < (*CurrentOutputNode).Threshold + this->ErrorOffset)
		{	// If an output node isn't active but should be
<<<<<<< HEAD
      this->MeanSquaredError += ((CurrentActivation - (*CurrentOutputNode).Threshold)*(CurrentActivation - (*CurrentOutputNode).Threshold));
=======
>>>>>>> cab62334720641f8a8ec8048aa4990c60b54b609
			ErrorVector.push_back((*CurrentOutputNode).Threshold - CurrentActivation + this->ErrorOffset);			// Error is positive
			deltaHiddenToOutput.push_back(ErrorVector[OutputNode] * CurrentActivation * (1 - CurrentActivation));
			for (int Input = 0; Input < (*CurrentOutputNode).Weights.size(); Input++)
			{
        (*CurrentOutputNode).Weights[Input] += ((this->LeanringRate) * ((*CurrentOutputNode).Inputs[Input]) * deltaHiddenToOutput[OutputNode])
					+ this->Momentum*(LastWeightChange);
        LastWeightChange = ((this->LeanringRate) * ((*CurrentOutputNode).Inputs[Input]) * deltaHiddenToOutput[OutputNode])
					+ this->Momentum*(LastWeightChange);
			}
		}
		else		//output nodes are in the correct state
		{
			ErrorVector.push_back(0);	// Correct classification interpreted as no error
			deltaHiddenToOutput.push_back(0);
		}
	}
	// ASSERT that all output nodes' weights have been adjusted
	//vector<double> deltaInputToHidden;
	double NewDelta;
	for (int HiddenNode = 0; HiddenNode < this->HiddenLayer.size(); HiddenNode++)
	{
		CurrentHiddenNode = &(this->HiddenLayer[HiddenNode]);
    NewDelta = 0;
    for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
    {
      CurrentOutputNode = &(this->OutputLayer[OutputNode]);
      // Try different braket locations --- not sure exatly what to sum
      NewDelta += (deltaHiddenToOutput[OutputNode] * (*CurrentOutputNode).Weights[HiddenNode]) * ((*CurrentHiddenNode).ActivationPotential) * (1 - (*CurrentHiddenNode).ActivationPotential);
    }
		for (int HiddenNodeWeight = 0; HiddenNodeWeight < this->HiddenLayer[HiddenNode].Weights.size(); HiddenNodeWeight++)
		{
			NewDelta = 0;
			CurrentInputNode = &(this->InputLayer[HiddenNodeWeight / this->InputLayer.size()][HiddenNodeWeight % this->InputLayer[HiddenNodeWeight / this->InputLayer.size()].size()]);			// I think this un-does the 2D to 1D conversion
			(*CurrentHiddenNode).Weights[HiddenNodeWeight] += this->LeanringRate * NewDelta * (*CurrentInputNode).ActivationPotential;		// This may need to be (*CurrentInputNode).Input
		}
	}
}

void Network::Train(ImageData TrainingData)
{
	int CurrentEpoch = 0;
  double MinMeanSquaredError = 100;     // Some large number to begin comparrison
	while (CurrentEpoch < this->MAX_TRAINGING_EPOCHS)			// Not bothering with IncorrectlyClassified count
	{
    this->MeanSquaredError = 0;   // Reinitialize for new epoch
		for (int i = 0; i < TrainingData.Images.size(); i++)
		{
			this->Feedworward(TrainingData.Images[i]);
			this->Backpropagate(TrainingData.CorrectClassifications[i]);
		}
    this->MeanSquaredError = this->MeanSquaredError / TrainingData.Images.size();
    cout << "Mean-squared-error for epoch " << CurrentEpoch << ": " << this->MeanSquaredError << endl;
		CurrentEpoch++;
    if (this->MeanSquaredError < MinMeanSquaredError)
    {
      MinMeanSquaredError = this->MeanSquaredError;
    }
    if (this->MeanSquaredError > MinMeanSquaredError*1.5)
    { // Last epoch made ouput worse
      cout << "Mean-squared-error worsening occurred after epoch" << CurrentEpoch << ". Ceasing training." << endl;
      break;
    }
		cout << "Training epoch " << CurrentEpoch << " out of " << this->MAX_TRAINGING_EPOCHS << " complete" << endl;
	}
}

void Network::Test(ImageData TestData)
{
	int CorrectlyClassified = 0;

	Node * CurrentOutputNode;
	Node * OtherOutputNode;
	for (int i = 0; i < TestData.Images.size(); i++)
	{
    cout << "Test image #" << i << "  ";
		this->Feedworward(TestData.Images[i]);
<<<<<<< HEAD
    //this->Classify();
    this->EasyClassify();
    if (this->Classification < 0)
    { // Unable to perform classification
      cout << endl;
    }
    else if (this->Classification == TestData.CorrectClassifications[i])
    {
      CorrectlyClassified++;
      cout << "corretly classified as " << TestData.CorrectClassifications[i] << endl;
    }
    else
      cout <<" incorrectly " << TestData.CorrectClassifications[i] << " classified as " << this->Classification << endl;
=======
		this->Classify();
		if (this->Classification == TestData.CorrectClassifications[i])
			CorrectlyClassified++;

	}
	std::cout << "Correctly classified " << CorrectlyClassified << " images out of " << TestData.Images.size() << " for a classification accuracy of " << ((double)CorrectlyClassified)/(TestData.Images.size()) << endl;
}

void Network::Classify()
{

	Node * CurrentOutputNode;
	Node * CurrentOtherNode;
	for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
	{	// For all 10 output nodes
		CurrentOutputNode = (&this->OutputLayer[OutputNode]);
		if ((*CurrentOutputNode).ActivationPotential > (*CurrentOutputNode).Threshold)
		{
			for (int OtherNodes = 0; OtherNodes < this->OutputLayer.size(); OtherNodes++)
			{
				CurrentOtherNode = (&this->OutputLayer[OtherNodes]);
				if (((*CurrentOtherNode).ActivationPotential > (*CurrentOtherNode).Threshold) && OtherNodes != OutputNode)
				{	// We have more than one active output neuron
					std::cout << "Multiple output neurons active - classification uncertain - this.Classification not updated" << endl;
					return;
				}
			}
			this->Classification = OutputNode;
			return;
		}
>>>>>>> cab62334720641f8a8ec8048aa4990c60b54b609
	}

	std::cout << "No output neurons active - classification uncertain - this.Classification not updated" << endl;
}

void Network::Classify()
{
  Node * CurrentOutputNode;
  Node * OtherOutputNode;
  for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
  {
    CurrentOutputNode = (&this->OutputLayer[OutputNode]);
    if ((*CurrentOutputNode).ActivationPotential > (*CurrentOutputNode).Threshold)
    {   // One of the output nodes is active
      for (int OtherNodes = 0; OtherNodes < this->OutputLayer.size(); OtherNodes++)
      {
        OtherOutputNode = &(this->OutputLayer[OtherNodes]);
        if ((*OtherOutputNode).ActivationPotential > (*OtherOutputNode).Threshold && OutputNode != OtherNodes)
        { // If more than one output node is active
          cout << "More than one output node active - could not perform classification" << endl;
          return;
        }
      }
      // Only the one ouput node is active
      this->Classification = OutputNode;
      return;
    }
  }
  // No output nodes were active
  cout << "No output nodes were active - could not perform classification" << endl;
}

void Network::EasyClassify()
{
  double MaxActivation = 0;
  int Classification;
  for (int i = 0; i < this->OutputLayer.size(); i++)
  {
    if (MaxActivation < this->OutputLayer[i].ActivationPotential)
    {
      MaxActivation = this->OutputLayer[i].ActivationPotential;
      Classification = i;
    }
  }
  this->Classification = Classification;
  return;
}

void Network::WriteSelf(string FileIdentifier)						// Just used for my debugging purposes. Output doesn't necessarily line up - can be confusing
{
	ofstream SelfOutput;
	SelfOutput.open("NetworkState" + FileIdentifier + ".txt");

	// Write input array
	SelfOutput << endl << "Input array: " << endl;
	for (int InputRow = 0; InputRow < this->InputLayer.size(); InputRow++)
	{
		SelfOutput << endl;
		//std::cout << "	Input array, row " << InputRow << ": " << endl;
		for (int InputNode = 0; InputNode < this->InputLayer[InputRow].size(); InputNode++)
		{	// Print nodes' index values
			SelfOutput << "		Index: " << InputRow << ", " << InputNode << "	";
		}
		SelfOutput << endl;
		for (int InputNode = 0; InputNode < this->InputLayer[InputRow].size(); InputNode++)
		{	// Print nodes' input values
			SelfOutput << "		Input: " << this->InputLayer[InputRow][InputNode].Inputs[0] << "	";
		}
		SelfOutput << endl;
		for (int InputNode = 0; InputNode < this->InputLayer[InputRow].size(); InputNode++)
		{	// Print nodes' weight values
			SelfOutput << "		Weight: " << this->InputLayer[InputRow][InputNode].Weights[0] << "	";
		}
		SelfOutput << endl;
		for (int InputNode = 0; InputNode < this->InputLayer[InputRow].size(); InputNode++)
		{	// Print nodes' activation potentials
			SelfOutput << "		Act.: " << this->InputLayer[InputRow][InputNode].ActivationPotential << "		";
		}
		SelfOutput << endl;
		for (int InputNode = 0; InputNode < this->InputLayer[InputRow].size(); InputNode++)
		{	// Print nodes' threshold values
			SelfOutput << "		Threshold: " << this->InputLayer[InputRow][InputNode].Threshold << "	";
		}
		SelfOutput << endl;
	}

	// Write hidden layer
	SelfOutput << "Hidden layer: " << endl;
	SelfOutput << endl;
	SelfOutput << "		";
	for (int Node = 0; Node < this->HiddenLayer.size(); Node++)
	{	// Print nodes' index values
		SelfOutput << "		Node: " << Node << "	";
	}
	SelfOutput << endl;

	SelfOutput << "	Inputs:" << endl;
	for (int Input = 0; Input < this->HiddenLayer[0].Inputs.size(); Input++)
	{	// Print nodes' input values
		SelfOutput << "		Input number" << Input;
		for (int Node = 0; Node < this->HiddenLayer.size(); Node++)
			SelfOutput << "		" << this->HiddenLayer[Node].Inputs[Input] << "	";
		SelfOutput << endl;
	}
	SelfOutput << endl;

	SelfOutput << "	Weights:" << endl;
	for (int Weight = 0; Weight < this->HiddenLayer[0].Weights.size(); Weight++)
	{	// Print nodes' input values
		SelfOutput << "		Weight number" << Weight;
		for (int Node = 0; Node < this->HiddenLayer.size(); Node++)
			SelfOutput << "		" << this->HiddenLayer[Node].Weights[Weight] << "	";
		SelfOutput << endl;
	}
	SelfOutput << endl;

	SelfOutput << "		Activation: ";
	for (int Node = 0; Node < this->HiddenLayer.size(); Node++)
	{	// Print nodes' activation potentials
		SelfOutput << "		" << this->HiddenLayer[Node].ActivationPotential << "	";
	}
	SelfOutput << endl;
	
	SelfOutput << endl;
	SelfOutput << "		Threshold: ";
	for (int Node = 0; Node < this->HiddenLayer.size(); Node++)
	{	// Print nodes' threshold values
		SelfOutput << "		" << this->HiddenLayer[Node].Threshold << "	";
	}
	SelfOutput << endl;

	// Write output layer
	SelfOutput << endl;
	SelfOutput << "Output layer: " << endl;
	SelfOutput << endl;
	SelfOutput << "		";
	for (int Node = 0; Node < this->OutputLayer.size(); Node++)
	{	// Print nodes' index values
		SelfOutput << "		Node: " << Node << "	";
	}
	SelfOutput << endl;

	SelfOutput << "	Inputs:" << endl;
	for (int Input = 0; Input < this->OutputLayer[0].Inputs.size(); Input++)
	{	// Print nodes' input values
		SelfOutput << "		Input number" << Input;
		for (int Node = 0; Node < this->OutputLayer.size(); Node++)
			SelfOutput << "		" << this->OutputLayer[Node].Inputs[Input] << "	";
		SelfOutput << endl;
	}
	SelfOutput << endl;

	SelfOutput << "	Weights:" << endl;
	for (int Weight = 0; Weight < this->OutputLayer[0].Weights.size(); Weight++)
	{	// Print nodes' input values
		SelfOutput << "		Weight number" << Weight;
		for (int Node = 0; Node < this->OutputLayer.size(); Node++)
			SelfOutput << "		" << this->OutputLayer[Node].Weights[Weight] << "	";
		SelfOutput << endl;
	}
	SelfOutput << endl;

	SelfOutput << "		Activation: ";
	for (int Node = 0; Node < this->OutputLayer.size(); Node++)
	{	// Print nodes' activation potentials
		SelfOutput << "		" << this->OutputLayer[Node].ActivationPotential << "	";
	}
	SelfOutput << endl;

	SelfOutput << endl;
	SelfOutput << "		Threshold: ";
	for (int Node = 0; Node < this->OutputLayer.size(); Node++)
	{	// Print nodes' threshold values
		SelfOutput << "		" << this->OutputLayer[Node].Threshold << "	";
	}
	SelfOutput << endl;
	SelfOutput.close();
}	  

int main()
{
	ImageData InputData;
	InputData = ReadInputs("training.txt");
	ImageData TestData;
	TestData = ReadInputs("testing.txt");
  WriteDataForMatlab(InputData, TestData);
  //InputData.PrintAllData();
	Network BackpropagationNetwork;
	BackpropagationNetwork.ConstructNodes(InputData.Images[0]);
	BackpropagationNetwork.WriteSelf("1");
  BackpropagationNetwork.Train(InputData);
	BackpropagationNetwork.WriteSelf("2");
	BackpropagationNetwork.Test(TestData);


	cout << "Press enter to end the program." << endl;
	cin.ignore();
	return 0;
}

void ImageData::PrintAllData()
{
	for (int i = 0; i < this->Images.size(); i++)			// For each image stored herein
	{
		cout << "Image number " << i << endl;
		for (int Row = 0; Row < this->Images[i].size(); Row++)
		{
			for (int Col = 0; Col < this->Images[i][Row].size(); Col++)
				cout << Images[i][Row][Col] << ", ";
			cout << endl;
		}
		cout << "		Correct classification: " << this->CorrectClassifications[i] << endl << endl;
	}
}

void WriteDataForMatlab(ImageData TrainingData, ImageData TestData)
{
  SetCurrentDirectoryA;
  ofstream Images, Classifications;
  Images.open("Images.txt");
  Classifications.open("Classsifications.txt");
  string line;
  for (int Image = 0; Image < TrainingData.Images.size(); Image++)
  {
    line = "";      // Reinitialize
    for (int Row = 0; Row < 8; Row++)
    {
      for (int Pixel = 0; Pixel < 8; Pixel++)
      {
        line = line + to_string(TrainingData.Images[Image][Row][Pixel]);
        if(Row != 7 || Pixel != 7)
          line = line + ',';
      }
    }
    Images << line << endl;
    Classifications << to_string(TrainingData.CorrectClassifications[Image]) << endl;
  }

  for (int Image = 0; Image < TestData.Images.size(); Image++)
  {
    line = "";      // Reinitialize
    for (int Row = 0; Row < 8; Row++)
    {
      for (int Pixel = 0; Pixel < 8; Pixel++)
      {
        line = line + to_string(TestData.Images[Image][Row][Pixel]);
        if (Row != 7 || Pixel != 7)
          line = line + ',';
      }
    }
    Images << line << endl;
    Classifications << to_string(TestData.CorrectClassifications[Image]) << endl;
  }
  Images.close();
  Classifications.close();
}

ImageData ReadInputs(const char * FileName)
{
	SetCurrentDirectoryA(Dir);
	ImageData InputData;
	ifstream Data(FileName);
	string line;
std:string::size_type DatumEnd;	// Repeatedly pushed past the next data value to be read into InputData
	std::string::size_type SubstringSize;
	int DatumValue;
	string Datum;

	vector<int> Cols;				// One pixel value for each col in a row
	vector<vector<int>> RowOfCols;	// Represents one image

	if (Data.is_open())
	{
		getline(Data, line);
		SubstringSize = line.size();
		if ((line[0]) == 48)		// Not exactly robust, but first char in both 4-bit image input files is 0, represented by an int = 48, whereas original-data files have headers
		{	// FileName contains rows of 64 4-bit pixel data corresponding to 8X8 pixel images, followed by one digit representing correct classification
			for (int Row = 1; Row <= 8; Row++)									// First line instance contains values we need
			{
				for (int Col = 1; Col <= 8; Col++)		
				{
					DatumEnd = line.find(",");
					Datum = line.substr(0, DatumEnd);
					line = line.substr(DatumEnd+1, SubstringSize);
					SubstringSize = line.size();
					DatumValue = atof(Datum.c_str());
					Cols.push_back(DatumValue);			
				}
				RowOfCols.push_back(Cols);
				Cols.clear();
			}
			InputData.Images.push_back(RowOfCols);
			RowOfCols.clear();
			Datum = line.substr(0, SubstringSize);	// The correct classification value remains - guaranteed to be 1 digit
			DatumValue = atof(Datum.c_str());
			InputData.CorrectClassifications.push_back(DatumValue);			
			while (getline(Data, line))				// Writes line line from FileName into line variable
			{
				SubstringSize = line.size();
				for (int Row = 1; Row <= 8; Row++)									// First line instance contains values we need
				{
					for (int Col = 1; Col <= 8; Col++)
					{
						DatumEnd = line.find(",");
						Datum = line.substr(0, DatumEnd);
						line = line.substr(DatumEnd + 1, SubstringSize);
						SubstringSize = line.size();
						DatumValue = atof(Datum.c_str());
						Cols.push_back(DatumValue);			
					}
					RowOfCols.push_back(Cols);
					Cols.clear();
				}
				InputData.Images.push_back(RowOfCols);
				RowOfCols.clear();
				Datum = line.substr(0, SubstringSize);	// The correct classification value remains - guaranteed to be 1 digit
				DatumValue = atof(Datum.c_str());
				InputData.CorrectClassifications.push_back(DatumValue);
			}
			Data.close();
			return InputData;
		}
		else
		{	// FileName constains Rows of binary pixel values corresponding to 8X8 1-bit images
			while (getline(Data, line) && (line[0] != 48 || line.size() < 10))		// Read past header
			{
				// Do nothing
			}
			// line that exits above while-loop is not a header line
			for (int Row = 0; Row < 32; Row++)
			{
				for (int Col = 0; Col < line.size(); Col++)		//line.size() == NumRows == 32
				{
					Cols.push_back(line[Col] - 48);				// val = data - 48 seems to be some encoding, subtract it off, undo it
				}
				RowOfCols.push_back(Cols);
				Cols.clear();
				getline(Data, line);
			}
			InputData.Images.push_back(RowOfCols);
			RowOfCols.clear();
			//getline(Data, line);				// now line == <CorrectClassifcation>
			InputData.CorrectClassifications.push_back(line[1] - 48);
			while (getline(Data, line))			
			{	// Iterate through the remaining, non-header lines
				for (int Row = 0; Row < 32; Row++)
				{
					for (int Col = 0; Col < line.size(); Col++)		//line.size() == NumRows == 32
					{
						Cols.push_back(line[Col] - 48);				// val = data - 48 seems to be some encoding, subtract it off, undo it
					}
					RowOfCols.push_back(Cols);
					Cols.clear();
					getline(Data, line);
				}

				InputData.Images.push_back(RowOfCols);
				RowOfCols.clear();
				// now line == <CorrectClassifcation>
				InputData.CorrectClassifications.push_back(line[1] - 48);
			}
			Data.close();
			return InputData;
		}
	}
	else
	{
		std::cout << "Warning - could not open data file " << Dir << FileName << endl;
		std::cout << "Returning empty ImageData object." << endl;
		return InputData;
	}
}