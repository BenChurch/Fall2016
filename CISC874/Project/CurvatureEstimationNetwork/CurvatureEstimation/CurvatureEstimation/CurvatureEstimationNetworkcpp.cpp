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

	// Each of these writes 2 .csv files, one for coords, one for angles, in a fashion suitable for MATLAB use 
	void WriteTestingData(string FileID);
	void WriteTrainingData(string FileID);
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

void LandmarkSets::WriteTestingData(string FileID)
{
	ofstream CoordsOutput, AnglesOutput;
	CoordsOutput.open("TestingCoords_" + FileID + ".txt");
	AnglesOutput.open("TestingAngles_" + FileID + ".txt");
	string line;

	LandmarksNode CurrentSetNode;
	LandmarkPoint CurrentLandmarkPoint;
	for (int TestSet = 0; TestSet < this->TestingData.size(); TestSet++)
	{
		CurrentSetNode = this->TestingData[TestSet];
		//cout << CurrentSetNode.Name << " (testing set)	" << "True curvature: " << CurrentSetNode.TrueCurvature << endl;
		//cout << "		" << "RL" << "		" << "AP" << "		" << "SI" << endl;
		for (int LandmarkPoint = 0; LandmarkPoint < CurrentSetNode.LandmarkPoints.size(); LandmarkPoint++)
		{
			CurrentLandmarkPoint = CurrentSetNode.LandmarkPoints[LandmarkPoint];
			//cout << CurrentLandmarkPoint.Name << "	";
			line += to_string((CurrentLandmarkPoint).Position[0]) + ", ";
			line += to_string((CurrentLandmarkPoint).Position[1]) + ", ";
			line += to_string((CurrentLandmarkPoint).Position[0]);
			CoordsOutput << line << endl;
			line.clear();		
		}
		AnglesOutput << CurrentSetNode.TrueCurvature << endl;
	}
	CoordsOutput.close();
	AnglesOutput.close();
}

