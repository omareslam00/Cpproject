#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class material {
public:
    int index;
    string name;
    float yield;
    float density;

    
    material(int i = 0, string n = "", float y = 0, float d = 0) {
        index = i;
        name = n;
        yield = y;
        density = d;
    }
};

vector<material> loadData(string filename) {
    vector<material> dataList;
    ifstream file(filename);
    string line;

   
    getline(file, line); 

    while (getline(file, line)) {
        

        stringstream ss(line);
        string n, i, y, d;

        getline(ss, i, ',');
        getline(ss, n, ',');
        getline(ss, y, ',');
        getline(ss, d, ',');

        
        
        dataList.push_back(material(stoi(i), n, stof(y), stof(d)));
        
    }
    file.close();
    return dataList;
}

void addData(string filename, material newMat) {
    ofstream file(filename, ios::app); 
    if (file.is_open()) {
        file << newMat.index << "," << newMat.name << "," << newMat.yield << "," << newMat.density << endl;
        file.close();
    }
}

int main() {
   
    string path = "Data/Processed/materials.csv"; 
    vector<material> allMat = loadData(path);
    
    int x;
    cout << "1. Choose a material \n2. Add a material\nSelection: ";
    cin >> x;

    if (x == 1) {
        
        if (allMat.empty()) {
            cout << "No data found or file error." << endl;
            return 0;
        }

        int y;
        cout << "Enter list index (0 to " << allMat.size()-1 << "): ";
        cin >> y;

        cout << allMat[y].name << " | " << allMat[y].yield << " | " << allMat[y].density << endl;
       
    } 
    else if (x == 2) {
        int i; 
        string n; 
        float y, d;
        cout << "Name: "; cin >> n;
        cout << "Yield: "; cin >> y;
        cout << "Density: "; cin >> d;

        material newMat(allMat.size(), n, y, d);
        addData(path, newMat);
        cout << "Material added!" << endl;
    }

    return 0;
}