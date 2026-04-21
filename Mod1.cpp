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
        file <<dataLine << endl;
        cout << "Data written successfully!" << endl;
        file.close();
    }
};


//Class to store material properties
class Material
{
public:
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
    void addMaterial()
    {
        string n, y, d;
        cout << "Enter material name | yield | density: ";
        while (true)
        {
            try
            {
                cin >> n >> y >> d;
                stof(y);
                stof(d);
                break; // Exit loop if conversion is successful
            }
            catch(exception e)
            {
                cout << "Invalid input. \nPlease enter valid yield and density values: " ;
            }
            
        }

        csvHandler handler("./Data/Processed/materials.csv");
        string dataLine = to_string(materialList().size()) + "," + n + "," + y + "," + d;
        handler.writeCSV(dataLine);
        materials.push_back(Material(n, y, d));
    }

    // display materials and get user selection
    Material getMaterial() {
        materials = materialList(); // Load materials from CSV file
        cout << "Available Materials:" << endl;
        cout << "Index | Name | Yield | Density" << endl;
        for(int i=0;i<materials.size();i++){
            cout << i << " | " << materials[i].name << " | " << materials[i].yield <<" | "<< materials[i].density << endl;
        }
        string i; // store index as string to be converted to int after validation
        cout << "enter index of the required material:  ";

        // Loop until a valid index is entered
        while (true)
        {
            try
            {
                cin>>i;
                int index = stoi(i); // Convert string input to integer
                if (index < 0 || index >= materials.size()) {
                    throw out_of_range("Index out of range");  // Check if index is within valid range
                }
                return materials[index];
            }
            catch(exception e)  //catch any error (invalid input or out of range)
            {
                cout << "Invalid input. \nPlease enter a valid index: " ;
            }
            
        }

    }

};


// demonstrate the usage of Material class and CSV handling
void doc()
{
    // vector<Material> materials= Material().materialList();  //// Get the list of materials from the CSV file

    ////how to get each material's properties

    // for (Material material : materials) {
    //     cout << "Index: " << material.index << ", Name: " << material.name 
    //          << ", Yield: " << material.yield << ", Density: " << material.density << endl;
    // }

    //// add a new material to the CSV file and the materials vector

    //Material().addMaterial();


    //// get the value of specific material property

    //cout<<Material().materialList()[0].name<<endl;


    //// get user selection of material and display its properties
    // Material material = Material().getMaterial();
    // cout << "Selected Material: " << material.name << ", Yield: " << material.yield << ", Density: " << material.density << endl;

}

int main()
{ 
    Material().addMaterial();
    return 0;
}