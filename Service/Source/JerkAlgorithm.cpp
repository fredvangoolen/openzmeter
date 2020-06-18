#include "JerkAlgorithm.h"
#include <chrono>

vector<int> JerkAlgorithm::ComputeDVDT(vector<int> input)
{
    vector<int> result = vector<int>();

    for (int i = 1; i < input.size(); i++)
    {
        int e1 = input[i - 1];
        int e2 = input[i];
        result.push_back(e2 - e1);
    }
}