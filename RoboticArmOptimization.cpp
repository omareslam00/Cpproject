#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

string dimPrecision(double value, string unit); // Function to convert dimensions to appropriate units based on their magnitude for better readability

// Class to handle CSV files operations

string stringInputValidation(string message)
{
    string input = "";
    cout << message;
    getline(cin,input);
    if (!input.empty())
    {
        return input;
    }
    else
    {
        cout << "Invalid input. \n";
        return stringInputValidation(message);
    }
}
double numberInputValidation(string message)
{
    string input = "";
    double number;
    cout << message;
    cin >> input;
    try
    {
        number = stod(input); // Try to convert input to a double
        cin.ignore();
        return number;
    }
    catch (exception e) // Catch any conversion error
    {
        cout << "Invalid input. \n";
        return numberInputValidation(message);
    }
}

class csvHandler
{
public:
    string fileName;
    csvHandler(string filename)
    {
        fileName = filename;
    }
    // Function to read from CSV file
    fstream readCSV()
    {
        fstream file(fileName);
        if (!file.is_open())
        {
            cout << "Could not open the file!" << endl;
        }
        cout << "File opened successfully!" << endl;
        return file;
    }
    void writeCSV(string dataLine)
    {
        fstream file(fileName, ios::app);
        if (!file.is_open())
        {
            cout << "Could not open the file!" << endl;
        }
        file << dataLine << endl;
        cout << "Data written successfully!" << endl;
        file.close();
    }
};

// Class to store material properties
class Material
{
public:
    string name;
    double yield;   // units in MPa
    double density; // units in g/cm^3
    vector<Material> materials;

    Material(string n = "", double y = 0.0, double d = 0.0)
    {
        name = n;
        yield = y;
        density = d;
    }

    // Function to read materials from CSV file and return a vector of Material objects
    vector<Material> materialList()
    {
        csvHandler handler("./Data/Processed/materials.csv");
        fstream file = handler.readCSV();
        string dataLine;
        getline(file, dataLine); // Skip header line
        while (getline(file, dataLine))
        {
            stringstream ss(dataLine);
            string index, n, y, d;
            getline(ss, index, ','); // Read index but not needed
            getline(ss, n, ',');
            getline(ss, y, ',');
            getline(ss, d, ',');
            materials.push_back(Material(n, stod(y), stod(d)));
        }
        file.close();
        return materials;
    }

    // add Material to the CSV file/Vector
    Material addMaterial()
    {
        string n;
        double y, d;
        n = stringInputValidation("Enter material name: ");
        y = numberInputValidation("Enter yield (MPa): ");
        d = numberInputValidation("Enter density (g/cm^3): ");

        csvHandler handler("./Data/Processed/materials.csv");
        string dataLine = to_string(materialList().size()) + "," + n + "," + to_string(y) + "," + to_string(d);
        handler.writeCSV(dataLine);
        materials.push_back(Material(n, y, d));
        cout << materials.back().name << " added to the materials list." << endl;
        return materials.back();
    }

    // display materials and get user selection
    Material getMaterial()
    {
        materials = materialList(); // Load materials from CSV file
        cout << "Available Materials:" << endl;
        cout << "Index | Name | Yield(MPa) | Density(g/cm^3)" << endl;
        for (int i = 0; i < materials.size(); i++)
        {
            cout << i << " | " << materials[i].name << " | " << materials[i].yield << " | " << materials[i].density << endl;
        }
        string i; // store index as string to be converted to int after validation
        cout << "enter index of the required material:  ";

        // Loop until a valid index is entered
        while (true)
        {
            try
            {
                cin >> i;
                int index = stoi(i); // Convert string input to integer
                if (index < 0 || index >= materials.size())
                {
                    throw out_of_range("Index out of range"); // Check if index is within valid range
                }
                cout << "Material selected successfully!" << endl;
                cout << "Selected Material: " << materials[index].name << endl;
                return materials[index];
            }
            catch (exception e) // catch any error (invalid input or out of range)
            {
                cout << "Invalid input. \nPlease enter a valid index: ";
            }
        }
    }
    Material controlMaterial()
    {
        int index = numberInputValidation("1-Add new material \n2-Select material from the list \nEnter your choice: ");
        switch (index)
        {
        case 1:
            return addMaterial();
        case 2:
            return getMaterial();
        default:
            cout << "Enter a valid number\n";
            return controlMaterial();
        }
    }
};

