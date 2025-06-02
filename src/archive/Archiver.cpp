#include "archive/Archiver.hpp"

std::string archive::Archiver::getErrorMessage(){
    return error_message_;
}

std::vector<std::string> archive::Archiver::getWarningsMessage(){
    return warnings_;
}

void archive::Archiver::addDirs(std::map<std::string, std::string> name_dirs)
{
    for (auto &&dir : name_dirs){
        addDir(dir.first, dir.second);
    }
}

void archive::Archiver::addFiles(
    std::map<std::string, std::string> name_files
){
    for (auto &&file_name : name_files){
        addFile(file_name.first, file_name.second);
    }
}

void archive::Archiver::addDir(std::string dir_source, std::string dir_destination){
    if(!std::filesystem::exists(dir_source)){
        warnings_.push_back("Dir is not exists " + dir_source);
    }
    add_dir_name_list_.insert(std::pair<std::string, std::string>(dir_source, dir_destination));
}

void archive::Archiver::addFile(std::string file_source, std::string file_destination){
    if(std::filesystem::exists(file_source)){
        add_file_name_list_.insert(std::pair<std::string, std::string>(file_source, file_destination));
    }else{
        warnings_.push_back("File is not exists " + file_source);
    }
}

void archive::Archiver::cancel(){
    add_file_name_list_.clear();
    add_dir_name_list_.clear();
    archive_file_name_.clear();
    add_delete_file_list_.clear();
    warnings_.clear();
}

bool archive::Archiver::putDirsList(
    zip_t *archive,
    std::map<std::string, std::string> list_to_add
){
    bool state = true;
    if (!list_to_add.empty()){
        for (auto &&dir : list_to_add){
            state = putDirs(archive, dir.first, dir.second);
            if(!state){
                break;
            }
        }
    }
    return state;
}

bool archive::Archiver::save(){

    int err;
    zip_error_t error;

    if(flag_ == 0){
        if(std::filesystem::exists(archive_file_name_)){
            flag_ = ZIP_CHECKCONS;
        }else{
            flag_ = ZIP_CREATE;
        }
    }
    bool state = true;
    if(state){
        zip_t *archive = zip_open(archive_file_name_.c_str(), flag_, &err);

        if (archive == nullptr) {
            error_message_ = "Cannot open zip archive. ";
            zip_error_init_with_code(&error, err);
            zip_error_fini(&error);
            error_message_.append(zip_error_strerror(&error));
            state = false;
        }

        if(state){
            state = putDirsList(archive, add_dir_name_list_);
            if(state){
                state = putDirsList(archive, add_file_name_list_);
            }
        }

        if(!add_delete_file_list_.empty() && state){
            for (auto &&name_file : add_delete_file_list_){
                delFile(archive, name_file);
            }
        }

        int rez = 0;
        if(state){
            rez = zip_close(archive);
            if (rez != 0) {
                state = false;
            }
        }

        if (!state) {
            error_message_.append(" Cannot close zip archive");
            zip_discard(archive);
            state = false;
        }
    }
    return state;
}

bool archive::Archiver::putDirs(zip_t *archive, std::string path_source, std::string path_destination){
    if(std::filesystem::exists(path_source)){
        if(std::filesystem::is_directory(path_source)){
            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(path_source)){
                std::string dir_source = dir_entry.path().string();
                if(std::filesystem::is_directory(dir_source)){
                    // if(zip_add_dir(archive, path_source.c_str()) != 0){
                    if(zip_dir_add(archive, path_source.c_str(), ZIP_FL_ENC_UTF_8) < 0){
                        zip_error_t error;
                        error_message_ = "Can`t add dir. ";
                        zip_error_fini(&error);
                        error_message_.append(zip_error_strerror(&error));
                        return false;
                    }
                }else{
                    if(!putFiels(archive, dir_source, dir_source))
                        return false;
                }
            }
        }else if(std::filesystem::is_regular_file(path_source)){
            if(!putFiels(archive, path_source, path_destination))
                return false;
        }
    }
    return true;
}

