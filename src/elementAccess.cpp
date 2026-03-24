#include "../include/kvs.h"
#include <iostream>

int main(int argc, char* argv[]) {
    
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " <file.deck|file.toml|file.yaml|file.json>" << std::endl;
        return 1;    
    }

    KVS k;

    //Strip off the extension to get the base name
    std::string filename = argv[1];
    std::string base_name = filename.substr(0, filename.find_last_of('.'));
    std::string extension = filename.substr(filename.find_last_of('.'));

    if (extension == ".deck") {
        k = KVSConversion::fromEPOCHFile(filename);
    } else if (extension == ".toml") {
        k = KVSConversion::fromTOMLFile(filename);
    } else if (extension == ".yaml" || extension == ".yml") {
        k = KVSConversion::fromYAMLFile(filename);
    } else if (extension == ".json") {
        k = KVSConversion::fromJSONFile(filename);
    } else {
        std::cerr << "Unsupported file format: " << extension << std::endl;
        return 1;
    }

    std::cout << "Domain nx: " << k["domain"]["nx"].asInt() << std::endl;

    std::cout << "Y Max boundary condition: " << k["domain"]["boundaries"]["y_max"].asString() << std::endl;

    std::cout << "LARE3D resistivity: " << k["LARE3D"]["physics"]["resistivity"].asReal() << std::endl;

    try{
        std::cout << "Trying to get made up value: " << k["LARE3D"]["physics"]["made_up_value"] << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "Error getting made up value: " << e.what() << std::endl;
    }

    std::cout << "Adding made up value\n";
    k["LARE3D"]["physics"].upsert("made_up_value") = 42;
    try {
        std::cout << "Trying to get made up value again: " << k["LARE3D"]["physics"]["made_up_value"].asInt() << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "Error getting made up value again: " << e.what() << std::endl;
    }

}