class StressAnalysis
{
public:
    const double g = 9.81;   // Acceleration due to gravity (m/s^2)
    int crossSectionType; // 1-circular or 2-rectangular
    double linkLength;       // Length of the link (m)
    double payloadMass;      // Mass of the payload (kg)
    double angAcc;           // Angular acceleration (rad/s^2)
    double r;                // radius for circular cross section (m)
    double b, h;             // width and height for rectangular cross section (m)
    double linkMass;         // Mass of the link (kg)
    double stress;           // Stress on the link (Pa)
    double tRequired;

    Material material = Material().controlMaterial(); // Get material properties from user selection/addition
    double density = material.density * 1000;         // Convert density from g/cm^3 to kg/m^3

    void calculateCircular()
    {
        linkMass = density * M_PI * pow(r, 2) * linkLength;     // Mass of the link (kg)
        double bendingMoment = linkMass * g * (linkLength / 2) + payloadMass * g * linkLength + linkMass * pow(linkLength / 2, 2) * angAcc + payloadMass * pow(linkLength, 2) * angAcc; // Momentum of the link (N.m)
        double I = M_PI * pow(r, 4) / 4;    // Moment of inertia for circular cross section (m^4)
        stress = (bendingMoment * r / I);   // Maximum stress formula
    }

    void calculateRectangular()
    {
        linkMass = density * b * h * linkLength;    // Mass of the link (kg)
        double bendingMoment = linkMass * g * (linkLength / 2) + payloadMass * g * linkLength + linkMass * pow(linkLength / 2, 2) * angAcc + payloadMass * pow(linkLength, 2) * angAcc; // Momentum of the link (N.m)
        double I = (b * pow(h, 3)) / 12;    // Moment of inertia for rectangular cross section (m^4)
        stress = (bendingMoment * h / (2 * I)); // Maximum stress formula
    }

    void controlStress()
    {
        crossSectionType = numberInputValidation("Enter cross section type (1-circular, 2-rectangular): ");
        switch (crossSectionType)
        {
        case 1:
        {
            r = numberInputValidation("Enter radius(m): ");
            linkLength = numberInputValidation("Enter link length(m): ");
            payloadMass = numberInputValidation("Enter payload mass(kg): ");
            angAcc = numberInputValidation("Enter angular acceleration(rad/s^2): ");

            calculateCircular();
        }
        break;

        case 2:
        {
            b = numberInputValidation("Enter width(m): ");
            h = numberInputValidation("Enter height(m): ");
            linkLength = numberInputValidation("Enter link length(m): ");
            payloadMass = numberInputValidation("Enter payload mass(kg): ");
            angAcc = numberInputValidation("Enter angular acceleration(rad/s^2): ");

            calculateRectangular();
        }
        break;

        default:
            cout << "Enter Valid type" << endl;
            controlStress();
        }
    }

