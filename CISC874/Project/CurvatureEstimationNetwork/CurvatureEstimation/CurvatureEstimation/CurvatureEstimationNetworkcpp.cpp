#include <fstream>
#include <iostream>
#include <math.h>		// For computation of nodes' sigmoidal-exponential activation function
#include <stdlib.h>		// For .txt file strings to int representations, and random number generation
#include <sstream>		// For .csv file reading
#include <string>
#include <time.h>		// Used to seed random number generator
#include <vector>
#include <Windows.h>

using namespace std;

static const char * Dir = ".";	// Using current directory
static const char * INPUT_FILE_NAME = "Input.csv";
static const char * OUTPUT_FILE_NAME = "Output.txt";

static const int MAX_TRAINING_EPOCHS = 100;

static const int NUM_HIDDEN_LAYERS = 1;
static const int NUMS_HIDDEN_NODES[1] = {10};
static const double LEARNING_RATE = 0.15;
static const double MOMENTUM = 0.1;
static const double INITIAL_THRESHOLDS = 0.5;   // Initial threshold value for NON-INPUT nodes

class LandmarkPoint
{
public:
	string Name;
	double Position[3];
};

class LandmarksNode
{
public:
	string Name;
	double TrueCurvature;
	vector<LandmarkPoint> LandmarkPoints;
};

class LandmarkSets
{
public:
	vector<LandmarksNode> MarkupNodes;
	
	vector<LandmarksNode> TrainingData;
	vector<LandmarksNode> TestingData;

	void ReadInputData(const char * FileName);
	void PrintAllData();

	void SeperateTestAndTrainData(double TestFraction);
	void ShuffleTestingData();
	void ShuffleTrainingData();
	void PrintTestingData();
	void PrintTrainingData();
};

void LandmarkSets::ReadInputData(const char * FileName)
{
	SetCurrentDirectoryA(Dir);
	ifstream InputData(FileName);
	string Line;
	if (InputData.is_open())
	{
		if (!getline(InputData, Line))
		{
			cout << "Input data file contains no recognizable data sets." << endl;
			cout << "	Data read returning empty data structure." << endl;
			InputData.close();
			return;
		}
		//cout << Line << endl;
		stringstream LineStream(Line);
		string Cell;
		getline(LineStream, Cell, ',');
		while ((Cell != "Max angle:") > 0)
		{	// Iterate through headers
			Cell.clear();
			if (!getline(InputData, Line))
			{
				cout << "Input data file contains no recognizable data sets." << endl;
				cout << "	Data read returning empty data structure." << endl;
				InputData.close();
				return;
			}
			LineStream.ignore();
			stringstream LineStream(Line);
			getline(LineStream, Cell, ',');
		}
		// ASSERT we are at the first landmark set and the input set is not empty
		LineStream.str(Line);
		getline(LineStream, Cell, ',');
		Cell.clear();
		getline(LineStream, Cell, ',');
		LandmarksNode CurrentLandmarkSet;
		CurrentLandmarkSet.TrueCurvature = atof(Cell.c_str());
		getline(LineStream, Cell, ',');
		CurrentLandmarkSet.Name = Cell;

		while (getline(InputData, Line))
		{	// The structure of the rest of the document is known
			while (Cell != "Max angle:" && Cell != "EOF")
			{
				stringstream LineStream(Line);
				Cell.clear();
				getline(LineStream, Cell, ',');
				LandmarkPoint CurrentLandmarkPoint;
				CurrentLandmarkPoint.Name = Cell;
				for (int dim = 0; dim < 3; dim++)
				{
					getline(LineStream, Cell, ',');
					CurrentLandmarkPoint.Position[dim] = atof(Cell.c_str());
				}
				CurrentLandmarkSet.LandmarkPoints.push_back(CurrentLandmarkPoint);
				getline(InputData, Line);
				LineStream.ignore();
				LineStream.str(Line);
				Cell = Line.substr(0, Line.find_first_of(','));
			}
			this->MarkupNodes.push_back(CurrentLandmarkSet);
			CurrentLandmarkSet.LandmarkPoints.clear();
			if (Cell == "EOF")
			{
				InputData.close();
				return;
			}
			else
			{	// Cell == "Max angle"
				stringstream LineStream(Line);
				getline(LineStream, Cell, ',');
				Cell.clear();
				getline(LineStream, Cell, ',');
				CurrentLandmarkSet.TrueCurvature = atof(Cell.c_str());
				getline(LineStream, Cell, ',');
				CurrentLandmarkSet.Name = Cell;
			}
		}
		InputData.close();
	}

}

