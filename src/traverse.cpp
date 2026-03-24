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

    int level = 0;

    k.traverse(
        [&level](const std::string& blockType, const KVS::Element& element){
            std::cout << std::string(level * 2, ' ') << "Entering block: " << blockType << std::endl;
            level++;
        },
        [&level](const std::string& blockType){
            level--;
            std::cout << std::string(level * 2, ' ') << "Exiting block: " << blockType << std::endl;
        },
        [&level](const std::string& key, const KVS::Element& element){
            std::cout << std::string(level * 2+1, ' ') << "Key: " << key << ", Value: " << element << std::endl;
        }
    );

}