    void optimizeDims()
    {
        bool optimized = false;
        double yield = material.yield * pow(10, 6); // Convert yield from MPa to Pa for comparison with stress in Pa
        switch (crossSectionType)
        {
        case 1:
            while (!optimized)
            {
                calculateCircular();
                if (stress > yield)
                {
                    r *= 1.01;
                }
                else if (stress < yield)
                {
                    r *= 0.99;
                }
                if (stress >= yield * 0.99 && stress <= yield)
                {
                    optimized = true;
                    cout << "Optimized radius: " << dimPrecision(r, "m") << endl;
                    cout << "Optimized stress: " << dimPrecision(stress, "Pa") << endl;
                    cout << "link mass: " << dimPrecision(linkMass, "kg") << endl;
                    tRequired = (linkMass * g * (linkLength / 2) + (payloadMass * g * linkLength) + (linkMass * pow((linkLength / 2), 2) * angAcc + payloadMass * pow(linkLength, 2) * angAcc))*1000; // Calculate required torque in mNm based on the stress analysis results
                    cout<<"Required Torque is: "<<dimPrecision(tRequired/1000, "Nm")<<endl;
                }
            }
            break;

        case 2:
            while (!optimized)
            {
                calculateRectangular();
                if (stress > yield)
                {
                    b *= 1.01;
                    h *= 1.01;
                }
                else if (stress < yield)
                {
                    b *= 0.99;
                    h *= 0.99;
                }
                if (stress >= yield * 0.99 && stress <= yield)
                {
                    optimized = true;
                    cout << "Optimized dimensions: width = " << dimPrecision(b, "m") << ", height = " << dimPrecision(h, "m") << endl;
                    cout << "Optimized stress: " << dimPrecision(stress, "Pa") << endl;
                    cout << "Optimized link mass: " << dimPrecision(linkMass, "kg") << endl;
                    tRequired = (linkMass * g * (linkLength / 2) + (payloadMass * g * linkLength) + (linkMass * pow((linkLength / 2), 2) * angAcc + payloadMass * pow(linkLength, 2) * angAcc))*1000; // Calculate required torque in mNm based on the stress analysis results
                    cout<<"Required Torque is: "<<dimPrecision(tRequired/1000, "Nm")<<endl;
                }
            }
            break;
        }
    }
};

// Class to store motors properties
class Motor
{
public:
    string url;     // URL to the motor product page
    double diameter; // units in mm
    double width;    // units in mm
    double speed;    // units in rpm
    double torque;   // units in mNm
    double weight;   // units in g
    vector<Motor> motors;

    Motor(string u="https://example.com/" ,double d = 0.0, double w = 0.0, double s = 0.0, double t = 0.0, double m = 0.0)
    {
        url = u;
        diameter = d;
        width = w;
        speed = s;
        torque = t;
        weight = m;
    }

    // Function to read motors from CSV file and return a vector of Motor objects
    vector<Motor> motorsList()
    {
        csvHandler handler("./Data/Processed/motors.csv");
        fstream file = handler.readCSV();
        string dataLine;
        getline(file, dataLine); // Skip header line
        while (getline(file, dataLine))
        {
            stringstream ss(dataLine);
            string index, url, d, w, s, t, m;
            getline(ss, index, ','); // Read index but not needed
            getline(ss, url, ',');
            getline(ss, d, ',');
            getline(ss, w, ',');
            getline(ss, s, ',');
            getline(ss, t, ',');
            getline(ss, m, ',');
            motors.push_back(Motor(url, stod(d), stod(w), stod(s), stod(t), stod(m)));
        }
        file.close();
        return motors;
    };

    // add Motor to the CSV file/Vector
    Motor addMotor()
    {
        string u;
        double d, w, s, t, m;

        u = stringInputValidation("Enter motor URL: ");
        d = numberInputValidation("Enter motor diameter (mm): ");
        w = numberInputValidation("Enter motor width (mm): ");
        s = numberInputValidation("Enter motor speed (rpm): ");
        t = numberInputValidation("Enter motor torque (mNm): ");
        m = numberInputValidation("Enter motor weight (g): ");

        cout << "Motor added successfully!" << endl;

        csvHandler handler("./Data/Processed/motors.csv");
        string dataLine = to_string(motors.size()) + "," + u + "," + to_string(d) + "," + to_string(w) + "," + to_string(s) + "," + to_string(t) + "," + to_string(m);
        handler.writeCSV(dataLine);
        motors.push_back(Motor(u, d, w, s, t, m));
        cout << motors.back().diameter << " added to the motors list." << endl;
        return motors.back();
    }
};