void LandmarkSets::WriteTrainingData(string FileID)
{
	ofstream CoordsOutput, AnglesOutput;
	CoordsOutput.open("TrainingCoords_" + FileID + ".txt");
	AnglesOutput.open("TrainingAngles_" + FileID + ".txt");
	string line;

	LandmarksNode CurrentSetNode;
	LandmarkPoint CurrentLandmarkPoint;
	for (int TestSet = 0; TestSet < this->TrainingData.size(); TestSet++)
	{
		CurrentSetNode = this->TrainingData[TestSet];
		//cout << CurrentSetNode.Name << " (testing set)	" << "True curvature: " << CurrentSetNode.TrueCurvature << endl;
		//cout << "		" << "RL" << "		" << "AP" << "		" << "SI" << endl;
		for (int LandmarkPoint = 0; LandmarkPoint < CurrentSetNode.LandmarkPoints.size(); LandmarkPoint++)
		{
			CurrentLandmarkPoint = CurrentSetNode.LandmarkPoints[LandmarkPoint];
			//cout << CurrentLandmarkPoint.Name << "	";
			line += to_string((CurrentLandmarkPoint).Position[0]) + ", ";
			line += to_string((CurrentLandmarkPoint).Position[1]) + ", ";
			line += to_string((CurrentLandmarkPoint).Position[0]);
			CoordsOutput << line << endl;
			line.clear();
		}
		AnglesOutput << CurrentSetNode.TrueCurvature << endl;
	}
	CoordsOutput.close();
	AnglesOutput.close();
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
	for (int Weight = 0; Weight < InitialWeights.size(); Weight++)
	{
		this->Inputs.push_back(0);	// Initialize inputs while we're at it
		this->Weights.push_back(InitialWeights[Weight]);
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
	double Momentum = MOMENTUM;				// THIS WONT WORK EACH WEIGHT NEEDS MOMENTUM
	
	double AverageEstimationError = 0;

	void ConstructNetwork();

	void Feedforward(vector<LandmarkPoint> PatientLandmarks);
	void Backpropagate(double CorrectAngle);

	void WriteSelf(string FileIdentifier);
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

  for (int OutputNode = 0; OutputNode < 1; OutputNode++)		// Hack - just need one output node
  {
	  for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size()-1].size(); HiddenNode++)
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

void FeedforwardLayeredNetwork::Feedforward(vector<LandmarkPoint> PatientLandmarks)
{
	LandmarkPoint * CurrentLandmarkPoint;
	for (int Landmark = 0; Landmark < PatientLandmarks.size(); Landmark++)
	{
		// We will assume that Landmarks are ordered properly, L-R-L-R-etc...
		for (int dim = 0; dim < 3; dim++)
		{
			this->InputLayer[Landmark][dim].Inputs[0] = PatientLandmarks[Landmark].Position[dim];
			this->InputLayer[Landmark][dim].ComputeIdentityActivation();
			for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[0].size(); HiddenNode++)
			{	// Seems a little odd to put this here, but makes looping through hidden layers easier
				this->HiddenLayers[0][HiddenNode].Inputs[(Landmark * 3) + dim] += this->HiddenLayers[0][HiddenNode].Weights[(Landmark * 3) + dim] * this->InputLayer[Landmark][dim].ActivationPotential;
			}
		}
	}	// ASSERT all InputNodes are at equilibrium with input

	Node * CurrentInputNode;
	Node * CurrentHiddenNode;
	for (int HiddenLayer = 0; HiddenLayer < this->HiddenLayers.size() - 1; HiddenLayer++)
	{	// Minus one to deal with the boundary condition of the output layer, as was done with odd hidden input initialization
		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
		{	
			this->HiddenLayers[HiddenLayer][HiddenNode].ComputeSigmoidalActivation();
			for (int NextLayerNode = 0; NextLayerNode < this->HiddenLayers[HiddenLayer + 1].size(); NextLayerNode++)
			{
				this->HiddenLayers[HiddenLayer + 1][NextLayerNode].Inputs[HiddenNode] += this->HiddenLayers[HiddenLayer + 1][NextLayerNode].Weights[HiddenNode] * this->HiddenLayers[HiddenLayer][HiddenNode].ActivationPotential;
			}
		}
	}	// ASSERT all HiddenLayers Node's inputs are at equilibrium, and all HiddenLayers Node's Activations at equilibrium except last layer

	for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size() - 1].size(); HiddenNode++)
	{
		this->HiddenLayers[this->HiddenLayers.size() - 1][HiddenNode].ComputeSigmoidalActivation();
		for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
		{	// Should only contain one node for one angle
			this->OutputLayer[OutputNode].Inputs[HiddenNode] += this->OutputLayer[OutputNode].Weights[HiddenNode] * this->HiddenLayers[this->HiddenLayers.size() - 1][HiddenNode].ActivationPotential;
		}
	}	// ASSERT all HiddenLayers Nodes inputs and activations and OutputLayer's inputs in equilibrium

	for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
	{
		this->OutputLayer[OutputNode].ComputeSigmoidalActivation();
	}	// ASSERT entire network in equilibrium
}

