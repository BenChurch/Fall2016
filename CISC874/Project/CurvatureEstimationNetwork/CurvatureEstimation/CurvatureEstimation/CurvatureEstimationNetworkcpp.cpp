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
	
	void ReadInputData(const char * FileName);
};

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

	vector<Node> InputLayer;
	vector<vector<Node>> HiddenLayers;
	vector<Node> OutputLayer;

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
	// Need way to read data in first
}

int main()
{
	LandmarkSets InputLandmarkSets;
	InputLandmarkSets.ReadInputData(INPUT_FILE_NAME);

	cout << "Press enter to end the program." << endl;
	cin.ignore();
	return 0;
}

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
		while ((Cell != "MaxAngle:") > 0)
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
			while (Cell != "MaxAngle:" && Cell != "EOF")
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