// Class to store gearboxes properties
class Gearbox
{
public:
    string url;       // URL to the gearbox product page
    double reductionRatio;
    double efficiency; // units in %
    double width;      // units in mm
    double weight;     // units in g
    double diameter;   // units in mm
    vector<Gearbox> gearboxes;

    Gearbox(string u="https://example.com/", string r = "0.0", double e = 0.0, double w = 0.0, double m = 0.0, double d = 0.0)
    {
        url = u;
        // handle \ in reduction ratio if it exists (example: 1/100)
        if (r.find('/'))
        {
            double numerator = stod(r.substr(0, r.find('/')));
            double denominator = stod(r.substr(r.find('/') + 1));
            reductionRatio = numerator / denominator;
        }
        else
        {
            reductionRatio = 0; // default value if reduction ratio is not provided or invalid
        }
        efficiency = e;
        width = w;
        weight = m;
        diameter = d;
    }

    // Function to read gearboxes from CSV file and return a vector of Gearbox objects
    vector<Gearbox> gearboxesList()
    {
        csvHandler handler("./Data/Processed/gears.csv");
        fstream file = handler.readCSV();
        string dataLine;
        getline(file, dataLine); // Skip header line
        while (getline(file, dataLine))
        {
            stringstream ss(dataLine);
            string index, url, r, e, w, m, d;
            getline(ss, index, ','); // Read index but not needed
            getline(ss, url, ',');
            getline(ss, r, ',');
            getline(ss, e, ',');
            getline(ss, w, ',');
            getline(ss, m, ',');
            getline(ss, d, ',');
            gearboxes.push_back(Gearbox(url, r, stod(e), stod(w), stod(m), stod(d)));
        }
        file.close();
        return gearboxes;
    }

    // add Gearbox to the CSV file/Vector
    Gearbox addGearbox()
    {
        string u, r;
        double e, w, m, d;

        u = stringInputValidation("Enter gearbox URL: ");
        r = stringInputValidation("Enter gearbox reduction ratio (example: 1/100): ");
        e = numberInputValidation("Enter gearbox efficiency (%): ");
        w = numberInputValidation("Enter gearbox width (mm): ");
        m = numberInputValidation("Enter gearbox weight (g): ");
        d = numberInputValidation("Enter gearbox diameter (mm): ");

        csvHandler handler("./Data/Processed/gearboxes.csv");
        string dataLine = to_string(gearboxes.size()) + "," + u + "," + r + "," + to_string(e) + "," + to_string(w) + "," + to_string(m) + "," + to_string(d);
        handler.writeCSV(dataLine);
        gearboxes.push_back(Gearbox(u, r, e, w, m, d));
        cout << gearboxes.back().reductionRatio << " added to the gearboxes list." << endl;
        return gearboxes.back();
    }
};

// class to calculate the output torque,output speed for all possible motor-gearbox combinations
class MotorGearboxPair
{
public:
    int motorIndex;
    int gearboxIndex;
    double tOutput; // units in mNm
    double angVel;  // units in rpm
    MotorGearboxPair(int mI = 0, int gI = 0, double tO = 0.0, double aV = 0.0)
    {
        motorIndex = mI;
        gearboxIndex = gI;
        tOutput = tO;
        angVel = aV;
    }

    void motorGearboxCombine()
    {
        csvHandler handler("./Data/Processed/motor_gearbox_pairs.csv");
        vector<Motor> motors = Motor().motorsList();
        vector<Gearbox> gearboxes = Gearbox().gearboxesList();
        for (int i = 0; i < motors.size(); i++)
        {
            for (int j = 0; j < gearboxes.size(); j++)
            {
                tOutput = motors[i].torque * gearboxes[j].reductionRatio * (gearboxes[j].efficiency / 100); // Calculate output torque in mNm
                angVel = motors[i].speed / gearboxes[j].reductionRatio;                                     // Calculate output speed in rpm
                cout << "Motor Index: " << i << ", Gearbox Index: " << j << ", Output Torque: " << tOutput << " mNm, Output Speed: " << angVel << " rpm" << endl;
                string dataLine = to_string(i) + "," + to_string(j) + "," + to_string(tOutput) + "," + to_string(angVel);
                handler.writeCSV(dataLine);
            }
        }
    }

