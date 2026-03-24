#include "../include/kvs.h"
#include <iostream>
#include <fstream>
#include <filesystem>

int main(){

    KVS k = KVS{
            KVS::Ordered{
                {
                    "runner",
                    KVS::Ordered{
                        {
                            "active",
                            KVS::Array{"LARE3D", "BrioAndWu"}
                        }
                    }
                },
                {
                    "domain",
                    KVS::Ordered{
                        {"nx", 100},
                        {"ny", 200},
                        {"nz", 300},
                        {"x_min", 0.0},
                        {"x_max", 1.0},
                        {"y_min", 0.0},
                        {"y_max", 1.0},
                        {"z_min", 0.0},
                        {"z_max", 1.0},
                        {"boundaries", KVS::Ordered{
                            {"x_min", "periodic"},
                            {"x_max", "periodic"},
                            {"y_min", "reflective"},
                            {"y_max", "reflective"},
                            {"z_min", "outflow"},
                            {"z_max", "outflow"}
                            }
                        }
                    }
                },
                {
                    "LARE3D", KVS::Ordered{
                        {"physics", KVS::Ordered{
                            {"resistivity", 1e-6},
                            {"viscosity", 1e-5},
                           }
                        },
                        {"domain", KVS::Ordered{
                            {"time_step", 0.01},
                            {"final_time", 100.0},
                            {"output_interval", 10.0}
                            }
                        }
                    }
                },
                {
                    "BrioAndWu", KVS::Ordered{
                        {"B0", 1.0},
                    }
                }
            }
        };

    KVSConversion::toEPOCHFile(k, "output/output.deck");
    KVSConversion::toJSONFile(k, "output/output.json",2);
    KVSConversion::toYAMLFile(k, "output/output.yaml");
    KVSConversion::toTOMLFile(k, "output/output.toml");

    std::cout << "Generated output files: output.deck, output.json, output.yaml, output.toml" << std::endl;

    return 0;
}
