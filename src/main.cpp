#include <iostream>
#include "archive/Archiver.hpp"



int main(int argc, char *argv[]){
    for (int i = 0; i < argc; i++){
        std::cout<< argv[i] <<std::endl;
    }
    try{

        // for (auto const& dir_entry : std::filesystem::directory_iterator("test")){
        //     std::string dir_source = dir_entry.path().string();
        //     std::cout<< dir_source <<std::endl;
        // }
        // remove("test.zip");
        archive::Archiver zip_compressor("test.zip");
        zip_compressor.addFiles({{"new_file.txt", "new_file.txt"}});
        // zip_compressor.addFiles({{"test.txt", "test.txt"}});
        // zip_compressor.addDirs({{"test", "test"}});
        // zip_compressor.deleteFile("test.txt");
        if(!zip_compressor.save()){
            std::cout<< "Error: " << zip_compressor.getErrorMessage() << std::endl;
        }
        for (auto &&warning : zip_compressor.getWarningsMessage()){
            std::cout<< warning << std::endl;
        }

    }catch(const std::string e){
        std::cerr << e << '\n';
    }catch(const char* e){
        std::cerr << e << '\n';
    }catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    return 0;
}