void LandmarkSets::PrintAllData()
{
	LandmarksNode CurrentMarkupsNode;
	LandmarkPoint CurrentLandmarkPoint;
	for (int LandmarkSet = 0; LandmarkSet < this->MarkupNodes.size(); LandmarkSet++)
	{
		CurrentMarkupsNode = this->MarkupNodes[LandmarkSet];
		cout << CurrentMarkupsNode.Name << "	True curvature: " << CurrentMarkupsNode.TrueCurvature << endl;
		cout << "		" << "RL" << "		" << "AP" << "		" << "SI" << endl;
		for (int LandmarkPoint = 0; LandmarkPoint < CurrentMarkupsNode.LandmarkPoints.size(); LandmarkPoint++)
		{
			CurrentLandmarkPoint = CurrentMarkupsNode.LandmarkPoints[LandmarkPoint];
			cout << CurrentLandmarkPoint.Name << "	";
			for (int dim = 0; dim < 3; dim++)
			{
				cout << CurrentLandmarkPoint.Position[dim] << "	";
			}
			cout << endl;
		}
		cout << endl;
	}
}

void LandmarkSets::SeperateTestAndTrainData(double TestFraction)
{
	if (TestFraction < 0 || TestFraction > 1)
	{
		cout << "Error - invalid test fraction, cannot have fewer than none or more than all data in a set." << endl;
		cout << "	Therefore doing nothing." << endl;
		return;
	}

	int TestSetIndex;
	int TrainSetIndex;
	int TestAmount = TestFraction * this->MarkupNodes.size();
	vector<LandmarksNode> RestorationSets = (*this).MarkupNodes;	// this.MarkupsNodes will be depopulated during assignment, need to restore it after
	vector<LandmarksNode>::iterator SetIt;
	while (TestAmount > 0)
	{
		SetIt = this->MarkupNodes.begin();
		TestSetIndex = (rand() % this->MarkupNodes.size());
		SetIt += TestSetIndex;
		this->TestingData.push_back(this->MarkupNodes[TestSetIndex]);
		this->MarkupNodes.erase(SetIt);
		TestAmount--;
	}
	while (this->MarkupNodes.size() > 0)
	{	// The remaining sets go to training data
		SetIt = this->MarkupNodes.begin();
		TrainSetIndex = (rand() % this->MarkupNodes.size());
		SetIt += TrainSetIndex;
		this->TrainingData.push_back(this->MarkupNodes[TrainSetIndex]);
		this->MarkupNodes.erase(SetIt);
	}
	this->MarkupNodes = RestorationSets;
}

void LandmarkSets::ShuffleTestingData()
{
	vector<LandmarksNode> ShuffledTestData;
	vector<LandmarksNode>::iterator SetIt;
	LandmarksNode RandomLandmarkSet;
	int RandomLandmarkSetIndex;
	int OrigingalTestingDataSize = this->TestingData.size();
	for (int LandmarkSet = 0; LandmarkSet < OrigingalTestingDataSize; LandmarkSet++)
	{
		SetIt = this->TestingData.begin();
		RandomLandmarkSetIndex = (rand() % this->TestingData.size());
		SetIt += RandomLandmarkSetIndex;
		RandomLandmarkSet = this->TestingData[RandomLandmarkSetIndex];
		ShuffledTestData.push_back(RandomLandmarkSet);
		this->TestingData.erase(SetIt);
	}
	this->TestingData = ShuffledTestData;
}