bool archive::Archiver::addFiel(
    zip_t *archive,
    zip_source_t* source,
    const char* file_destination,
    const std::string &file_sours
){
    bool ret_value = true;
    if(zip_file_add(
            archive,
            file_destination,
            source,
            ZIP_FL_ENC_UTF_8
        ) < 0
    ){
        zip_source_free(source);
        error_message_.append("Can't add file to zip archive.")
            . append(" File source: ") . append(file_sours)
            . append(", file destination ") . append(file_destination);
        ret_value = false;
    }
    return ret_value;
}

bool archive::Archiver::replaceFile(
    zip_t *archive,
    zip_uint64_t index,
    zip_source_t *source,
    const std::string &file_destination,
    const std::string &file_sours
){
    bool ret_value = true;

    if(zip_file_replace(
            archive,
            index,
            source,
            ZIP_FL_ENC_UTF_8
        ) < 0
    ){
        zip_source_free(source);
        error_message_.append("Can't replace file in zip archive.")
            . append(" File source: ")
            . append(file_sours)
            . append(", file destination ")
            . append(file_destination);
        ret_value = false;
    }
    return ret_value;
}

bool archive::Archiver::putFiels(
    zip_t *archive,
    std::string file_sours,
    std::string file_destination
){
    bool ret_value = true;
    zip_source_t* source = zip_source_file(
        archive, file_sours.c_str(), 0, -1
    );

    if (source == nullptr){
        error_message_ = "Can't open zip from source " + file_sours;
        return false;
    }
    zip_int64_t file_pos =
        findFileInZipT(archive, file_destination);
    if(file_pos >= 0){
        ret_value = replaceFile(
            archive,
            file_pos,
            source,
            file_destination,
            file_sours
        );
    }else{
        ret_value = addFiel(
            archive,
            source,
            file_destination.c_str(),
            file_sours
        );
    }

    zip_source_close(source);
    return ret_value;
}

bool archive::Archiver::delFile(
    zip_t *archive, std::string file_name
){
    bool ret_value = true;
    if(zip_delete(
            archive, findFileInZipT(archive, file_name.c_str())
        ) != 0
    ){
        error_message_.append(" Can`t delete fiel. File name: ")
            . append(file_name);
        ret_value = false;
    }
    return ret_value;
}

bool archive::Archiver::delFile(
    zip_t *archive, zip_int64_t file_pos
){
    bool ret_value = true;
    if(file_pos >= 0){
        if(zip_delete(archive, file_pos) != 0){
            error_message_.append(" Can`t delete fiel. File pos: ")
                . append(std::to_string(file_pos));
            ret_value = false;
        }
    }
    return ret_value;
}

bool archive::Archiver::deleteList(
    zip_t *archive,
    std::map<std::string, std::string> list_to_clear
){
    bool state = true;
    if (!list_to_clear.empty()){
        for (auto &&list : list_to_clear){
            zip_int64_t file_pos =
                findFileInZipT(archive, list.second.c_str());
            if(file_pos >= 0){
                state = delFile(archive, file_pos);
            }
            if(!state)
                break;
        }
    }
    return state;
}

bool archive::Archiver::removeFilesIfExistsInArchive(){
    int err;
    zip_error_t error;
    bool state = true;

    zip_t *archive = zip_open(
        archive_file_name_.c_str(), flag_, &err);

    if (archive == nullptr) {
        error_message_ = "Cannot open zip archive. ";
        zip_error_init_with_code(&error, err);
        zip_error_fini(&error);
        error_message_.append(zip_error_strerror(&error));
        state = false;
    }
    if(state){
        state = deleteList(archive, add_dir_name_list_);
        if(state){
            state = deleteList(archive, add_file_name_list_);
        }
    }

    int rez = 0;
    if(state){
        rez = zip_close(archive);
        if (rez != 0) {
            state = false;
        }
    }

    if (!state) {
        error_message_.append(" Cannot close zip archive");
        zip_discard(archive);
        state = false;
    }

    return state;

}

void archive::Archiver::deleteFiles(
    std::vector<std::string> file_list
){
    for (auto &&file_name : file_list){
        deleteFile(file_name);
    }
}

void archive::Archiver::deleteFile(std::string file_name){
    if(std::filesystem::exists(file_name)){
        add_delete_file_list_.push_back(file_name);
    }
}

zip_int64_t archive::Archiver::findFileInZipT(
    zip_t *archive, std::string file_name
){
    return zip_name_locate(
        archive, file_name.c_str(), ZIP_FL_ENC_UTF_8
    );
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
