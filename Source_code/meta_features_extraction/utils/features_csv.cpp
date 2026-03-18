#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <filesystem>

#include "features_csv.h"
#include "../headers/globals.h"

using namespace std;

void build_csv(const vector<double> &mo_features, const vector<string> &column_names, 
               const string &rootfolder, const string &instance, const string &folder, 
               const string &subfolder, const string &subsubfolder, const string &filename) {
    // rootfolder / instance / folder / subfolder / subsubfolder / filename
    // subfolder may already contain internal slashes
    string separator = "/"; 
    string aux = "";
    string full_path = rootfolder;
    if(!instance.empty()) full_path += separator + instance;
    if(!folder.empty()) full_path += separator + folder;
    if(!subfolder.empty()) full_path += separator + subfolder; // ||may already contain nested levels
    if(!subsubfolder.empty()) full_path += separator + subsubfolder;

    string file_path = aux + full_path + separator + filename;
    filesystem::create_directories(full_path);
    cout << "Creating file: " << file_path << endl;
    ofstream file(file_path);
    if (file.is_open()) {
        vector<string> updated_column_names = column_names;
        updated_column_names.insert(updated_column_names.begin(), "Instance");
        for (size_t i = 0; i < updated_column_names.size(); ++i) {
            file << updated_column_names[i];
            if (i != updated_column_names.size() - 1) file << ",";
        }
        file << "\n";
        file << fixed << setprecision(10);
        // ||Write original instance path if available
        if(!original_instance_path.empty()) file << original_instance_path; else file << instance;
        for (size_t i = 0; i < mo_features.size(); ++i) {
            //double value = isnan(mo_features[i]) ? 0.0 : mo_features[i];
            double value = mo_features[i];
            file << "," << value;
        }
        file << "\n";
        file.close();
    } else {
        cerr << "File path Error: " << file_path << endl;
    }
}
