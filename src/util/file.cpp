// Copyright (c) 2022 Leo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of
// the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "internal/util.h"

namespace vhash {

FileLoader::FileLoader(const std::string& file_path, const std::string& mode) : FileLoader() {
    file = fopen(file_path.c_str(), mode.c_str());
    if (!file) {
        error = true;
        return;
    }
    struct stat statbuf = {0};
    int rtn = stat(file_path.c_str(), &statbuf);
    if (rtn) {
        error = true;
        return;
    }
    length = statbuf.st_size;
}

FileLoader::~FileLoader() {
    if (file) fclose(file);
    file = nullptr;
}

bool FileLoader::is_open() const {
    return file != nullptr && !error;
}

size_t FileLoader::size() const {
    return length;
}

int FileLoader::read(std::vector<uint8_t>& vec) {
    vec.clear();
    vec.resize(length);
    size_t n = fread(&vec[0], 1, length, file);
    if (n != length)
        return VERROR(errors::ERR_READ_FILE);
    return static_cast<int>(n);
}

FileWriter::FileWriter(const std::string& file_path): error(false), is_cout(true) {
    if (!file_path.empty()) {
        try {
            of.open(file_path, std::ios::out | std::ios::trunc);
            is_cout = false;
        } catch(std::exception& e) {
            error = true;
            return;
        }
    }
}

FileWriter::~FileWriter() {
    if (!is_cout && of.is_open()) {
        of.close();
    }
}

bool FileWriter::is_open() const {
    return !is_cout && of.is_open();
}

}