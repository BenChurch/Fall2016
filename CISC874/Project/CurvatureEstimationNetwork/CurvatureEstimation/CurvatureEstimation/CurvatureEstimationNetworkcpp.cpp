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

class LandmarkSets
{
	vector<LandmarksNode> MarkupNodes;
};

class LandmarksNode
{
public:
	string Name;
	double TrueCurvature;
	vector<LandmarkPoint> LandmarkPoints;
};

class LandmarkPoint
{
public:
	string Name;
	double Position[3];
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
	LandmarkSets InputLandmarkSets = ReadInputData(INPUT_FILE_NAME);

	cout << "Press enter to end the program." << endl;
	cin.ignore();
	return 0;
}

LandmarkSets ReadInputData(const char * FileName)
{
	LandmarkSets InputLandmarkSets;
	SetCurrentDirectoryA(Dir);
	ifstream InputData(FileName);
	string Line;
	while (getline(InputData, Line))
	{
		stringstream LineStream(Line);
		string Cell;
		while (getline(LineStream, Cell, ','))
		{
			if (Cell == "Max angle:")
			{	// We've found the start of one set's data
				LandmarksNode CurrentLandmarkSet;
				getline(LineStream, Cell, ',');
				CurrentLandmarkSet.TrueCurvature = atof(Cell.c_str());
			}
		}
	}
}