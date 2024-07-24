#include "archive/Archiver.hpp"
#include "Archiver.hpp"

std::string archive::Archiver::getErrorMessage(){
    // error_message_.append(": ").append(zip_error_strerror(&error_));
    return error_message_;
}

void archive::Archiver::addDirs(std::map<std::string, std::string> name_dirs){
    for (auto &&dir : name_dirs){
        addDir(dir.first, dir.second);
    }
}

// void addTextInFiel(std::string name_file, std::string text){
//     source_ = zip_source_buffer(archive_, text.c_str(), text.size(), 0);
//     if(source_  == nullptr){
//         // throw
//         error_message_ = "Error adding file. ";
//         error_message_.append(zip_strerror(archive_)).append(" ").append(name_file);
//     }

//         if(zip_file_add(archive_, name_file.c_str(), source_, ZIP_FL_ENC_UTF_8) < 0){
//             // throw
//             "Can't file to zip archive";
//         }
//         zip_source_keep(source_);
// }

void archive::Archiver::addFiles(std::map<std::string, std::string> name_files){
    for (auto &&file_name : name_files){
        addFile(file_name.first, file_name.second);
    }
}

void archive::Archiver::addDir(std::string dir_source, std::string dir_destination){
    // add_dir_name_list_[dir_source] = dir_destination;
    add_dir_name_list_.insert(std::pair<std::string, std::string>(dir_source, dir_destination));
}

void archive::Archiver::addFile(std::string file_source, std::string file_destination){
    add_file_name_list_.insert(std::pair<std::string, std::string>(file_source, file_destination));
}

void archive::Archiver::cancel(){
    add_file_name_list_.clear();
    add_dir_name_list_.clear();
    archive_file_name_.clear();
    add_delete_file_list_.clear();
}

void archive::Archiver::save()
{
    int err;
    zip_error_t error;
    // zip_error_init(&error_);
    if(flag_ == 0){
        if(std::filesystem::exists(archive_file_name_)){
            flag_ = ZIP_CHECKCONS;
        }else{
            flag_ = ZIP_CREATE;
        }
    }
    zip_t *archive = zip_open(archive_file_name_.c_str(), flag_, &err);

    if (archive == nullptr) {
        error_message_ = "Cannot open zip archive";
        // error_message_.append(zip_strerror(archive_));
        zip_error_init_with_code(&error, err);
        zip_error_fini(&error);
    }

    if (!add_dir_name_list_.empty()){
        for (auto &&dir : add_dir_name_list_){
            putDirs(archive, dir.first, dir.second);
        }
    }

    if (!add_file_name_list_.empty()){
        for (auto &&name_file : add_file_name_list_){
            putFiels(archive, name_file.first, name_file.second);
        }
    }
    int rez = zip_close(archive);
    if (rez != 0) {
        zip_discard(archive);
    }
    // zip_source_close(source);
}

void archive::Archiver::putDirs(zip_t *archive, std::string path_source, std::string path_destination){
    if(std::filesystem::exists(path_source)){
        if(std::filesystem::is_directory(path_source)){
            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(path_source)){
                std::string dir_source = dir_entry.path().string();

                std::cout<< dir_source <<std::endl;

                if(std::filesystem::is_directory(dir_source)){
                    zip_add_dir(archive, path_source.c_str());
                }else{
                    putFiels(archive, dir_source, dir_source);
                }
            }
        }else if(std::filesystem::is_regular_file(path_source)){
            putFiels(archive, path_source, path_destination);
        }
    }
}

void archive::Archiver::putFiels(zip_t *archive, std::string file_sours, std::string file_destination){
    zip_source_t* source = zip_source_file(archive, file_sours.c_str(), 0, -1);

    if (source == nullptr){
        // throw MsgException("Can't open zip from source: %s.");
        throw "Can't open zip from source";
    }

    if(zip_file_add(archive, file_destination.c_str(), source, ZIP_FL_ENC_UTF_8) < 0){
        zip_source_free(source);
        throw "Can't add file to zip archive " + file_sours;
    }
    zip_source_close(source);
}

void archive::Archiver::deleteFiles(std::vector<std::string> file_list){
    for (auto &&file_name : file_list){
        deleteFile(file_name);
    }
}

void archive::Archiver::deleteFile(std::string file_name){
    // zip_source_free(source_);
    if(std::filesystem::exists(file_name)){
        add_delete_file_list_.push_back(file_name);
    }else{
        // throw "";
    }
}

zip_int64_t archive::Archiver::findFileInZipT(std::string file_name, zip_t *archive){
    return zip_name_locate(archive, file_name.c_str(), ZIP_FL_ENC_UTF_8);
}
