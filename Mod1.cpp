#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
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
    void writeCSV(string dataLine) {
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

    
    Material(string n="", string y="0.0", string d="0.0") {
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
    Material addMaterial()
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
                cout << "Material added successfully!" << endl;
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
        cout << materials.back().name << " added to the materials list." << endl;
        return materials.back();
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
                cout << "Material selected successfully!" << endl;
                cout << "Selected Material: " << materials[index].name << endl;
                return materials[index];
            }
            catch(exception e)  //catch any error (invalid input or out of range)
            {
                cout << "Invalid input. \nPlease enter a valid index: " ;
            }
            
        }

    }
    Material controlMaterial()
    {
        string index;
        cout<<"1-Add new material \n2-Select material from the list \nEnter your choice: ";
        try
        {
            cin>>index;
            stoi(index); // Validate if input is an integer
        }
        catch(exception e)
        {
            cout << "Invalid input. \nPlease enter a valid number: " ;
            return controlMaterial();
        }
        switch (stoi(index))
        {
        case 1:
            return addMaterial();
        case 2:
            return getMaterial();
        default:
            cout<<"Enter a valid number \n";
            return controlMaterial();
        }
    }

};

class StressAnalysis
{
public:
const float g = 9.81; // Acceleration due to gravity

string crossSectionType; // 1-circular or 2-rectangular
float linkLength,payloadMass,angAcc,r,b,h;
Material material = Material().controlMaterial(); // Get material properties from user selection/addition
float density = material.density*1000; // Convert density from g/cm^3 to kg/m^3
float calculateCirclar()
{
    float linkMass = density * M_PI * pow(r,2) * linkLength; // Mass of the link
    float bendingMoment = linkMass * g * (linkLength/2) + payloadMass * g * linkLength + linkMass * pow(linkLength/2,2) * angAcc + payloadMass * pow(linkLength,2) * angAcc; // Momentum of the link
    float I = M_PI*pow(r,4)/4; // Moment of inertia for circular cross section
    float stress = (bendingMoment * r / I)/pow(10,6); // Maximum stress formula converted to MPa
    cout << "Calculated Bending Stress for Circular Cross Section: " << stress << " MPa" << endl;
    if(stress > material.yield)
    {
        cout << "The link will fail under the given conditions." << endl;
    }
    else
    {
        cout << "The link is safe under the given conditions." << endl;
    }
    return stress;
}

float calculateRectangular()
{
    float linkMass = density * b * h * linkLength; // Mass of the link
    float bendingMoment = linkMass * g * (linkLength/2) + payloadMass * g * linkLength + linkMass * pow(linkLength/2,2) * angAcc + payloadMass * pow(linkLength,2) * angAcc; // Momentum of the link
    float I = (b * pow(h,3)) / 12; // Moment of inertia for rectangular cross section
    float stress = (bendingMoment * h / (2 * I))/pow(10,6); // Maximum stress formula converted to MPa
    cout << "Calculated Bending Stress for rectangular Cross Section: " << stress << " MPa" << endl;
    if(stress > material.yield)
    {
        cout << "The link will fail under the given conditions." << endl;
    }
    else
    {
        cout << "The link is safe under the given conditions." << endl;
    }
    return stress;
}

float controlStress(){
    cout<<"Enter cross section type (1-circular 2-rectangular): ";
    try
    {
        cin>> crossSectionType;
        stoi(crossSectionType); // Validate if input is an integer
    }
    catch(exception e)
    {
        cout << "Invalid input. \nPlease enter a valid number: " ;
        return controlStress();
    }
    switch (stoi(crossSectionType))
    {    
        case 1:
        {
            cout<<"Enter radius: ";
            cin>>r;

            cout<<"enter link length: ";
            cin>>linkLength;

            cout<<"enter payload mass: ";
            cin>>payloadMass;

            cout<<"enter angular acceleration: ";
            cin>>angAcc;

            return calculateCirclar();
        }

        case 2 :
        {
            cout<<"Enter width: ";
            cin>>b;

            cout<<"Enter height: ";
            cin>>h;

            cout<<"enter link length: ";
            cin>>linkLength;

            cout<<"enter payload mass: ";
            cin>>payloadMass;

            cout<<"enter angular acceleration: ";
            cin>>angAcc;

            return calculateRectangular();
        }

        default :
        cout<<"Enter Valid type"<<endl;
        return controlStress();
}
}};

// demonstrate the usage of Material class and CSV handling
void doc()
{
    // vector<Material> materials= Material().materialList();  //// Get the list of materials from the CSV file

    ////how to get each material's properties

    // for(int i=0;i<materials.size();i++){
    //         cout << i << " | " << materials[i].name << " | " << materials[i].yield <<" | "<< materials[i].density << endl;
    //     }

    //// add a new material to the CSV file and the materials vector

    //Material().addMaterial();


    //// get the value of specific material property

    //cout<<Material().materialList()[0].name<<endl;


    //// get user selection of material and display its properties
    // Material material = Material().getMaterial();
    // cout << "Selected Material: " << material.name << ", Yield: " << material.yield << ", Density: " << material.density << endl;

    //// get user select/add material and display its properties
    //Material material = Material().controlMaterial();

    //// get stress and find if it is safe or not
    // float stress = StressAnalysis().controlStress();

}

int main()
{
    float stress = StressAnalysis().controlStress();
    return 0;
}