    vector<MotorGearboxPair> motorGearboxList()
    {
        csvHandler handler("./Data/Processed/motor_gearbox_pairs.csv");
        fstream file = handler.readCSV();
        vector<MotorGearboxPair> pairs;
        string dataLine;
        getline(file, dataLine); // Skip header line
        while (getline(file, dataLine))
        {
            stringstream ss(dataLine);
            string mI, gI, tO, aV;
            getline(ss, mI, ',');
            getline(ss, gI, ',');
            getline(ss, tO, ',');
            getline(ss, aV, ',');
            pairs.push_back(MotorGearboxPair(stoi(mI), stoi(gI), abs(stod(tO)), stod(aV)));
        }
        return pairs;
    }
};

// Class to calculate Trequired, Toutput, Output speed and cost function
class MotorGearboxOptimization : public StressAnalysis
{
public:
    vector<MotorGearboxPair> pairs = MotorGearboxPair().motorGearboxList();
    vector<Motor> motors = Motor().motorsList();
    vector<Gearbox> gearboxes = Gearbox().gearboxesList();
    // sort pairs based on output torque in descending order
    void sortPairs(int criteria)
    {
        sort(pairs.begin(), pairs.end(), [criteria](const MotorGearboxPair &a, const MotorGearboxPair &b)
             {
                    switch (criteria)
                    {
                    case 1: // Sort by output torque
                        return a.tOutput > b.tOutput; // Descending order
                    case 2: // Sort by output speed
                        return a.angVel > b.angVel; // Descending order
                    default:
                        return false; // No sorting if criteria is invalid
                    } });
    }

    // function to elimenate pairs that do not meet the required torque
    int filterPairs(int criteria)
    {
        switch (criteria)
        {
        case 1: // Filter by output torque
        {
            while(pairs.back().tOutput < tRequired) // Remove pairs that do not meet the required torque
            {
            pairs.pop_back();
                if(pairs.empty())
                {
                    cout<<"No motor-gearbox pair meets the required torque. Please optimize the link dimensions or select a different material."<<endl;
                    return 0;
                }
            }
            return 1;
            break;
        }
        case 2: // Filter by output speed
        {
        double angVelRequired = numberInputValidation("Enter required output speed (rpm): ");
        while(pairs.back().angVel < angVelRequired) // Remove pairs that do not meet the required output speed
            {
                pairs.pop_back();
                if(pairs.empty())
                {
                    cout<<"No motor-gearbox pair meets the required output speed. Please optimize the link dimensions or select a different material."<<endl;
                    return 0;
                }
            }
            return 1;
            break;
        }
        }
        return 0;
    }