void FeedforwardLayeredNetwork::Backpropagate(double CorrectAngle)
{
	double SumSquaredError = 0;
	double LastWeightChange = 0;			// Used with momentum feature
	double Output;

	Node * CurrentOutputNode;
	Node * CurrentHiddenNode;
	Node * FeedingNode;						// Points to node who feeds input corresponding to weight being changed
	double NewDelta;

	vector<double> ErrorVector;				// Stores output nodes errors, and used to calculate deltas
	vector<vector<double>> HiddenDeltas;	// Stores weight change factors computed from gradient descent
	vector<double> HiddenDeltaLayer;		// Stores each hidden layers deltas to push onto HiddenDeltas

	for (int OutputNode = 0; OutputNode < this->OutputLayer.size(); OutputNode++)
	{	// OutputNode can only be 0, with CorrectAngle as double
		CurrentOutputNode = &(this->OutputLayer[OutputNode]);
		Output = ((*CurrentOutputNode).ActivationPotential - 0.5) * 360;	// Subtract 0.5 and multiply by 180 to map [0,1] to [-180,180]
		SumSquaredError += (CorrectAngle - Output) * (CorrectAngle - Output);
		ErrorVector.push_back(CorrectAngle - Output);
		HiddenDeltaLayer.push_back(ErrorVector[OutputNode] * (*CurrentOutputNode).ActivationPotential * (1 - (*CurrentOutputNode).ActivationPotential));
		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size() - 1].size(); HiddenNode++)
		{
			(*CurrentOutputNode).Weights[HiddenNode] += (this->LearningRate) * ((*CurrentOutputNode).Inputs[HiddenNode]) * (HiddenDeltaLayer[OutputNode])
				+ (this->Momentum * LastWeightChange);
			LastWeightChange = (this->LearningRate) * ((*CurrentOutputNode).Inputs[HiddenNode]) * (HiddenDeltaLayer[OutputNode])
				+ (this->Momentum * LastWeightChange);
		}
	}	// ASSERT that all output nodes' weights have been adjusted

	HiddenDeltas.push_back(HiddenDeltaLayer);
	HiddenDeltaLayer.clear();
	for (int HiddenLayer = this->HiddenLayers.size() - 1; HiddenLayer >= 1; HiddenLayer--)
	{
		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
		{
			NewDelta = 0;
			CurrentHiddenNode = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
			for (int RemainingHiddenLayer = this->HiddenLayers.size() - HiddenLayer - 1; RemainingHiddenLayer >= 0; RemainingHiddenLayer--)
			{
				for (int RemainingLayerNode = 0; RemainingLayerNode < this->HiddenLayers[RemainingHiddenLayer].size(); RemainingLayerNode++)
				{
					NewDelta += HiddenDeltas[RemainingHiddenLayer][RemainingLayerNode] * (this->HiddenLayers[RemainingHiddenLayer][RemainingLayerNode].Weights[HiddenNode]);
				}
			}
			HiddenDeltaLayer.push_back(NewDelta * (*CurrentHiddenNode).ActivationPotential * (1 - (*CurrentHiddenNode).ActivationPotential));
		}
		HiddenDeltas.push_back(HiddenDeltaLayer);
		HiddenDeltaLayer.clear();
		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
		{
			CurrentHiddenNode = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
			for (int HiddenNodeWeight = 0; HiddenNodeWeight < (*CurrentHiddenNode).Weights.size(); HiddenNodeWeight++)
			{
				FeedingNode = &(this->HiddenLayers[HiddenLayer - 1][HiddenNodeWeight]);
				(*CurrentHiddenNode).Weights[HiddenNodeWeight] += (this->LearningRate) * (HiddenDeltas[this->HiddenLayers.size() - HiddenLayer - 1][HiddenNode])* ((*FeedingNode).ActivationPotential);
			}
		}
	}	// ASSERT that all weights have been updated except those connecting input to first layer of hidden nodes

	for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[0].size(); HiddenNode++)
	{
		NewDelta = 0;
		CurrentHiddenNode = &(this->HiddenLayers[0][HiddenNode]);
		for (int RemainingHiddenLayer = this->HiddenLayers.size() - 1; RemainingHiddenLayer >= 0; RemainingHiddenLayer--)
		{
			for (int RemainingLayerNode = this->HiddenLayers.size() - 1; RemainingLayerNode > 0; RemainingLayerNode--)
			{
				NewDelta += HiddenDeltas[RemainingHiddenLayer][RemainingLayerNode] * (this->HiddenLayers[RemainingHiddenLayer][RemainingLayerNode].Weights[HiddenNode]);		// CANT BE RIGHT, HiddenNode not connected to HiddenLayerss[][]
			}
		}
		HiddenDeltaLayer.push_back(NewDelta * (*CurrentHiddenNode).ActivationPotential * (1 - (*CurrentHiddenNode).ActivationPotential));
	}
	HiddenDeltas.push_back(HiddenDeltaLayer);
	HiddenDeltaLayer.clear();

	for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[0].size(); HiddenNode++)
	{
		CurrentHiddenNode = &(this->HiddenLayers[0][HiddenNode]);
		for (int HiddenNodeWeight = 0; HiddenNodeWeight < (*CurrentHiddenNode).Weights.size(); HiddenNodeWeight++)
		{
			FeedingNode = &(this->InputLayer[HiddenNodeWeight / 3][HiddenNodeWeight % 3]);
			(*CurrentHiddenNode).Weights[HiddenNodeWeight] += (this->LearningRate) * (HiddenDeltas[HiddenDeltas.size() - 1][HiddenNode])* ((*FeedingNode).ActivationPotential);
		}
	}	// ASSERT that all weights have been updated
}