void LandmarkSets::ShuffleTrainingData()
{
	vector<LandmarksNode> ShuffledTrainData;
	vector<LandmarksNode>::iterator SetIt;
	LandmarksNode RandomLandmarkSet;
	int RandomLandmarkSetIndex;
	int OrigingalTraininDataSize = this->TrainingData.size();
	for (int LandmarkSet = 0; LandmarkSet < OrigingalTraininDataSize; LandmarkSet++)
	{
		SetIt = this->TrainingData.begin();
		RandomLandmarkSetIndex = (rand() % this->TrainingData.size());
		SetIt += RandomLandmarkSetIndex;
		RandomLandmarkSet = this->TrainingData[RandomLandmarkSetIndex];
		ShuffledTrainData.push_back(RandomLandmarkSet);
		this->TrainingData.erase(SetIt);
	}
	this->TrainingData = ShuffledTrainData;
}

void LandmarkSets::PrintTestingData()
{
	LandmarksNode CurrentSetNode;
	LandmarkPoint CurrentLandmarkPoint;
	for (int TestSet = 0; TestSet < this->TestingData.size(); TestSet++)
	{
		CurrentSetNode = this->TestingData[TestSet];
		cout << CurrentSetNode.Name << " (testing set)	" << "True curvature: " << CurrentSetNode.TrueCurvature << endl;
		cout << "		" << "RL" << "		" << "AP" << "		" << "SI" << endl;
		for (int LandmarkPoint = 0; LandmarkPoint < CurrentSetNode.LandmarkPoints.size(); LandmarkPoint++)
		{
			CurrentLandmarkPoint = CurrentSetNode.LandmarkPoints[LandmarkPoint];
			cout << CurrentLandmarkPoint.Name << "	";
			for (int dim = 0; dim < 3; dim++)
			{
				cout << CurrentLandmarkPoint.Position[dim] << "	";
			}
			cout << endl;
		}
		cout << endl;
	}
}

void LandmarkSets::PrintTrainingData()
{
	LandmarksNode CurrentSetNode;
	LandmarkPoint CurrentLandmarkPoint;
	for (int TrainSet = 0; TrainSet < this->TrainingData.size(); TrainSet++)
	{
		CurrentSetNode = this->TrainingData[TrainSet];
		cout << CurrentSetNode.Name << " (training set)	" << "True curvature: " << CurrentSetNode.TrueCurvature << endl;
		cout << "		" << "RL" << "		" << "AP" << "		" << "SI" << endl;
		for (int LandmarkPoint = 0; LandmarkPoint < CurrentSetNode.LandmarkPoints.size(); LandmarkPoint++)
		{
			CurrentLandmarkPoint = CurrentSetNode.LandmarkPoints[LandmarkPoint];
			cout << CurrentLandmarkPoint.Name << "	";
			for (int dim = 0; dim < 3; dim++)
			{
				cout << CurrentLandmarkPoint.Position[dim] << "	";
			}
			cout << endl;
		}
		cout << endl;
	}
}

class Node
{
public:
	// Default constructor needs Weight and Threhsold values
	Node(vector<double> InitialWeights, double InitialThreshold);
	vector<double> Inputs;
	vector<double> Weights;
	double Threshold;
	double ActivationPotential = 0;

	void ComputeIdentityActivation();
	void ComputeSigmoidalActivation();
};

Node::Node(vector<double> InitialWeights, double InitialThreshold)
{
	for (int Weight = 0; Weight < this->Weights.size(); Weight++)
	{
		this->Inputs[Weight] = 0;	// Initialize inputs while we're at it
		this->Weights[Weight] = InitialWeights[Weight];
	}
	this->Threshold = InitialThreshold;
}

void Node::ComputeIdentityActivation()
{
	// Reinitialize activation potential
	this->ActivationPotential = 0;
	for (int Input = 0; Input < this->Inputs.size(); Input++)
	{
		this->ActivationPotential += (this->Inputs[Input]) * (this->Weights[Input]);
	}
}

void Node::ComputeSigmoidalActivation()
{
	// Reinitialize activation potential
	this->ActivationPotential = 0;
	double Net = 0;
	for (int Input = 0; Input < this->Inputs.size(); Input++)
	{
		 Net += (this->Inputs[Input]) * (this->Weights[Input]);
	}
	this->ActivationPotential = 1.0 / (1 + exp((-1) * Net));
}