    // function to calculate cost function for each pair and sort pairs based on cost function in ascending order
    MotorGearboxPair costFunction()
    {
        string criteria;
        cout << "Enter Cost function criteria (1-mass + dimensions 2- mass only): ";
        try
        {
            cin >> criteria;
            stoi(criteria); // Validate if input is an integer
        }
        catch (exception e)
        {
            cout << "Invalid input. \nPlease enter a valid number: ";
            return costFunction();
        }
        vector<pair<double, MotorGearboxPair>> costPairs; // Vector to store cost and corresponding pair

        switch (stoi(criteria))
        {
        case 1:
            for (int i = pairs.size()-1; i >= 0; i--)
            {
                Motor motor = motors[pairs[i].motorIndex];
                Gearbox gearbox = gearboxes[pairs[i].gearboxIndex];
                double cost = motor.weight + gearbox.weight + (motor.diameter + motor.width + gearbox.width + gearbox.diameter) / 100; // Example cost function based on weight and dimensions
                costPairs.push_back(make_pair(cost, pairs[i]));                                                                        // Add cost and pair to the vector
                // Sort costPairs based on cost in ascending order
            }
            sort(costPairs.begin(), costPairs.end(), [](const pair<double, MotorGearboxPair> &a, const pair<double, MotorGearboxPair> &b)
                    {
                        return a.first < b.first; // Ascending order
                    });
            return costPairs[0].second; // Return the pair with the lowest cost
        case 2:
            for (int i = pairs.size()-1; i >= 0; i--)
            {
                Motor motor = motors[pairs[i].motorIndex];
                Gearbox gearbox = gearboxes[pairs[i].gearboxIndex];
                double cost = motor.weight + gearbox.weight;    // Example cost function based on weight and dimensions
                costPairs.push_back(make_pair(cost, pairs[i])); // Add cost and pair to the vector
                // Sort costPairs based on cost in ascending order
            }
            sort(costPairs.begin(), costPairs.end(), [](const pair<double, MotorGearboxPair> &a, const pair<double, MotorGearboxPair> &b)
                    {
                        return a.first < b.first; // Ascending order
                    });
            return costPairs[0].second; // Return the pair with the lowest cost
        default:
            cout << "Enter a valid number \n";
            return costFunction();
        }
    }
};

////Documentation
// vector<Material> materials= Material().materialList();  //// Get the list of materials from the CSV file

////how to get each material's properties

// for(int i=0;i<materials.size();i++){
//         cout << i << " | " << materials[i].name << " | " << materials[i].yield <<" | "<< materials[i].density << endl;
//     }

//// add a new material to the CSV file and the materials vector

// Material().addMaterial();

//// get the value of specific material property

// cout<<Material().materialList()[0].name<<endl;

//// get user selection of material and display its properties
// Material material = Material().getMaterial();
// cout << "Selected Material: " << material.name << ", Yield: " << material.yield << ", Density: " << material.density << endl;

//// get user select/add material and display its properties
// Material material = Material().controlMaterial();

//// get stress and find if it is safe or not
// double stress = StressAnalysis().controlStress();

int main()
{
    MotorGearboxOptimization optimization;
    optimization.controlStress(); // Get the required torque based on the stress analysis results
    optimization.optimizeDims(); // Optimize the dimensions of the link to meet the required torque
    optimization.sortPairs(1);   // Sort pairs based on output torque
    if (!optimization.filterPairs(1))  // Filter pairs based on output torque, terminate if no pair meets the required torque
    {
        return 0;
    }
    optimization.sortPairs(2);   // Sort pairs based on output speed, terminate if no pair meets the required output speed
    if (!optimization.filterPairs(2))
    {
        return 0;
    }
    MotorGearboxPair bestPair = optimization.costFunction(); // Get the best motor-gearbox pair based on the cost function
    cout << "Best Motor-Gearbox Pair:\nMotor Index: " << bestPair.motorIndex << ", Motor link: " << optimization.motors[bestPair.motorIndex].url << "\nGearbox Index: " << bestPair.gearboxIndex << ", Gearbox link: " << optimization.gearboxes[bestPair.gearboxIndex].url << "\nOutput Torque: " << dimPrecision(bestPair.tOutput/1000, "Nm") << ", Output Speed: " << bestPair.angVel << "rpm" << endl;
    return 0;
}

string dimPrecision(double value, string unit)
{
    int orderOfMagnitude = floor(log10(abs(value)));
    if(orderOfMagnitude >= 3 || orderOfMagnitude <= -2){
    return to_string(value * pow(10, -orderOfMagnitude)) + " x 10^" + to_string(orderOfMagnitude) + " " + unit;
    }
    else{
        return to_string(value) + " " + unit;
    }
}