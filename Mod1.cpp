#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;


// Class to handle CSV files operations
class csvHandler 
{
public:      
    string fileName;
    csvHandler(string filename)
    {
        fileName = filename;
    }
    // Function to read from CSV file
    fstream readCSV() {
        fstream file(fileName);
        if (!file.is_open()) {
            cout << "Could not open the file!" << endl;
        }
        cout << "File opened successfully!" << endl;
        return file;
    }
    void writeCSV(string dataLine="") {
        fstream file(fileName,ios::app);
        if (!file.is_open()) {
            cout << "Could not open the file!" << endl;
        }
        file << "\n" + dataLine << endl;
        cout << "Data written successfully!" << endl;
        file.close();
    }
};


//Class to store material properties
class Material
{
public:
    int index;
    string name;
    float yield;
    float density;
    vector<Material> materials;

    
    Material(string n = "", string y = "0.0", string d = "0.0") {
        name = n;
        yield = stof(y);
        density = stof(d);
    }

    // Function to read materials from CSV file and return a vector of Material objects
    vector<Material> materialList()
    {
        csvHandler handler("./Data/Processed/materials.csv");
        fstream file = handler.readCSV();
        string dataLine;
        getline(file, dataLine); // Skip header line
        while (getline(file, dataLine)) {
            stringstream ss(dataLine);
            string index, n, y, d;
            getline(ss, index, ',');  // Read index but not needed
            getline(ss, n, ',');
            getline(ss, y, ',');
            getline(ss, d, ',');
            materials.push_back(Material(n, y, d));
        }
        file.close();
        return materials;
    };

    // add Material to the CSV file/Vector
    void addMaterial(Material material)
    {
        csvHandler handler("./Data/Processed/materials.csv");
        string dataLine = to_string(materialList().size()) + "," + material.name + "," + to_string(material.yield) + "," + to_string(material.density);
        handler.writeCSV(dataLine);
        materials.push_back(material);
    }

};


// demonstrate the usage of Material class and CSV handling
void doc()
{
    // vector<Material> materials= Material().materialList();  // Get the list of materials from the CSV file

    //how to get each material's properties

    // for (Material material : materials) {
    //     cout << "Index: " << material.index << ", Name: " << material.name 
    //          << ", Yield: " << material.yield << ", Density: " << material.density << endl;
    // }

    // add a new material to the CSV file and the materials vector

    //Material().addMaterial(Material("dgdg", "454.0", "55.0"));


    // ger the value of specific material property

    //cout<<Material().materialList()[0].name<<endl;
}


int main()
{
    doc();
    return 0;

}