class FeedforwardLayeredNetwork
{
public:
	FeedforwardLayeredNetwork();

	vector<vector<Node>> InputLayer;     // Try organizing input analogously to spinal geometry    InputLayer[Vertebra][0] == Left     InputLayer[Vertebra][1] == Right
	vector<vector<Node>> HiddenLayers;
	vector<Node> OutputLayer;				// Should just contain one node if estimating curvature angle

	double LearningRate = LEARNING_RATE;
	double Momentum = MOMENTUM;

	void ConstructNetwork();

	void Feedforward();
	void Backpropagate();

private:

};
FeedforwardLayeredNetwork::FeedforwardLayeredNetwork()
{
	SetCurrentDirectoryA(Dir);
	srand(time(NULL));			// Provide random number seed for random weight initialization
}

void FeedforwardLayeredNetwork::ConstructNetwork()
{
	// Currently assuming that all input points contain the same number of points
  int RandomIntLeft;
  double RandomDoubleLeft;
  int RandomIntRight;
  double RandomDoubleRight;
  vector<double> InitialInputWeightLeft;
  vector<double> InitialInputWeightRight;
  vector<Node> CurrentLeftPoint;
  vector<Node> CurrentRightPoint;
  for (int LandmarkPoint = 0; LandmarkPoint < 34; LandmarkPoint+=2) // 34 being 2 * numberOfVertebrae           
  {
	for (int dim = 0; dim < 3; dim++)
	{
		RandomIntLeft = (rand() % 100);     // Random number in the range of [0,100]
		RandomDoubleLeft = (double)(RandomIntLeft) / 100.0;
		RandomIntRight = (rand() % 100);     // Random number in the range of [0,100]
		RandomDoubleRight = (double)(RandomIntRight) / 100.0;
		CurrentLeftPoint.push_back(Node({RandomDoubleLeft}, 0));
		CurrentRightPoint.push_back(Node({ RandomDoubleRight }, 0));
	}
	this->InputLayer.push_back(CurrentLeftPoint);
	this->InputLayer.push_back(CurrentRightPoint);
	CurrentLeftPoint.clear();
	CurrentRightPoint.clear();
  }


  vector<Node> CurrentHiddenLayer;
  int RandomInt;
  double RandomDouble;
  vector<double> CurrentNodeInitialWeights;
  for (int HiddenLayer = 0; HiddenLayer < NUM_HIDDEN_LAYERS; HiddenLayer++)
  {
    for (int LayerNode = 0; LayerNode < NUMS_HIDDEN_NODES[HiddenLayer]; LayerNode++)
    {
		for (int InputNode = 0; InputNode < this->InputLayer.size() * 6; InputNode++)
		{
			RandomInt = (rand() % 100);
			RandomDouble = (double)(RandomInt) / 100.0;
			CurrentNodeInitialWeights.push_back(RandomDouble);
		}
		Node CurrentLayerNode(CurrentNodeInitialWeights, 0.5);
		CurrentNodeInitialWeights.clear();
		CurrentHiddenLayer.push_back(CurrentLayerNode);
    }
	this->HiddenLayers.push_back(CurrentHiddenLayer);
  }

  for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
  {
	  for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size()].size(); HiddenNode++)
	  {
		  RandomInt = (rand() % 100);
		  RandomDouble = (double)(RandomInt) / 100.0;
		  CurrentNodeInitialWeights.push_back(RandomDouble);
	  }
	  Node CurrentOutputNode(CurrentNodeInitialWeights, 0);
	  CurrentNodeInitialWeights.clear();
	  this->OutputLayer.push_back(CurrentOutputNode);
  }

}

int main()
{
	LandmarkSets InputLandmarkSets;
	InputLandmarkSets.ReadInputData(INPUT_FILE_NAME);
	InputLandmarkSets.SeperateTestAndTrainData(0.2);

	cout << "Press enter to end the program." << endl;
	cin.ignore();
	return 0;
}

