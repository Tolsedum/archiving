#ifndef ARCHIVER_HPP
#define ARCHIVER_HPP
/** Version 1.2 */

/**
 *  __________________________________________
 * |                                          |
 * |   ╭━━━━┳━━━┳╮╱╱╭━━━┳━━━┳━━━┳╮╱╭┳━╮╭━╮    |
 * |   ┃╭╮╭╮┃╭━╮┃┃╱╱┃╭━╮┃╭━━┻╮╭╮┃┃╱┃┃┃╰╯┃┃    |
 * |   ╰╯┃┃╰┫┃╱┃┃┃╱╱┃╰━━┫╰━━╮┃┃┃┃┃╱┃┃╭╮╭╮┃    |
 * |   ╱╱┃┃╱┃┃╱┃┃┃╱╭╋━━╮┃╭━━╯┃┃┃┃┃╱┃┃┃┃┃┃┃    |
 * |   ╱╱┃┃╱┃╰━╯┃╰━╯┃╰━╯┃╰━━┳╯╰╯┃╰━╯┃┃┃┃┃┃    |
 * |   ╱╱╰╯╱╰━━━┻━━━┻━━━┻━━━┻━━━┻━━━┻╯╰╯╰╯    |
 * |__________________________________________|
 * |                                          |
 * | Permission is hereby granted, free of    |
 * | charge, to any person obtaining a copy of|
 * | of this software and accompanying files, |
 * | to use them without restriction,         |
 * | including, without limitation, the       |
 * | rights to use, copy, modify, merge,      |
 * | publish, distribute, sublicense and/or   |
 * | sell copies of the software. The authors |
 * | or copyright holders shall not be liable |
 * | for any claims, damages or other         |
 * | liability, whether in contract, tort or  |
 * | otherwise, arising out of or in          |
 * | connection with the software or your use |
 * | or other dealings with the software.     |
 * |__________________________________________|
 * |   website: tolsedum.ru                   |
 * |   email: tolsedum@gmail.com              |
 * |   email: tolsedum@yandex.ru              |
 * |__________________________________________|
 */

#include <zip.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>

namespace archive{
    class Archiver{
    private:
        /** https://libzip.org/documentation/libzip.html */

        // bool throw_if_zip_exsists_;
        // int compression_method_;

        short flag_;

        std::string archive_file_name_;
        std::string error_message_;

        std::map<std::string, std::string> add_file_name_list_;
        std::map<std::string, std::string> add_dir_name_list_;

        std::vector<std::string> add_delete_file_list_;
        std::vector<std::string> warnings_;

        /**
         * @brief Find file in archive
         */
        zip_int64_t findFileInZipT(zip_t *archive, std::string file_name);


        /**
         * @brief Recursively put directories and files in a directory in archive
         * @param archive
         * @param path_source
         * @param path_destination
         */
        bool putDirs(zip_t *archive, std::string path_source, std::string path_destination);
        /**
         * @brief Put files in archive
         * @param archive
         * @param file_sours file sours
         * @param file_destination file destination
         */
        bool putFiels(zip_t *archive, std::string file_sours, std::string file_destination);
        /**
         * Add file
         */
        bool addFiel(
            zip_t *archive,
            zip_source_t* source,
            const char* file_destination,
            const std::string &file_sours
        );

        /**
         * Replace file
         */
        bool replaceFile(
            zip_t *archive,
            zip_uint64_t index,
            zip_source_t *source,
            const std::string &file_destination,
            const std::string &file_sours
        );

        /**
         * @brief Delete file in archive
         * @param file_name
        */
        bool delFile(zip_t *archive, std::string file_name);
        bool delFile(zip_t *archive, zip_int64_t file_pos);

        bool removeFilesIfExistsInArchive();

        bool deleteList(
            zip_t *archive,
            std::map<std::string, std::string> list_to_clear
        );
        bool putDirsList(
            zip_t *archive,
            std::map<std::string, std::string> list_to_add
        );
    public:
        Archiver(
            std::string name,
            short flag = 0,
            bool throw_if_zip_exsists = false
        )
            : flag_(flag)
            , archive_file_name_(name)
        {
            if(throw_if_zip_exsists && std::filesystem::exists(name)){
                throw "Zip file exists " + name;
            }
        };
        ~Archiver(){};

        std::string getErrorMessage();
        std::vector<std::string> getWarningsMessage();

        /**
         * @brief Add dirs in list
         * @param name_dirs pair<dir source, dir destination>
         */
        void addDirs(std::map<std::string, std::string> name_dirs);

        /**
         * @brief Add files in list
         * @param name_dirs pair<file source, file destination>
         */
        void addFiles(std::map<std::string, std::string> name_files);

        /**
         * @brief Add dir in list
         */
        void addDir(std::string dir_source, std::string dir_destination);

        /**
         * @brief Add file in list
         */
        void addFile(std::string file_source, std::string file_destination);

        /**
         * @brief Delete files from list and archive
         */
        void deleteFiles(std::vector<std::string> file_list);

        /**
         * @brief Delete file from  list and archive
         */
        void deleteFile(std::string file_name);

        /**
         * @brief Reset all changes and close archive
         */
        void cancel();

        /**
         * @brief Save and close archive
         */
        bool save();
    };
}

#endif // ARCHIVER_HPP