void FeedforwardLayeredNetwork::WriteSelf(string FileIdentifier)						// Just used for my debugging purposes. Output doesn't necessarily line up - can be confusing
{
	ofstream SelfOutput;
	SelfOutput.open("NetworkState" + FileIdentifier + ".txt");

	vector<string> Dims = { "RL", "AP", "SI" };
	vector<string> Vertebrae = { "T1L", "T1R", "T2L", "T2R", "T3L", "T3R", "T4L", "T4R", "T5L", "T5R", "T6L", "T6R", "T7L", "T7R", "T8L", "T8R", "T9L", "T9R",
		"T10L", "T10R", "T11L", "T11R", "T12L", "T12R", "L1L", "L1R", "L2L", "L2R", "L3L", "L3R", "L4L", "L4R", "L5L", "L5R" };

	vector<Node> * CurrentNodeTrio;
	Node * CurrentNode;

	SelfOutput << "Input layer(s):" << endl;

	// Write input array
	SelfOutput << endl << "Vertebra:" << endl << "	";
	for (int Vertebra = 0; Vertebra < Vertebrae.size(); Vertebra += 2)
	{
		SelfOutput << "		" << Vertebrae[Vertebra] << "		";
	}

	SelfOutput << endl;
	SelfOutput << "InputL:		";
	for (int LeftInputPoint = 0; LeftInputPoint < this->InputLayer.size(); LeftInputPoint+=2)
	{	// Inputs
		(CurrentNodeTrio) = &(this->InputLayer[LeftInputPoint]);
		
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{	
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Inputs[0] << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "WeightL:	";
	for (int LeftInputPoint = 0; LeftInputPoint < this->InputLayer.size(); LeftInputPoint += 2)
	{	// Weights
		(CurrentNodeTrio) = &(this->InputLayer[LeftInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Weights[0] << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "ThresholdL:	";
	for (int LeftInputPoint = 0; LeftInputPoint < this->InputLayer.size(); LeftInputPoint += 2)
	{	// Threshold
		(CurrentNodeTrio) = &(this->InputLayer[LeftInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Threshold << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "ActivationL:	";
	for (int LeftInputPoint = 0; LeftInputPoint < this->InputLayer.size(); LeftInputPoint += 2)
	{	// Activation
		(CurrentNodeTrio) = &(this->InputLayer[LeftInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).ActivationPotential << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl << endl;
	SelfOutput << "InputR:		";
	for (int RightInputPoint = 1; RightInputPoint < this->InputLayer.size(); RightInputPoint += 2)
	{	// Inputs
		(CurrentNodeTrio) = &(this->InputLayer[RightInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Inputs[0] << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "WeightR:	";
	for (int RightInputPoint = 0; RightInputPoint < this->InputLayer.size(); RightInputPoint += 2)
	{	// Weights
		(CurrentNodeTrio) = &(this->InputLayer[RightInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Weights[0] << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "ThresholdR:	";
	for (int RightInputPoint = 0; RightInputPoint < this->InputLayer.size(); RightInputPoint += 2)
	{	// Threshold
		(CurrentNodeTrio) = &(this->InputLayer[RightInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).Threshold << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;
	SelfOutput << "ActivationR:	";
	for (int RightInputPoint = 0; RightInputPoint < this->InputLayer.size(); RightInputPoint += 2)
	{	// Activation
		(CurrentNodeTrio) = &(this->InputLayer[RightInputPoint]);
		for (int InputNode = 0; InputNode < (*CurrentNodeTrio).size(); InputNode++)
		{
			(CurrentNode) = &(*CurrentNodeTrio)[InputNode];
			SelfOutput << (*CurrentNode).ActivationPotential << "	";		// Input nodes having always 1 input
		}
		SelfOutput << "	";
	}

	SelfOutput << endl;

	// Write hidden layer
	SelfOutput << endl << "Hidden layer(s): " << endl;
	SelfOutput << endl;
	
	for (int HiddenLayer = 0; HiddenLayer < this->HiddenLayers.size(); HiddenLayer++)
	{
		SelfOutput << "Hidden layer #" << HiddenLayer << endl;
		SelfOutput << "Inputs:	" << endl;
		for (int InputNode = 0; InputNode < this->InputLayer.size() * this->InputLayer[0].size(); InputNode++)
		{	// Inputs:
			SelfOutput << InputNode << "		";
			for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
			{
				(CurrentNode) = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
				SelfOutput << (*CurrentNode).Inputs[InputNode] << "	";
			}
			SelfOutput << endl;
		}

		SelfOutput << endl;
		SelfOutput << "Weights:	" << endl;
		for (int InputNode = 0; InputNode < this->InputLayer.size() * this->InputLayer[0].size(); InputNode++)
		{	// Weights:
			SelfOutput << InputNode << "		";
			for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
			{
				(CurrentNode) = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
				SelfOutput << (*CurrentNode).Weights[InputNode] << "	";
			}
			SelfOutput << endl;
		}
		SelfOutput << endl;
		SelfOutput << "Threshold:	";

		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
		{
			(CurrentNode) = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
			SelfOutput << (*CurrentNode).Threshold << "	";
		}

		SelfOutput << endl;
		SelfOutput << "Activation:	";
		for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[HiddenLayer].size(); HiddenNode++)
		{
			(CurrentNode) = &(this->HiddenLayers[HiddenLayer][HiddenNode]);
			SelfOutput << (*CurrentNode).ActivationPotential << "	";
		}
	}
	

	// Write output layer
	SelfOutput << endl;
	SelfOutput << endl << "Output layer: " << endl;
	SelfOutput << endl;

	SelfOutput << "Inputs:	" << endl;
	for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size()-1].size(); HiddenNode++)
	{	// Inputs:
		SelfOutput << HiddenNode << "		";
		SelfOutput << this->OutputLayer[0].Inputs[HiddenNode] << endl;		// Hack it to OutputLayer[0] for now, only the one output angle
	}

	SelfOutput << endl;
	SelfOutput << "Weights:	" << endl;
	for (int HiddenNode = 0; HiddenNode < this->HiddenLayers[this->HiddenLayers.size()-1].size(); HiddenNode++)
	{	// Weights:
		SelfOutput << HiddenNode << "		";
		SelfOutput << this->OutputLayer[0].Weights[HiddenNode] << endl;		// Hack it to OutputLayer[0] for now, only the one output angle
	}
	SelfOutput << endl;
	SelfOutput << "Threshold:	" << this->OutputLayer[0].Threshold;

	SelfOutput << endl;
	SelfOutput << "Activation:	" << this->OutputLayer[0].ActivationPotential;
	SelfOutput << endl;

	SelfOutput.close();
}

int main()
{
	LandmarkSets InputLandmarkSets;
	InputLandmarkSets.ReadInputData(INPUT_FILE_NAME);

	for (int i = 0; i < 10; i++)
	{	// Use a for-loop to write data to MATLAB csv files - DANGEROUS - make sure terminates - includes user input continuation
		cout << "Press enter to generate file set " << i << " or press crtl + c to terminate program." << endl;
		cin.ignore();

		InputLandmarkSets.SeperateTestAndTrainData(0.2);
		InputLandmarkSets.WriteTestingData(to_string(i));
		InputLandmarkSets.WriteTrainingData(to_string(i));
	}

	FeedforwardLayeredNetwork AngleEstimator;
	AngleEstimator.ConstructNetwork();
	AngleEstimator.WriteSelf("1");
	//AngleEstimator.Feedforward(InputLandmarkSets.MarkupNodes[0].LandmarkPoints);
	//AngleEstimator.WriteSelf("2");
	//AngleEstimator.Backpropagate(InputLandmarkSets.MarkupNodes[0].TrueCurvature);
	//AngleEstimator.WriteSelf("3");

	cout << "Press enter to end the program." << endl;
	cin.ignore();
	return 0;
}

